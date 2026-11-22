#include "core.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <random>
#include <limits>
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
 

 {
  auto c=mb::parse_cgroup("52428800\n","104857600\n","4096\n","max\n");
  assert(c.available&&c.limited&&c.current_bytes==52428800&&c.limit_bytes==104857600);
  assert(c.swap_available&&!c.swap_limited);
  auto unlimited=mb::parse_cgroup("15","max","","max");
  assert(unlimited.available&&!unlimited.limited&&!unlimited.swap_available);
  auto invalid=mb::parse_cgroup("-1","100","0","100");assert(!invalid.available);
  invalid=mb::parse_cgroup("18446744073709551616","100","0","100");assert(!invalid.available);
 }
 {
  mb::Snapshot a;a.memory.valid=true;a.memory.total=1048576;a.memory.available=524288;
  a.pressure.available=true;a.pressure.some10=30.0;
  auto d=mb::diagnose(a);assert(d.level=="high"&&!d.findings.empty());
  a.pressure.some10=12.0;d=mb::diagnose(a);assert(d.level=="elevated");
  a.pressure.some10=1.0;d=mb::diagnose(a);assert(d.level=="normal");
  a.pressure.available=false;a.cgroup.available=true;a.cgroup.limited=true;
  a.cgroup.current_bytes=95;a.cgroup.limit_bytes=100;
  d=mb::diagnose(a);assert(d.level=="high");
  a.cgroup.current_bytes=85;d=mb::diagnose(a);assert(d.level=="elevated");
  a.cgroup.current_bytes=40;d=mb::diagnose(a);assert(d.level=="normal");
  assert(mb::diagnostic_report(a).find("Cgroup v2")!=std::string::npos);
  a.memory.valid=false;d=mb::diagnose(a);assert(d.level=="unknown");
 }
 {
  std::istringstream v("pgmajfault 101\npswpin 17\npswpout 19\noom_kill 2\nunknown 999\n");
  auto counters=mb::parse_vmstat(v);
  assert(counters.available&&counters.oom_available);
  assert(counters.major_faults==101&&counters.swap_in==17&&counters.swap_out==19&&counters.oom_kills==2);
  std::istringstream damaged("pgmajfault -1\npswpin xx\npswpout 4\noom_kill 18446744073709551616\n");
  auto bad=mb::parse_vmstat(damaged);assert(!bad.available&&!bad.oom_available);
  std::istringstream old_kernel("pgmajfault 9\npswpin 4\npswpout 3\n");
  auto optional=mb::parse_vmstat(old_kernel);assert(optional.available&&!optional.oom_available);
 }
 {
  std::istringstream e("low 1\nhigh 2\nmax 3\noom 4\noom_kill 5\n");
  auto events=mb::parse_cgroup_events(e);assert(events.available&&events.high==2&&events.oom_kill==5);
  std::istringstream incomplete("high 8\noom invalid\noom_kill 2\n");
  assert(!mb::parse_cgroup_events(incomplete).available);
 }
 {
  mb::Snapshot a,b;a.vmstat={true,true,100,20,30,1};b.vmstat={true,true,120,24,35,2};
  a.cgroup_events={true,0,5,0,2,1};b.cgroup_events={true,0,7,0,3,2};
  auto diff=mb::telemetry_delta(a,b,2.0);
  assert(diff.vm_available&&diff.cgroup_available&&!diff.reset_detected);
  assert(diff.major_faults==20&&diff.swap_in==4&&diff.swap_out==5&&diff.oom_kills==1);
  assert(diff.cgroup_high==2&&diff.cgroup_oom==1&&diff.cgroup_oom_kill==1);
  assert(mb::signals_report(b,diff).find("major faults/s=10.00")!=std::string::npos);
  assert(mb::telemetry_json(b,diff,2).find("\"telemetry_schema\":1")!=std::string::npos);
  assert(!mb::telemetry_delta(a,b,0).vm_available);
  assert(!mb::telemetry_delta(a,b,-1).vm_available);
  assert(!mb::telemetry_delta(a,b,std::numeric_limits<double>::infinity()).vm_available);
  b.vmstat.major_faults=1;b.cgroup_events.oom=0;
  diff=mb::telemetry_delta(a,b,1);
  assert(!diff.vm_available&&!diff.cgroup_available&&diff.reset_detected);
  assert(diff.major_faults==0&&diff.cgroup_oom==0);
  b.vmstat.available=false;b.cgroup_events.available=false;
  diff=mb::telemetry_delta(a,b,1);assert(!diff.vm_available&&!diff.cgroup_available);
 }
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
 auto json=mb::json(escaped);assert(json.find("\"schema_version\":3")!=std::string::npos);
 assert(json.find("\"diagnosis\"")!=std::string::npos);assert(json.find("foo\\\"\\\\bar\\u000a")!=std::string::npos);

 std::cout<<"MemBoost core tests passed\n";
}
