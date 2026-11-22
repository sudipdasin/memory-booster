#include "core.hpp"
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <string_view>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>

namespace mb {
namespace fs=std::filesystem;
static std::string trim(std::string s){auto p=s.find_first_not_of(" \t\r\n"); if(p==s.npos)return {}; auto q=s.find_last_not_of(" \t\r\n");return s.substr(p,q-p+1);}
static std::string read_first(const fs::path& p){std::ifstream f(p);std::string s;std::getline(f,s);return trim(s);}
static bool write_value(const fs::path& p,const std::string& v){std::ofstream f(p);if(!f)return false;f<<v<<'\n';f.close();return !f.fail();}
static bool numeric(std::string s){return !s.empty() && std::all_of(s.begin(),s.end(),[](unsigned char c){return std::isdigit(c);});}
Memory parse_memory(std::istream& in){
 Memory m;std::string line;std::map<std::string,uint64_t> vals;
 while(std::getline(in,line)){std::istringstream is(line);std::string key,unit;uint64_t value=0;if(!(is>>key>>value))continue;if(key.empty()||key.back()!=':')continue;key.pop_back();if(!(is>>unit)||unit!="kB")continue;vals[key]=value;}
 auto get=[&](const char* k){auto it=vals.find(k);return it==vals.end()?uint64_t(0):it->second;};
 m.total=get("MemTotal");m.free=get("MemFree");m.available=get("MemAvailable");m.cached=get("Cached");m.buffers=get("Buffers");m.swap_total=get("SwapTotal");m.swap_free=get("SwapFree");m.valid=m.total>0 && (!vals.count("MemAvailable") || get("MemAvailable") <= m.total) && (!vals.count("SwapFree") || get("SwapFree") <= m.swap_total);
 if(!vals.count("MemAvailable")){uint64_t v=m.free;for(auto n:{m.cached,m.buffers}){v = (n > m.total || v > m.total-n) ? m.total : v+n;}m.available=std::min(v,m.total);}
 m.available=std::min(m.available,m.total);m.swap_free=std::min(m.swap_free,m.swap_total);return m;
}
Pressure parse_pressure(std::istream& in){Pressure p;std::string line;
 while(std::getline(in,line)){std::istringstream is(line);std::string kind,part;if(!(is>>kind))continue; if(kind!="some"&&kind!="full")continue;
 while(is>>part){auto pos=part.find('=');if(pos==std::string::npos)continue;std::string key=part.substr(0,pos);try{const std::string str=part.substr(pos+1);std::size_t consumed=0;double val=std::stod(str,&consumed);if(consumed!=str.size()||!std::isfinite(val)||val<0||val>100)continue;
 if(kind=="some"&&key=="avg10"){p.some10=val;p.available=true;}else if(kind=="some"&&key=="avg60")p.some60=val;
 else if(kind=="full"&&key=="avg10"){p.full10=val;p.available=true;}else if(kind=="full"&&key=="avg60")p.full60=val;}catch(...){} }
 }return p;
}
static std::vector<Process> top_processes(){std::vector<Process> v;std::error_code ec;
 for(auto it=fs::directory_iterator("/proc",fs::directory_options::skip_permission_denied,ec);!ec&&it!=fs::directory_iterator();it.increment(ec)){
  const std::string name=it->path().filename().string();if(!numeric(name))continue;int pid=0;try{pid=std::stoi(name);}catch(...){continue;}
  std::ifstream stat(it->path()/"statm");uint64_t pages=0,rss=0;if(!(stat>>pages>>rss))continue;
  std::string comm=read_first(it->path()/"comm");if(comm.empty())comm="unknown";
  auto page=::sysconf(_SC_PAGESIZE);if(page<=0)continue;uint64_t kb=rss*static_cast<uint64_t>(page)/1024;
  if(kb) v.push_back({pid,comm,kb});
 }
 std::sort(v.begin(),v.end(),[](auto& a,auto& b){return a.rss_kib>b.rss_kib;});if(v.size()>12)v.resize(12);return v;
}
Snapshot collect(){Snapshot s;{std::ifstream f("/proc/meminfo");s.memory=parse_memory(f);} {std::ifstream f("/proc/pressure/memory");if(f)s.pressure=parse_pressure(f);}
 s.processes=top_processes();s.swappiness=read_first("/proc/sys/vm/swappiness");s.vfs_cache_pressure=read_first("/proc/sys/vm/vfs_cache_pressure");
 if(s.swappiness.empty()) s.swappiness="unavailable";
 if(s.vfs_cache_pressure.empty()) s.vfs_cache_pressure="unavailable";
 std::error_code ec;for(auto it=fs::directory_iterator("/sys/block",fs::directory_options::skip_permission_denied,ec);!ec&&it!=fs::directory_iterator();it.increment(ec)){
  auto name=it->path().filename().string();if(name.rfind("zram",0)!=0)continue;
  auto disk=read_first(it->path()/"disksize");auto mm=read_first(it->path()/"mm_stat");std::string label=name+" size="+ (disk.empty()?"?":disk)+" bytes";
  if(!mm.empty()){std::istringstream is(mm);uint64_t orig=0,compr=0,mem=0;if(is>>orig>>compr>>mem){label+=" compressed="+std::to_string(compr)+" bytes";}}
  s.zram.push_back(label);
 }std::sort(s.zram.begin(),s.zram.end());return s;
}
std::vector<Profile> profiles(){return {{"balanced",60,100,"Conservative defaults; prioritize kernel-managed caching."},{"responsive",20,75,"Favor interactive responsiveness when RAM headroom is adequate."},{"compressed-swap",100,100,"Consider when zram or compressed swap is already configured; inspect workload first."}};}
ApplyResult apply_profile(const Profile& p,const std::string& proc_root,bool dry_run){
 if(p.swappiness<0||p.swappiness>200||p.vfs_cache_pressure<1||p.vfs_cache_pressure>1000)return {false,"Invalid profile range"};
 fs::path root=fs::path(proc_root)/"sys/vm";auto sw=root/"swappiness",vf=root/"vfs_cache_pressure";
 auto old_sw=read_first(sw),old_vf=read_first(vf);if(!numeric(old_sw)||!numeric(old_vf))return {false,"Kernel settings unavailable or unreadable"};
 // Reject implausible kernel state rather than writing a profile that cannot be safely restored.
 try {if(std::stoll(old_sw)>200 || std::stoll(old_vf)<1 || std::stoll(old_vf)>1000) return {false,"Original kernel settings outside supported range"};} catch(...) {return {false,"Unparseable kernel settings"};}
 if(dry_run)return {true,"Dry run: would change swappiness "+old_sw+" -> "+std::to_string(p.swappiness)+", vfs_cache_pressure "+old_vf+" -> "+std::to_string(p.vfs_cache_pressure)};
 if(proc_root=="/proc" && ::geteuid()!=0)return {false,"Administrator privileges required (use pkexec)"};
 // A failed write can still have partially modified a kernel value. Always
 // attempt both rollbacks, then read back both original values.
 auto restore=[&](){
  bool ok_sw=write_value(sw,old_sw);
  bool ok_vf=write_value(vf,old_vf);
  return ok_sw&&ok_vf&&read_first(sw)==old_sw&&read_first(vf)==old_vf;
 };
 if(!write_value(sw,std::to_string(p.swappiness))){
  bool restored=restore();return {false,restored?"Could not update swappiness; original settings verified":"Swappiness write failed; rollback incomplete, inspect kernel values"};
 }
 if(!write_value(vf,std::to_string(p.vfs_cache_pressure))){
  bool restored=restore();return {false,restored?"VFS cache pressure write failed; original settings restored":"VFS cache pressure write failed; rollback incomplete, inspect kernel values"};
 }
 if(read_first(sw)!=std::to_string(p.swappiness)||read_first(vf)!=std::to_string(p.vfs_cache_pressure)){
  bool restored=restore();return {false,restored?"Verification failed; original settings restored":"Verification failed; rollback incomplete, inspect kernel values"};
 }
 return {true,"Applied "+p.name+" temporarily (reset by reboot); original swappiness="+old_sw+", vfs_cache_pressure="+old_vf};
}
static std::string fmt(uint64_t kib){std::ostringstream out;out<<std::fixed<<std::setprecision(1)<<kib/1048576.0<<" GiB";return out.str();}
std::string report(const Snapshot& s){std::ostringstream o;auto& m=s.memory;if(!m.valid)return "MemBoost Pro: /proc/meminfo unavailable\n";
 o<<"MemBoost Pro | memory report\nRAM total "<<fmt(m.total)<<" | available "<<fmt(m.available)<<" | used "<<fmt(m.total-m.available)<<"\nSwap total "<<fmt(m.swap_total)<<" | used "<<fmt(m.swap_total-m.swap_free)<<"\n";
 if(s.pressure.available)o<<"PSI memory some avg10="<<s.pressure.some10<<"% full avg10="<<s.pressure.full10<<"%\n";
 o<<"vm.swappiness="<<s.swappiness<<" | vm.vfs_cache_pressure="<<s.vfs_cache_pressure<<"\nZram devices: "<<s.zram.size()<<"\nTop memory processes:\n";
 for(auto& p:s.processes) o<<"  "<<p.pid<<"\t"<<p.name<<"\t"<<p.rss_kib/1024<<" MiB RSS\n";
 return o.str();}
static std::string esc(const std::string& s){std::ostringstream o;for(unsigned char c:s){if(c=='"'||c=='\\')o<<'\\'<<c;else if(c<32){o<<"\\u"<<std::hex<<std::setw(4)<<std::setfill('0')<<(unsigned)c<<std::dec;}else o<<c;}return o.str();}
std::string json(const Snapshot& s){auto& m=s.memory;std::ostringstream o;o<<"{\"valid\":"<<(m.valid?"true":"false")<<",\"ram_total_kib\":"<<m.total<<",\"ram_available_kib\":"<<m.available<<",\"swap_total_kib\":"<<m.swap_total<<",\"swap_free_kib\":"<<m.swap_free<<",\"psi_available\":"<<(s.pressure.available?"true":"false")<<",\"psi_some_avg10\":"<<s.pressure.some10<<",\"psi_full_avg10\":"<<s.pressure.full10<<",\"swappiness\":\""<<esc(s.swappiness)<<"\",\"vfs_cache_pressure\":\""<<esc(s.vfs_cache_pressure)<<"\",\"processes\":[";
 for(size_t i=0;i<s.processes.size();++i){auto& p=s.processes[i];if(i)o<<",";o<<"{\"pid\":"<<p.pid<<",\"name\":\""<<esc(p.name)<<"\",\"rss_kib\":"<<p.rss_kib<<"}";}
 o<<"],\"zram\":[";for(size_t i=0;i<s.zram.size();++i){if(i)o<<",";o<<"\""<<esc(s.zram[i])<<"\"";}o<<"]}";return o.str();}
}
