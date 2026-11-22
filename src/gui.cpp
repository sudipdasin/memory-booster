#include "core.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <deque>
#include <iostream>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {
constexpr int W=1040,H=720;
struct App {
 Display* d=nullptr;Window w=0;GC gc=0;XFontStruct* font=nullptr;Atom close=0;
 unsigned long bg,side,card,text,muted,teal,blue,amber,border,red;
 int width=W,height=H,tab=0,profile=0;bool confirming=false;pid_t child=-1;
 std::string message="Read-only monitoring active. System settings are unchanged.";
 mb::Snapshot snap;std::deque<double> history;
 unsigned long color(const char* name){XColor c,e;if(XAllocNamedColor(d,DefaultColormap(d,DefaultScreen(d)),name,&c,&e))return c.pixel;return BlackPixel(d,DefaultScreen(d));}
 void fill(int x,int y,int ww,int hh,unsigned long c){XSetForeground(d,gc,c);XFillRectangle(d,w,gc,x,y,std::max(0,ww),std::max(0,hh));}
 void outline(int x,int y,int ww,int hh,unsigned long c){XSetForeground(d,gc,c);XDrawRectangle(d,w,gc,x,y,std::max(0,ww-1),std::max(0,hh-1));}
 void label(int x,int y,const std::string& s,unsigned long c){XSetForeground(d,gc,c);XDrawString(d,w,gc,x,y,s.c_str(),static_cast<int>(s.size()));}
 void box(int x,int y,int ww,int hh,const char* title){fill(x,y,ww,hh,card);outline(x,y,ww,hh,border);label(x+18,y+30,title,muted);}
 void button(int x,int y,int ww,int hh,const std::string& s,bool active=false){fill(x,y,ww,hh,active?teal:side);outline(x,y,ww,hh,active?teal:border);label(x+15,y+hh/2+5,s,active?bg:text);}
 static std::string pct(uint64_t used,uint64_t tot){if(!tot)return "N/A";return std::to_string((unsigned)(100.0*used/tot+0.5))+"%";}
 static std::string gib(uint64_t kb){char b[40];std::snprintf(b,sizeof(b),"%.2f GiB",kb/1048576.0);return b;}
 void gauge(int x,int y,int ww,int hh,double share,unsigned long c){fill(x,y,ww,hh,side);fill(x,y,static_cast<int>(ww*std::clamp(share,0.0,1.0)),hh,c);}
 void tabs(){fill(0,0,width,84,side);label(32,34,"MEMBOOST  PRO",teal);label(32,59,"Native Linux memory diagnostics",muted);
  const char* labels[]={"Overview","Processes","Profiles","Setup guide","Activity"};for(int i=0;i<5;i++)button(330+i*137,22,127,40,labels[i],tab==i);
 }
 void overview(){auto& m=snap.memory;int half=(width-76)/2;
 box(26,104,half,172,"PHYSICAL MEMORY");box(50+half,104,half,172,"SWAP");
 label(46,156,gib(m.total-m.available)+"  used",text);label(46,184,gib(m.available)+"  available of "+gib(m.total),muted);
 gauge(46,212,half-40,17,m.total?double(m.total-m.available)/m.total:0,teal);
 label(70+half,156,gib(m.swap_total-m.swap_free)+"  used",text);label(70+half,184,gib(m.swap_total)+"  capacity",muted);
 gauge(70+half,212,half-40,17,m.swap_total?double(m.swap_total-m.swap_free)/m.swap_total:0,blue);
 box(26,294,width-52,213,"LIVE RAM USAGE / 2s samples");
 int gx=45,gy=341,gw=width-90,gh=135;for(int j=0;j<4;j++){XSetForeground(d,gc,border);XDrawLine(d,w,gc,gx,gy+j*gh/3,gx+gw,gy+j*gh/3);}
 if(history.size()>1){XSetForeground(d,gc,teal);for(size_t i=1;i<history.size();i++){int x1=gx+(i-1)*gw/119,x2=gx+i*gw/119;int y1=gy+gh-int(history[i-1]*gh),y2=gy+gh-int(history[i]*gh);XDrawLine(d,w,gc,x1,y1,x2,y2);}}
 box(26,525,half,132,"MEMORY PRESSURE (PSI)");box(50+half,525,half,132,"KERNEL & COMPRESSION");
 if(snap.pressure.available){label(46,571,"Some: "+std::to_string(snap.pressure.some10).substr(0,5)+"%   Full: "+std::to_string(snap.pressure.full10).substr(0,5)+"%",text);label(46,598,"10-second stall averages",muted);}else label(46,580,"PSI unavailable on this kernel",muted);
 label(70+half,569,"Swappiness: "+snap.swappiness+"    VFS: "+snap.vfs_cache_pressure,text);label(70+half,598,"zram devices detected: "+std::to_string(snap.zram.size()),muted);
 }
 void processes(){box(26,104,width-52,553,"TOP MEMORY CONSUMERS  /  resident set size");label(54,158,"PID",muted);label(155,158,"PROCESS",muted);label(width-205,158,"RESIDENT RAM",muted);
 int yy=189;for(auto& p:snap.processes){fill(43,yy-24,width-87,40,(yy/40)%2?card:side);label(54,yy,std::to_string(p.pid),text);label(155,yy,p.name.substr(0,42),text);label(width-205,yy,std::to_string(p.rss_kib/1024)+" MiB",teal);yy+=42;if(yy>640)break;}
 label(54,636,"Process list is monitoring-only; no processes are terminated.",muted);
 }
 void profiles(){box(26,104,width-52,420,"SELECT A REVERSIBLE, TEMPORARY TUNING PROFILE");
 auto ps=mb::profiles();for(int i=0;i<3;i++){int y=160+i*91;fill(48,y,width-96,78,i==profile?side:card);outline(48,y,width-96,78,i==profile?teal:border);label(73,y+29,ps[i].name+"     swappiness="+std::to_string(ps[i].swappiness)+"  vfs_cache_pressure="+std::to_string(ps[i].vfs_cache_pressure),text);label(73,y+56,ps[i].explanation,muted);}
 button(48,453,270,48,"Apply selected profile",true);label(337,473,"Requires administrator authorization; confirmation follows.",muted);
 box(26,546,width-52,110,"SAFETY GUARANTEES");label(48,588,"No automatic cache dropping, swapoff, swap recreation, service restarts or startup changes.",text);
 label(48,614,"Changes are read back and verified. Partial failure triggers rollback. Reboot resets tuning.",muted);
 }
 void guide(){box(26,104,width-52,553,"GUIDED MEMORY OPTIMIZATION  /  5 STEPS");
 const char* steps[]={"1   Inspect RAM availability, not simply free RAM.","2   Check sustained PSI stall pressure and your highest-RSS processes.","3   Review whether your distribution already manages zram/zswap.","4   Choose a profile on the Profiles tab; test with a representative workload.","5   Compare memory pressure and latency; reboot to restore kernel defaults."};
 for(int i=0;i<5;i++){fill(48,153+i*82,width-96,69,side);label(70,193+i*82,steps[i],text);}
 label(48,597,"MemBoost Pro does not promise to 'double RAM' or reclaim useful Linux page cache.",amber);
 label(48,622,"Compression and OOM management vary by kernel and distribution and are never auto-enabled.",muted);
 }
 void activity(){box(26,104,width-52,553,"STATUS & EVENT FEEDBACK");label(50,172,"CURRENT SESSION",muted);label(50,210,message,text);
 label(50,283,"Read-only observation runs under your normal user account.",muted);
 label(50,320,"Explicit changes apply only to vm.swappiness and vm.vfs_cache_pressure.",muted);
 label(50,357,"System modifications are NOT persisted to boot.",muted);
 label(50,394,"For advanced operations (zram, zswap, swap, oomd), use your distro's tools.",muted);
 }
 void modal(){fill(width/2-325,height/2-120,650,240,side);outline(width/2-325,height/2-120,650,240,amber);
 label(width/2-293,height/2-72,"Confirm kernel memory tuning",text);auto p=mb::profiles()[profile];
 label(width/2-293,height/2-30,"Apply profile: "+p.name+"?",text);label(width/2-293,height/2+2,"Only swappiness and vfs_cache_pressure will change.",muted);
 button(width/2-280,height/2+40,200,44,"Cancel");button(width/2+50,height/2+40,230,44,"Apply (polkit)",true);
 }
 void draw(){fill(0,0,width,height,bg);tabs();if(tab==0)overview();else if(tab==1)processes();else if(tab==2)profiles();else if(tab==3)guide();else activity();
 fill(0,height-43,width,43,side);label(27,height-17,child>0?"Authorization / tuning in progress...":message.substr(0,115),child>0?amber:muted);if(confirming)modal();XFlush(d);
 }
 void sample(){snap=mb::collect();double share=snap.memory.total?double(snap.memory.total-snap.memory.available)/snap.memory.total:0;history.push_back(share);if(history.size()>120)history.pop_front();}
 void apply(){if(child>0){message="An action is already in progress.";return;}auto p=mb::profiles()[profile];
 if(geteuid()==0){auto r=mb::apply_profile(p);message=r.message;sample();return;}
 pid_t pid=fork();if(pid<0){message="Could not create authorization process";return;}
 if(pid==0){char self[4096];auto n=readlink("/proc/self/exe",self,sizeof(self)-1);if(n<0)_exit(127);self[n]='\0';execlp("pkexec","pkexec",self,"--apply",p.name.c_str(),"--yes",(char*)nullptr);_exit(127);}
 child=pid;message="Waiting for administrator authorization...";
 }
 void click(int x,int y){if(confirming){if(y>=height/2+40&&y<height/2+84){if(x>=width/2-280&&x<width/2-80)confirming=false;else if(x>=width/2+50&&x<width/2+280){confirming=false;apply();}}return;}
 if(y>=22&&y<=62){for(int i=0;i<5;i++)if(x>=330+i*137&&x<457+i*137){tab=i;return;}}
 if(tab==2){for(int i=0;i<3;i++)if(x>=48&&x<width-48&&y>=160+i*91&&y<238+i*91){profile=i;return;}
  if(x>=48&&x<=318&&y>=453&&y<501){confirming=true;return;}}
 }
 int run(){d=XOpenDisplay(nullptr);if(!d){std::cerr<<"Cannot open X11 display. Use memboost --status for headless mode.\n";return 3;}
 int screen=DefaultScreen(d);w=XCreateSimpleWindow(d,RootWindow(d,screen),80,60,W,H,0,0,0);
 XStoreName(d,w,"MemBoost Pro - Linux Memory Optimizer");XSelectInput(d,w,ExposureMask|StructureNotifyMask|ButtonPressMask|KeyPressMask);
 close=XInternAtom(d,"WM_DELETE_WINDOW",False);XSetWMProtocols(d,w,&close,1);gc=XCreateGC(d,w,0,nullptr);
 font=XLoadQueryFont(d,"9x15");if(!font)font=XLoadQueryFont(d,"fixed");if(font)XSetFont(d,gc,font->fid);
 bg=color("#101827");side=color("#1a2537");card=color("#172132");text=color("#e7edf7");muted=color("#a5b6cb");teal=color("#54dfc4");blue=color("#67abff");amber=color("#ffc477");border=color("#32445a");red=color("#ff8585");
 XSizeHints hints{};hints.flags=PMinSize;hints.min_width=900;hints.min_height=680;XSetWMNormalHints(d,w,&hints);
 XMapWindow(d,w);sample();draw();bool running=true;
 while(running){struct pollfd pfd{ConnectionNumber(d),POLLIN,0};poll(&pfd,1,2000);
  while(XPending(d)){XEvent ev;XNextEvent(d,&ev);if(ev.type==ClientMessage&&static_cast<Atom>(ev.xclient.data.l[0])==close)running=false;
   else if(ev.type==ConfigureNotify){width=ev.xconfigure.width;height=ev.xconfigure.height;}
   else if(ev.type==ButtonPress&&ev.xbutton.button==Button1)click(ev.xbutton.x,ev.xbutton.y);
   else if(ev.type==KeyPress){char b[16];KeySym k;XLookupString(&ev.xkey,b,sizeof(b),&k,nullptr);if(k==XK_Escape)confirming=false;}
  }
  if(child>0){int status=0;pid_t r=waitpid(child,&status,WNOHANG);if(r==child){child=-1;if(WIFEXITED(status)&&WEXITSTATUS(status)==0)message="Profile applied and verified. Reboot restores prior kernel defaults.";else message="Tuning not applied: authorization was cancelled or the operation failed. Check CLI output.";}}
  sample();draw();
 }
 if(child>0){/* Do not kill a polkit authorization workflow merely because the UI exits. */}
 if(font)XFreeFont(d,font);XFreeGC(d,gc);XDestroyWindow(d,w);XCloseDisplay(d);return 0;
 }
};
}
int gui_main(int,char**){App a;return a.run();}
