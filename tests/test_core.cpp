#include "core.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
namespace fs=std::filesystem;
int main(){
 {std::istringstream i("MemTotal: 4096000 kB\nMemFree: 1000 kB\nMemAvailable: 1024000 kB\nSwapTotal: 2048 kB\nSwapFree: 5000 kB\n");auto m=mb::parse_memory(i);assert(m.valid&&m.total==4096000&&m.available==1024000&&m.swap_free==2048);}
 {std::istringstream i("MemTotal: 2000 kB\nMemFree: 300 kB\nCached: 100 kB\nBuffers: 50 kB\n");auto m=mb::parse_memory(i);assert(m.available==450);}
 {std::istringstream i("some avg10=3.14 avg60=2.10 avg300=1.00 total=100\nfull avg10=0.50 avg60=0.20 avg300=0.10 total=1\n");auto p=mb::parse_pressure(i);assert(p.available&&p.some10>3.13&&p.full10==0.5);}
 {std::istringstream i("bad input\n");auto m=mb::parse_memory(i);assert(!m.valid);}
 auto root=fs::temp_directory_path()/("mb-test-"+std::to_string(::getpid()));fs::create_directories(root/"sys/vm");
 auto sw=root/"sys/vm/swappiness",vf=root/"sys/vm/vfs_cache_pressure";
 {std::ofstream(sw)<<"60\n";std::ofstream(vf)<<"100\n";}
 auto p=mb::profiles()[1];auto dry=mb::apply_profile(p,root.string(),true);assert(dry.ok);{std::string s;std::ifstream(sw)>>s;assert(s=="60");}
 auto applied=mb::apply_profile(p,root.string(),false);assert(applied.ok);{std::string s;std::ifstream(sw)>>s;assert(s=="20");}
 assert(!mb::apply_profile({"bad",999,10,""},root.string(),false).ok);
 fs::remove(vf);auto failed=mb::apply_profile(mb::profiles()[0],root.string(),false);assert(!failed.ok);
 fs::remove_all(root);
 auto js=mb::json(mb::collect());assert(js.find("\"processes\"")!=std::string::npos);assert(js.find("\"ram_total_kib\"")!=std::string::npos);
 std::cout<<"MemBoost core tests passed\n";
}
