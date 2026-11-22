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
static bool parse_u64(const std::string& raw,uint64_t& out){
 const auto s=trim(raw);if(!numeric(s))return false;
 try {size_t n=0;auto v=std::stoull(s,&n);if(n!=s.size())return false;out=v;return true;}catch(...){return false;}
}
// Parsers use strict nonnegative decimal counters. Missing optional kernel
// fields remain unavailable, instead of being misreported as zero activity.
VmCounters parse_vmstat(std::istream& in) {
 VmCounters v;std::string key,raw;bool major=false,swap_in=false,swap_out=false;
 while(in>>key>>raw){uint64_t value=0;if(!parse_u64(raw,value))continue;
  if(key=="pgmajfault"){v.major_faults=value;major=true;}
  else if(key=="pswpin"){v.swap_in=value;swap_in=true;}
  else if(key=="pswpout"){v.swap_out=value;swap_out=true;}
  else if(key=="oom_kill"){v.oom_kills=value;v.oom_available=true;}
 }
 v.available=major&&swap_in&&swap_out;return v;
}
CgroupEvents parse_cgroup_events(std::istream& in){
 CgroupEvents e;std::string key,raw;bool found_oom=false,found_kill=false;
 while(in>>key>>raw){uint64_t value=0;if(!parse_u64(raw,value))continue;
  if(key=="low")e.low=value;
  else if(key=="high")e.high=value;
  else if(key=="max")e.max=value;
  else if(key=="oom"){e.oom=value;found_oom=true;}
  else if(key=="oom_kill"){e.oom_kill=value;found_kill=true;}
 }
 e.available=found_oom&&found_kill;return e;
}
TelemetryDelta telemetry_delta(const Snapshot& before,const Snapshot& after,double seconds){
 TelemetryDelta d;
 if(!std::isfinite(seconds)||seconds<=0)return d;
 d.seconds=seconds;
 if(before.vmstat.available&&after.vmstat.available){
  const auto& a=before.vmstat;const auto& b=after.vmstat;
  if(b.major_faults<a.major_faults||b.swap_in<a.swap_in||b.swap_out<a.swap_out||
    (a.oom_available&&b.oom_available&&b.oom_kills<a.oom_kills)) d.reset_detected=true;
  else{d.vm_available=true;d.major_faults=b.major_faults-a.major_faults;
   d.swap_in=b.swap_in-a.swap_in;d.swap_out=b.swap_out-a.swap_out;
   if(a.oom_available&&b.oom_available)d.oom_kills=b.oom_kills-a.oom_kills;}
 }
 if(before.cgroup_events.available&&after.cgroup_events.available){
  const auto& a=before.cgroup_events;const auto& b=after.cgroup_events;
  if(b.high<a.high||b.oom<a.oom||b.oom_kill<a.oom_kill)d.reset_detected=true;
  else{d.cgroup_available=true;d.cgroup_high=b.high-a.high;
   d.cgroup_oom=b.oom-a.oom;d.cgroup_oom_kill=b.oom_kill-a.oom_kill;}
 }
 return d;
}
std::string signals_report(const Snapshot& s,const TelemetryDelta& d){
 std::ostringstream o;
 o<<"Memory telemetry | read-only, cumulative counters since boot/cgroup creation\n";
 if(s.vmstat.available){o<<"Host major faults="<<s.vmstat.major_faults<<" pages; swap-in="<<s.vmstat.swap_in
  <<" pages; swap-out="<<s.vmstat.swap_out<<" pages";
  if(s.vmstat.oom_available)o<<"; OOM kills="<<s.vmstat.oom_kills;
  o<<"\n";
 }else o<<"Host vmstat counters unavailable\n";
 if(s.cgroup_events.available)o<<"Cgroup v2 high="<<s.cgroup_events.high<<"; OOM="<<s.cgroup_events.oom
  <<"; OOM kills="<<s.cgroup_events.oom_kill<<" (cumulative)\n";
 else o<<"Cgroup memory.events not available\n";
 if(d.vm_available){o<<std::fixed<<std::setprecision(2)<<"Observed over "<<d.seconds<<"s: major faults/s="
  <<d.major_faults/d.seconds<<" swap-in pages/s="<<d.swap_in/d.seconds
  <<" swap-out pages/s="<<d.swap_out/d.seconds<<" OOM kills="<<d.oom_kills<<"\n";}
 if(d.cgroup_available)o<<"Cgroup interval: high events="<<d.cgroup_high
  <<"; OOM events="<<d.cgroup_oom<<"; OOM kills="<<d.cgroup_oom_kill<<"\n";
 if(d.reset_detected)o<<"Counter reset detected; rate from affected source suppressed.\n";
 if(!d.vm_available&&!d.cgroup_available)o<<"No interval yet; use --track for measured rates.\n";
 return o.str();
}
Cgroup parse_cgroup(const std::string& current,const std::string& limit,const std::string& swap_current,const std::string& swap_limit){
 Cgroup c;
 if(!parse_u64(current,c.current_bytes))return c;
 c.available=true;
 if(parse_u64(limit,c.limit_bytes)&&c.limit_bytes>0)c.limited=true;
 if(parse_u64(swap_current,c.swap_current_bytes)){
  c.swap_available=true;
  if(parse_u64(swap_limit,c.swap_limit_bytes)&&c.swap_limit_bytes>0)c.swap_limited=true;
 }
 return c;
}
Diagnosis diagnose(const Snapshot& s){
 Diagnosis d;
 if(!s.memory.valid){d.findings.push_back("Memory counters missing or inconsistent; diagnostics unavailable.");d.advice.push_back("Verify that /proc/meminfo is mounted and readable.");return d;}
 bool warn=false,critical=false;
 if(s.pressure.available){
  critical=s.pressure.some10>=25.0 || s.pressure.full10>=5.0;
  warn=s.pressure.some10>=10.0 || s.pressure.full10>=1.0;
  if(critical)d.findings.push_back("High short-term task stalls detected (PSI); investigate active workloads.");
  else if(warn)d.findings.push_back("Elevated short-term task stalls detected (PSI).");
  else d.findings.push_back("Low short-term memory stalls reported by PSI.");
 }else d.findings.push_back("Kernel memory PSI is unavailable; pressure grade is limited.");
 if(s.cgroup.available&&s.cgroup.limited){
  const double share=static_cast<double>(s.cgroup.current_bytes)/static_cast<double>(s.cgroup.limit_bytes);
  if(share>=0.90){critical=true;d.findings.push_back("Cgroup memory usage is at least 90% of its limit.");}
  else if(share>=0.80){warn=true;d.findings.push_back("Cgroup memory usage is at least 80% of its limit.");}
  else d.findings.push_back("Cgroup memory usage is below 80% of its limit.");
 }else d.findings.push_back("No finite cgroup v2 memory limit was detected.");
 d.level=critical?"high":warn?"elevated":(s.pressure.available||s.cgroup.limited?"normal":"unknown");
 if(critical||warn){
  d.advice.push_back("Inspect the processes tab and identify persistent memory consumers before changing settings.");
  d.advice.push_back("Compare PSI and application latency under the same workload; avoid clearing useful page caches.");
 }else d.advice.push_back("No automatic tuning is indicated by these observations; continue monitoring under load.");
 if(s.cgroup.limited)d.advice.push_back("A container or service memory limit may matter more than host free RAM.");
 d.advice.push_back("zram, zswap and OOM policy should be managed using distribution-specific tools.");
 return d;
}
std::string diagnostic_report(const Snapshot& s){
 auto d=diagnose(s);std::ostringstream o;
 o<<"MemBoost Pro | diagnostic grade: "<<d.level<<" (advisory, not a performance guarantee)\n";
 if(s.cgroup.available){o<<"Cgroup v2 current="<<s.cgroup.current_bytes<<" bytes; limit=";
  if(s.cgroup.limited)o<<s.cgroup.limit_bytes;else o<<"unlimited";o<<"\n";}
 for(auto& f:d.findings)o<<"Finding: "<<f<<"\n";
 for(auto& a:d.advice)o<<"Suggestion: "<<a<<"\n";
 return o.str();
}
// Use the process' actual unified cgroup path rather than assuming the host root.
// Reject traversal and non-cgroup paths; a container may have no v2 mount.
static fs::path local_cgroup_mount(){
 fs::path base="/sys/fs/cgroup";
 std::ifstream proc("/proc/self/cgroup"); std::string line;
 while(std::getline(proc,line)){
  if(line.rfind("0::",0)!=0)continue;
  std::string rel=line.substr(3);
  if(rel.empty() || rel[0]!='/' || rel.find("..")!=std::string::npos || rel.find('\\')!=std::string::npos)return {};
  // The mount root of a cgroup namespace often already represents this cgroup.
  // Prefer the nested path if it exists; otherwise use mount root if readable.
  fs::path nested=base/rel.substr(1);
  if(fs::is_regular_file(nested/"memory.current"))return nested;
  break;
 }
 if(fs::is_regular_file(base/"memory.current"))return base;
 return {};
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
 const fs::path cgroot=local_cgroup_mount();
 if(!cgroot.empty())s.cgroup=parse_cgroup(read_first(cgroot/"memory.current"),read_first(cgroot/"memory.max"),
                         read_first(cgroot/"memory.swap.current"),read_first(cgroot/"memory.swap.max"));
 {std::ifstream f("/proc/vmstat");if(f)s.vmstat=parse_vmstat(f);}
 if(!cgroot.empty()){std::ifstream f(cgroot/"memory.events");if(f)s.cgroup_events=parse_cgroup_events(f);}
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
 if(s.cgroup.available){o<<"cgroup v2: "<<s.cgroup.current_bytes<<" bytes used, ";
  if(s.cgroup.limited)o<<s.cgroup.limit_bytes<<" bytes limit\n";else o<<"no finite limit\n";}
 o<<"vm.swappiness="<<s.swappiness<<" | vm.vfs_cache_pressure="<<s.vfs_cache_pressure<<"\nZram devices: "<<s.zram.size()<<"\nTop memory processes:\n";
 for(auto& p:s.processes) o<<"  "<<p.pid<<"\t"<<p.name<<"\t"<<p.rss_kib/1024<<" MiB RSS\n";
 return o.str();}
static std::string esc(const std::string& s){std::ostringstream o;for(unsigned char c:s){if(c=='"'||c=='\\')o<<'\\'<<c;else if(c<32){o<<"\\u"<<std::hex<<std::setw(4)<<std::setfill('0')<<(unsigned)c<<std::dec;}else o<<c;}return o.str();}
std::string json(const Snapshot& s){auto& m=s.memory;std::ostringstream o;o<<"{\"valid\":"<<(m.valid?"true":"false")<<",\"ram_total_kib\":"<<m.total<<",\"ram_available_kib\":"<<m.available<<",\"swap_total_kib\":"<<m.swap_total<<",\"swap_free_kib\":"<<m.swap_free<<",\"psi_available\":"<<(s.pressure.available?"true":"false")<<",\"psi_some_avg10\":"<<s.pressure.some10<<",\"psi_full_avg10\":"<<s.pressure.full10<<",\"swappiness\":\""<<esc(s.swappiness)<<"\",\"vfs_cache_pressure\":\""<<esc(s.vfs_cache_pressure)<<"\",\"processes\":[";
 for(size_t i=0;i<s.processes.size();++i){auto& p=s.processes[i];if(i)o<<",";o<<"{\"pid\":"<<p.pid<<",\"name\":\""<<esc(p.name)<<"\",\"rss_kib\":"<<p.rss_kib<<"}";}
 o<<"],\"zram\":[";for(size_t i=0;i<s.zram.size();++i){if(i)o<<",";o<<"\""<<esc(s.zram[i])<<"\"";}o<<"],\"schema_version\":3,\"vmstat\":{\"available\":"<<(s.vmstat.available?"true":"false")
 <<",\"major_faults\":"<<s.vmstat.major_faults<<",\"swap_in_pages\":"<<s.vmstat.swap_in
 <<",\"swap_out_pages\":"<<s.vmstat.swap_out<<",\"oom_kills\":"<<s.vmstat.oom_kills
 <<"},\"cgroup_events\":{\"available\":"<<(s.cgroup_events.available?"true":"false")
 <<",\"high\":"<<s.cgroup_events.high<<",\"oom\":"<<s.cgroup_events.oom
 <<",\"oom_kill\":"<<s.cgroup_events.oom_kill<<"},\"cgroup\":{\"available\":"<<(s.cgroup.available?"true":"false")
 <<",\"limited\":"<<(s.cgroup.limited?"true":"false")<<",\"current_bytes\":"<<s.cgroup.current_bytes
 <<",\"limit_bytes\":"<<s.cgroup.limit_bytes<<",\"swap_available\":"<<(s.cgroup.swap_available?"true":"false")
 <<",\"swap_current_bytes\":"<<s.cgroup.swap_current_bytes<<"},\"diagnosis\":{\"level\":\""<<esc(diagnose(s).level)<<"\"}}";
 return o.str();}
std::string telemetry_json(const Snapshot& s,const TelemetryDelta& d,unsigned sample){
 std::ostringstream o;
 o<<"{\"telemetry_schema\":1,\"sample\":"<<sample<<",\"snapshot\":"<<json(s)
  <<",\"interval\":{\"seconds\":"<<std::fixed<<std::setprecision(4)<<d.seconds
  <<",\"vm_available\":"<<(d.vm_available?"true":"false")
  <<",\"cgroup_available\":"<<(d.cgroup_available?"true":"false")
  <<",\"reset_detected\":"<<(d.reset_detected?"true":"false")
  <<",\"major_faults\":"<<d.major_faults
  <<",\"swap_in_pages\":"<<d.swap_in<<",\"swap_out_pages\":"<<d.swap_out
  <<",\"oom_kills\":"<<d.oom_kills
  <<",\"cgroup_high\":"<<d.cgroup_high
  <<",\"cgroup_oom\":"<<d.cgroup_oom
  <<",\"cgroup_oom_kills\":"<<d.cgroup_oom_kill<<"}}";
 return o.str();
}

}
