#include "core.hpp"
#include <iostream>
#include <string>
#ifndef MEMBOOST_VERSION
#define MEMBOOST_VERSION "development"
#endif
#include <unistd.h>
#ifdef MEMBOOST_WITH_X11
int gui_main(int argc,char** argv);
#endif
static void usage(){std::cout<<"MemBoost Pro v" MEMBOOST_VERSION "\nUsage: memboost [--status|--json|--recommend|--gui]\n       memboost --apply balanced|responsive|compressed-swap --dry-run\n       pkexec memboost --apply PROFILE --yes\n\nNo system changes occur without --apply PROFILE --yes and root privileges.\n";}
int main(int argc,char** argv){
 std::string command=argc>1?argv[1]:"--gui";
 if(command=="--version"){std::cout<<"MemBoost Pro " MEMBOOST_VERSION "\n";return 0;}
 if(command=="--help"||command=="-h"){usage();return 0;}
 if(command=="--status"||command=="--json"){auto s=mb::collect();if(command=="--json")std::cout<<mb::json(s)<<"\n";else std::cout<<mb::report(s);return s.memory.valid?0:2;}
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
