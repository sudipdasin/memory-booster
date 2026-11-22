#include "core.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <random>
namespace fs=std::filesystem;
int main(){
 {std::istringstream i("MemTotal: 4096000 kB\nMemFree: 1000 kB\nMemAvailable: 1024000 kB\nSwapTotal: 2048 kB\nSwapFree: 5000 kB\n");auto m=mb::parse_memory(i);assert(!m.valid&&m.total==4096000&&m.available==1024000&&m.swap_free==2048);}
 {std::istringstream i("MemTotal: 2000 kB\nMemFree: 300 kB\nCached: 100 kB\nBuffers: 50 kB\n");auto m=mb::parse_memory(i);assert(m.available==450);}
 {std::istringstream i("some avg10=3.14 avg60=2.10 avg300=1.00 total=100\nfull avg10=0.50 avg60=0.20 avg300=0.10 total=1\n");auto p=mb::parse_pressure(i);assert(p.available&&p.some10>3.13&&p.full10==0.5);}
 {std::istringstream i("bad input\n");auto m=mb::parse_memory(i);assert(!m.valid);}
 {std::istringstream i("MemTotal: 0 kB\nMemAvailable: 0 kB\n");auto m=mb::parse_memory(i);assert(!m.valid);}
 {std::istringstream i("MemTotal: 1000 kB\nMemAvailable: 2000 kB\nSwapTotal: 0 kB\nSwapFree: 100 kB\n");auto m=mb::parse_memory(i);assert(m.available==1000&&m.swap_free==0&&!m.valid);}
 {std::istringstream i("MemTotal: 1024 kB\nMemFree: 18446744073709551615 kB\nCached: 900 kB\nBuffers: 900 kB\n");auto m=mb::parse_memory(i);assert(m.available==1024);}
 {std::istringstream i("some avg10=-2.00 avg60=101.00\nfull avg10=0.25 avg60=0.10\n");auto pr=mb::parse_pressure(i);assert(pr.available&&pr.some10==0&&pr.full10==0.25);}
 {std::istringstream i("some avg10=20junk avg60=2\n");auto pr=mb::parse_pressure(i);assert(!pr.available&&pr.some10==0);}
 {std::istringstream i("some avg10=nan\nfull avg10=inf\n");auto pr=mb::parse_pressure(i);assert(!pr.available);}
 {std::istringstream i("MemTotal: 1024 kB\nMemAvailable: 9999 kB\n");auto m=mb::parse_memory(i);assert(!m.valid&&m.available==1024);}
 {std::istringstream i("MemTotal: 1024 kB\nSwapTotal: 100 kB\nSwapFree: 101 kB\n");auto m=mb::parse_memory(i);assert(!m.valid&&m.swap_free==100);}
 {std::istringstream i("MemTotal: 1024 kB\nMemFree: 512 kB\nCached: 1000 kB\n");auto m=mb::parse_memory(i);assert(m.valid&&m.available==1024);}
 

 char root_template[]="/tmp/memboost-test-XXXXXX";char* created=::mkdtemp(root_template);assert(created!=nullptr);auto root=fs::path(created);fs::create_directories(root/"sys/vm");
 auto sw=root/"sys/vm/swappiness",vf=root/"sys/vm/vfs_cache_pressure";
 {std::ofstream(sw)<<"60\n";std::ofstream(vf)<<"100\n";}
 auto p=mb::profiles()[1];auto dry=mb::apply_profile(p,root.string(),true);assert(dry.ok);{std::string s;std::ifstream(sw)>>s;assert(s=="60");}
 {std::ofstream(sw)<<"201\n";assert(!mb::apply_profile(p,root.string(),false).ok);std::ofstream(sw)<<"60\n";}
 auto applied=mb::apply_profile(p,root.string(),false);assert(applied.ok);{std::string s;std::ifstream(sw)>>s;assert(s=="20");}
 assert(!mb::apply_profile({"bad",999,10,""},root.string(),false).ok);
 assert(!mb::apply_profile({"bad",-1,100,""},root.string(),false).ok);
 assert(!mb::apply_profile({"bad",60,0,""},root.string(),false).ok);
 assert(!mb::apply_profile({"bad",60,1001,""},root.string(),false).ok);
 assert(mb::apply_profile(mb::profiles()[0],root.string(),true).ok);
 {std::string s;std::ifstream(sw)>>s;assert(s=="20");}

 fs::remove(vf);auto failed=mb::apply_profile(mb::profiles()[0],root.string(),false);assert(!failed.ok);
 fs::remove_all(root);
 auto js=mb::json(mb::collect());assert(js.find("\"processes\"")!=std::string::npos);assert(js.find("\"ram_total_kib\"")!=std::string::npos);
 mb::Snapshot escaped;escaped.memory.total=1024;escaped.memory.available=512;escaped.memory.valid=true;
 escaped.processes.push_back({1,"foo\"\\bar\n",12});
 auto json=mb::json(escaped);assert(json.find("foo\\\"\\\\bar\\u000a")!=std::string::npos);

 std::cout<<"MemBoost core tests passed\n";
}
