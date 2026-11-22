#include "core.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <limits>
#ifndef MEMBOOST_VERSION
#define MEMBOOST_VERSION "development"
#endif
#include <unistd.h>
#ifdef MEMBOOST_WITH_X11
int gui_main(int argc,char** argv);
#endif
static void usage(){std::cout<<"MemBoost Pro v" MEMBOOST_VERSION "\nUsage: memboost [--status|--json|--diagnose|--recommend|--self-test|--gui]\n       memboost --watch [--samples 1..60] [--interval-ms 200..60000]\n       memboost --apply balanced|responsive|compressed-swap --dry-run\n       pkexec memboost --apply PROFILE --yes\n\nNo system changes occur without --apply PROFILE --yes and root privileges.\n";}
int main(int argc,char** argv){
 std::string command=argc>1?argv[1]:"--gui";
 if((command!="--apply" && command!="--watch" && argc>2) || (command=="--apply" && argc>7)){std::cerr<<"Unexpected extra arguments\n";return 2;}
 if(command=="--version"){std::cout<<"MemBoost Pro " MEMBOOST_VERSION "\n";return 0;}
 if(command=="--help"||command=="-h"){usage();return 0;}
 if(command=="--status"||command=="--json"){auto s=mb::collect();if(command=="--json")std::cout<<mb::json(s)<<"\n";else std::cout<<mb::report(s);return s.memory.valid?0:2;}
 if(command=="--diagnose"){
  auto s=mb::collect();std::cout<<mb::diagnostic_report(s);return s.memory.valid?0:2;
 }
 if(command=="--watch"){
  int samples=5,interval=2000;
  if((argc-2)%2!=0){std::cerr<<"Watch parameters require values\n";return 2;}
  for(int i=2;i<argc;i+=2){
   std::string key=argv[i],value=argv[i+1];
   if(value.empty()||value.find_first_not_of("0123456789")!=std::string::npos){std::cerr<<"Invalid watch number\n";return 2;}
   int parsed=0;try{size_t end=0;auto v=std::stoll(value,&end);if(end!=value.size()||v>60000)throw std::out_of_range("watch");parsed=static_cast<int>(v);}catch(...){std::cerr<<"Invalid watch number\n";return 2;}
   if(key=="--samples")samples=parsed;
   else if(key=="--interval-ms")interval=parsed;
   else{std::cerr<<"Unknown watch option\n";return 2;}
  }
  if(samples<1||samples>60||interval<200||interval>60000){std::cerr<<"Watch settings outside safe limits\n";return 2;}
  for(int i=0;i<samples;i++){
   auto s=mb::collect();if(!s.memory.valid){std::cerr<<"Memory counters unavailable\n";return 2;}
   std::cout<<"Sample "<<(i+1)<<"/"<<samples<<"\n"<<mb::diagnostic_report(s)<<std::flush;
   if(i+1<samples)std::this_thread::sleep_for(std::chrono::milliseconds(interval));
  }return 0;
 }
 if(command=="--self-test"){
  auto s=mb::collect();
  bool ok=s.memory.valid&&s.memory.available<=s.memory.total&&s.memory.swap_free<=s.memory.swap_total;
  const auto numeric=[](const std::string& value){return !value.empty()&&value.find_first_not_of("0123456789")==std::string::npos;};
  ok=ok&&numeric(s.swappiness)&&numeric(s.vfs_cache_pressure);
  std::cout<<"MemBoost Pro self-test: "<<(ok?"PASS":"FAIL")<<"\n";
  std::cout<<"procfs memory: "<<(s.memory.valid?"available":"unavailable or invalid")<<"\n";
  std::cout<<"PSI memory: "<<(s.pressure.available?"supported":"not available (optional)")<<"\n";
  std::cout<<"Memory tuning interface: "<<((numeric(s.swappiness)&&numeric(s.vfs_cache_pressure))?"readable":"unavailable")<<"\n";
  std::cout<<"No system settings were changed.\n";
  return ok?0:2;
 }
 if(command=="--recommend"){auto s=mb::collect();std::cout<<mb::report(s)<<"\nAvailable profiles (recommendations, never automatic):\n";for(auto& p:mb::profiles())std::cout<<"  "<<p.name<<": "<<p.explanation<<"\n";return s.memory.valid?0:2;}
 if(command=="--apply"){
  if(argc<3){usage();return 2;}const std::string name=argv[2];bool yes=false,dry=false;
  for(int i=3;i<argc;i++){std::string s=argv[i];if(s=="--yes")yes=true;else if(s=="--dry-run")dry=true;else{std::cerr<<"Unknown option: "<<s<<"\n";return 2;}}
  for(auto& p:mb::profiles())if(p.name==name){if(!dry&&!yes){std::cerr<<"Refusing changes without explicit --yes\n";return 2;}auto r=mb::apply_profile(p,"/proc",dry);std::cout<<r.message<<"\n";return r.ok?0:1;}
  std::cerr<<"Unknown profile: "<<name<<"\n";return 2;
 }
 if(command=="--gui"){
#ifdef MEMBOOST_WITH_X11
  return gui_main(argc,argv);
#else
  std::cerr<<"GUI unavailable in this build. Use --status or --recommend.\n";return 3;
#endif
 }
 usage();return 2;
}
