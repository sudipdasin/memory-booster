// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 MemBoost Pro contributors
// Premium native X11 dashboard. Xft is loaded at runtime for anti-aliased type,
// with a core-X11 text fallback when a desktop does not ship libXft.
#include "core.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <dlfcn.h>
#include <map>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

namespace {
constexpr int W=1280,H=800,SB=238;
constexpr double PI=3.141592653589793;
struct XftPaint { unsigned long pixel; struct {unsigned short red,green,blue,alpha;} color; };
struct XftGlyph {unsigned short width,height;short x,y,xoff,yoff;};
class Fonts {
 using Create=void* (*)(Display*,Drawable,Visual*,Colormap);
 using Destroy=void (*)(void*);
 using Open=void* (*)(Display*,int,const char*);
 using Close=void (*)(Display*,void*);
 using Text=void (*)(void*,const XftPaint*,void*,int,int,const unsigned char*,int);
 using Measure=void (*)(Display*,void*,const unsigned char*,int,XftGlyph*);
 void* handle_=nullptr;void* target_=nullptr;std::array<void*,9> fonts_{};
 Create create_=nullptr;Destroy destroy_=nullptr;Open open_=nullptr;Close close_=nullptr;
 Text text_=nullptr;Measure measure_=nullptr;Display* d_=nullptr;
 XFontStruct* fallback_=nullptr;
public:
 enum Size{tiny,small,regular,medium,subtitle,title,display,metric,button};
 void initialize(Display* d,Window w){
  d_=d;fallback_=XLoadQueryFont(d,"9x15");if(!fallback_)fallback_=XLoadQueryFont(d,"fixed");
  handle_=dlopen("libXft.so.2",RTLD_LAZY|RTLD_LOCAL);
  if(!handle_)return;
  create_=reinterpret_cast<Create>(dlsym(handle_,"XftDrawCreate"));
  destroy_=reinterpret_cast<Destroy>(dlsym(handle_,"XftDrawDestroy"));
  open_=reinterpret_cast<Open>(dlsym(handle_,"XftFontOpenName"));
  close_=reinterpret_cast<Close>(dlsym(handle_,"XftFontClose"));
  text_=reinterpret_cast<Text>(dlsym(handle_,"XftDrawStringUtf8"));
  measure_=reinterpret_cast<Measure>(dlsym(handle_,"XftTextExtentsUtf8"));
  if(!create_||!destroy_||!open_||!close_||!text_||!measure_)return;
  const char* names[]={"DejaVu Sans:size=9","DejaVu Sans:size=10.5","DejaVu Sans:size=12",
    "DejaVu Sans:style=Bold:size=12","DejaVu Sans:style=Bold:size=15",
    "DejaVu Sans:style=Bold:size=22","DejaVu Sans:style=Bold:size=29",
    "DejaVu Sans:style=Bold:size=26","DejaVu Sans:style=Bold:size=14"};
  for(int i=0;i<9;i++)fonts_[i]=open_(d,DefaultScreen(d),names[i]);
  target_=create_(d,w,DefaultVisual(d,DefaultScreen(d)),DefaultColormap(d,DefaultScreen(d)));
 }
 void paint(Display* d,Window w,GC gc,int x,int y,const std::string& raw,Size size,
            const XftPaint& color,int max_width=0){
  std::string s=raw;
  if(max_width>0){
   while(!s.empty()&&width(d,s,size)>max_width-10)s.pop_back();
   if(s.size()<raw.size()&&s.size()>=3)s.replace(s.size()-3,3,"...");
  }
  if(target_&&fonts_[size])text_(target_,&color,fonts_[size],x,y,reinterpret_cast<const unsigned char*>(s.c_str()),(int)s.size());
  else if(fallback_){XSetFont(d,gc,fallback_->fid);XSetForeground(d,gc,color.pixel);
   XDrawString(d,w,gc,x,y,s.c_str(),(int)s.size());}
 }
 int width(Display* d,const std::string& s,Size size){
  if(target_&&fonts_[size]){XftGlyph g{};measure_(d,fonts_[size],reinterpret_cast<const unsigned char*>(s.c_str()),(int)s.size(),&g);return g.xoff;}
  return fallback_?XTextWidth(fallback_,s.c_str(),(int)s.size()):(int)s.size()*8;
 }
 void finish(){if(close_&&d_)for(auto f:fonts_)if(f)close_(d_,f);if(destroy_&&target_)destroy_(target_);
  if(fallback_)XFreeFont(d_,fallback_);/* Keep Xft mapped until process exit: Xlib/Fontconfig may retain hooks. */}
};
struct Color {uint16_t red=0,green=0,blue=0;unsigned long pixel=0;XftPaint xft{};};
struct App {
 Display* d=nullptr;Window win=0;GC gc=0;Atom close_atom=0;Fonts fonts;int width=W,height=H,tab=0,profile=0,modal=0;
 bool alive=true;pid_t child=-1;std::string message="Monitoring is active - kernel settings unchanged.";
 std::map<std::string,Color> colors;
 std::deque<double> history;mb::Snapshot snapshot;mb::TelemetryDelta delta;bool sampled=false;
 std::chrono::steady_clock::time_point sample_time,redraw_time;
 Color col(const std::string& h){auto it=colors.find(h);if(it!=colors.end())return it->second;
  Color v;unsigned rr=0,gg=0,bb=0;
  if(std::sscanf(h.c_str(),"#%02x%02x%02x",&rr,&gg,&bb)!=3){rr=255;gg=255;bb=255;}
  v.red=rr*257;v.green=gg*257;v.blue=bb*257;
  XColor c{};c.red=v.red;c.green=v.green;c.blue=v.blue;c.flags=DoRed|DoGreen|DoBlue;
  if(XAllocColor(d,DefaultColormap(d,DefaultScreen(d)),&c))v.pixel=c.pixel;
  else v.pixel=WhitePixel(d,DefaultScreen(d));
  v.xft={v.pixel,{v.red,v.green,v.blue,65535}};colors[h]=v;return v;
 }
 void paint(int x,int y,const std::string& s,Fonts::Size f,const char* hex,int bound=0){fonts.paint(d,win,gc,x,y,s,f,col(hex).xft,bound);}
 void rect(int x,int y,int w,int h,const char* c){if(w<=0||h<=0)return;XSetForeground(d,gc,col(c).pixel);XFillRectangle(d,win,gc,x,y,w,h);}
 void line(int x1,int y1,int x2,int y2,const char* c,int t=1){XSetForeground(d,gc,col(c).pixel);XSetLineAttributes(d,gc,t,LineSolid,CapRound,JoinRound);XDrawLine(d,win,gc,x1,y1,x2,y2);XSetLineAttributes(d,gc,1,LineSolid,CapButt,JoinMiter);}
 void disk(int x,int y,int r,const char* c){XSetForeground(d,gc,col(c).pixel);XFillArc(d,win,gc,x-r,y-r,2*r,2*r,0,360*64);}
 void rounded(int x,int y,int w,int h,int r,const char* c){if(w<=0||h<=0)return;r=std::max(0,std::min({r,w/2,h/2}));if(!r){rect(x,y,w,h,c);return;}
  XSetForeground(d,gc,col(c).pixel);XFillRectangle(d,win,gc,x+r,y,w-2*r,h);XFillRectangle(d,win,gc,x,y+r,w,h-2*r);
  for(const auto& a:std::array<std::array<int,3>,4>{{{x,y,90},{x+w-2*r,y,0},{x,y+h-2*r,180},{x+w-2*r,y+h-2*r,270}}})
   XFillArc(d,win,gc,a[0],a[1],2*r,2*r,a[2]*64,90*64);
 }
 void gradient(int x,int y,int w,int h,const std::string& start,const std::string& end){
  auto a=col(start),b=col(end);const int strips=24;
  for(int i=0;i<strips;i++){
   int yy=y+i*h/strips,hh=y+(i+1)*h/strips-yy;
   auto mix=[&](unsigned x,unsigned z){return (unsigned)((x*(strips-i)+z*i)/strips/257);};
   char hex[8];std::snprintf(hex,sizeof(hex),"#%02x%02x%02x",mix(a.red,b.red),mix(a.green,b.green),mix(a.blue,b.blue));rect(x,yy,w,hh+1,hex);
  }
 }
 void panel(int x,int y,int w,int h,const char* bg="#121E33"){
  rounded(x,y+4,w,h,20,"#090F1E");rounded(x,y,w,h-1,20,bg);
 }
 void pill(int x,int y,int w,int h,const char* bg){rounded(x,y,w,h,h/2,bg);}
 void bar(int x,int y,int w,int h,double fraction,const char* base="#263B50",const char* color="#62E4C1"){
  rounded(x,y,w,h,h/2,base);const int val=(int)std::round(w*std::clamp(fraction,0.,1.));if(val>1)rounded(x,y,val,h,h/2,color);
 }
 void circle_meter(int cx,int cy,int radius,double fraction,const char* foreground){
  XSetLineAttributes(d,gc,12,LineSolid,CapRound,JoinRound);
  XSetForeground(d,gc,col("#243850").pixel);XDrawArc(d,win,gc,cx-radius,cy-radius,radius*2,radius*2,0,360*64);
  XSetForeground(d,gc,col(foreground).pixel);XDrawArc(d,win,gc,cx-radius,cy-radius,radius*2,radius*2,90*64,-(int)(std::clamp(fraction,0.,1.)*360*64));
  XSetLineAttributes(d,gc,1,LineSolid,CapButt,JoinMiter);
 }
 std::string amount(uint64_t kib){char out[50];std::snprintf(out,sizeof(out),"%.1f GB",kib/1048576.0);return out;}
 std::string num(uint64_t v){return std::to_string(v);}
 std::string fmt(double n,int dp=1){char out[50];std::snprintf(out,sizeof(out),dp==1?"%.1f":"%.2f",n);return out;}
 int cx()const{return SB+34;}
 int content_w()const{return width-SB-66;}
 static constexpr int nav_y=214,nav_step=57;
 const char* tablabel(int i)const{return std::array<const char*,7>{"Dashboard","Processes","Diagnosis","Telemetry","Profiles","Setup Guide","Activity"}[std::clamp(i,0,6)];}
 const char* tabicon(int i)const{return std::array<const char*,7>{"D","P","+","T","S","?","L"}[std::clamp(i,0,6)];}
 void sidebar(){rect(0,0,SB,height,"#0C1728");rect(SB-1,0,1,height,"#283751");
  rounded(26,28,46,46,13,"#173E45");paint(40,59,"M",Fonts::title,"#70EDCC");
  paint(86,48,"MemBoost",Fonts::subtitle,"#F5F8FF");paint(86,67,"PRO  /  COMMUNITY",Fonts::tiny,"#6CDEC7");
  line(23,98,SB-23,98,"#26364B");paint(26,143,"YOUR WORKSPACE",Fonts::tiny,"#7790AB");
  paint(26,183,"SYSTEM TOOLS",Fonts::small,"#B1C1D4");
  for(int i=0;i<7;i++){
   int y=nav_y+i*nav_step;bool active=(i==tab);
   if(active){rounded(16,y-24,SB-32,47,13,"#153D43");rect(16,y-18,3,33,"#63E7C3");}
   rounded(29,y-15,28,28,8,active?"#20575A":"#1B2A3B");
   paint(39,y+4,tabicon(i),Fonts::small,active?"#7BF0D0":"#879DB5");
   paint(73,y+5,tablabel(i),active?Fonts::medium:Fonts::regular,active?"#F1FFFB":"#A4B6CC",SB-93);
  }
  rounded(19,height-144,SB-39,112,15,"#14263A");
  disk(43,height-120,6,"#5DDBB8");paint(60,height-115,"LOCAL & PRIVATE",Fonts::small,"#D6F1E9");
  paint(33,height-87,"No background uploads",Fonts::small,"#96AAC1");
  paint(33,height-64,"GPL-3.0-or-later",Fonts::small,"#96AAC1");
 }
 void topbar(){rect(SB+1,0,width-SB-1,89,"#0B1527");line(SB+1,89,width,89,"#25374A");
  paint(cx(),39,tablabel(tab),Fonts::subtitle,"#F4F8FF");
  paint(cx(),64,"Memory intelligence and safe performance tuning",Fonts::small,"#91A8C0");
  pill(width-201,24,164,34,"#14372F");disk(width-182,40,4,"#66E7C0");
  paint(width-168,46,"LIVE MONITORING",Fonts::small,"#91F0D2");
 }
 void footer(){rect(SB+1,height-45,width-SB-1,45,"#0C1728");line(SB+1,height-45,width,height-45,"#26364C");
  disk(cx()+4,height-22,4,child>0?"#FFBD70":"#6AE6BE");
  paint(cx()+19,height-17,child>0?"Awaiting administrator authorization...":message,Fonts::small,"#9AAEC5",width-SB-88);
 }
 void hero_button(int x,int y,int w,int h){rounded(x,y+5,w,h,17,"#0C3C3D");rounded(x,y,w,h-2,17,"#6BE7C5");
  disk(x+29,y+h/2-1,15,"#1F7667");paint(x+22,y+h/2+4,"+",Fonts::button,"#FFFFFF");
  paint(x+56,y+h/2+7,"OPTIMIZE MEMORY",Fonts::button,"#082B2E");
 }
 void dashboard(){const int x=cx(),cw=content_w(),y=113;
  panel(x,y,cw,219,"#153047");
  rounded(x+1,y+1,cw-2,216,19,"#132D44");
  rect(x+22,y+29,3,31,"#6DEAC7");
  pill(x+37,y+23,204,29,"#214A50");paint(x+52,y+43,"SMART MEMORY ASSISTANT",Fonts::tiny,"#A4F6DB");
  paint(x+33,y+111,"Your memory, in control.",Fonts::title,"#F7FCFF",cw-350);
  paint(x+34,y+145,"Health-aware analysis. No blind cache clearing or forced process termination.",Fonts::small,"#B1CFD9",cw-362);
  disk(x+cw-77,y+53,20,"#295C59");disk(x+cw-77,y+53,7,"#70F2C6");
  hero_button(x+cw-314,y+103,270,64);
  paint(x+cw-307,y+188,"PREVIEW FIRST  /  EXPLICIT APPROVAL",Fonts::tiny,"#A8E4D4",260);
  int gap=18,cardW=(cw-2*gap)/3,cy=353,ch=150;
  panel(x,cy,cardW,ch);panel(x+cardW+gap,cy,cardW,ch);panel(x+2*(cardW+gap),cy,cardW,ch);
  const auto& mem=snapshot.memory;
  double used=mem.total?double(mem.total-mem.available)/mem.total:0.;
  paint(x+22,cy+35,"RAM USED",Fonts::small,"#94ACC7");
  paint(x+20,cy+87,mem.valid?amount(mem.total-mem.available):"Unavailable",Fonts::metric,"#F2F8FF",cardW-85);
  paint(x+23,cy+112,"of "+amount(mem.total)+" physical",Fonts::small,"#94ACC7");
  bar(x+22,cy+126,cardW-44,8,used,"#26394E","#5CE1C2");
  int sx=x+cardW+gap;
  paint(sx+22,cy+35,"AVAILABLE",Fonts::small,"#94ACC7");
  paint(sx+20,cy+87,mem.valid?amount(mem.available):"Unavailable",Fonts::metric,"#F2F8FF",cardW-50);
  paint(sx+23,cy+112,"Linux reclaimable headroom",Fonts::small,"#94ACC7");
  bar(sx+22,cy+126,cardW-44,8,1.-used,"#26394E","#73AFFF");
  sx=x+2*(cardW+gap);
  paint(sx+22,cy+35,"MEMORY PRESSURE",Fonts::small,"#94ACC7");
  auto diag=mb::diagnose(snapshot);
  const char* grade=diag.level=="high"?"HIGH":diag.level=="elevated"?"ELEVATED":diag.level=="normal"?"LOW":"UNKNOWN";
  paint(sx+20,cy+86,grade,Fonts::metric,diag.level=="high"?"#FF8792":"#80E8CE",cardW-50);
  paint(sx+23,cy+112,snapshot.pressure.available?"PSI stalls "+fmt(snapshot.pressure.some10)+"% / 10s":"PSI not available",Fonts::small,"#94ACC7");
  bar(sx+22,cy+126,cardW-44,8,snapshot.pressure.available?snapshot.pressure.some10/30.:0,"#26394E","#A196FA");
  int by=521,bh=height-by-64;
  int leftW=(int)(cw*0.66)-9,rightX=x+leftW+18,rightW=cw-leftW-18;
  panel(x,by,leftW,bh);panel(rightX,by,rightW,bh);
  paint(x+22,by+35,"Memory activity",Fonts::medium,"#E9F5FF");
  paint(x+22,by+58,"Available headroom tracked every 2 seconds",Fonts::small,"#8199B4");
  int gx=x+24,gy=by+89,gw=leftW-50,gh=bh-119;
  for(int i=0;i<4;i++)line(gx,gy+i*gh/3,gx+gw,gy+i*gh/3,"#284059");
  if(history.size()>1){
   for(size_t i=1;i<history.size();++i){
    int x1=gx+(int)((i-1)*gw/119),x2=gx+(int)(i*gw/119);
    int y1=gy+gh-(int)(history[i-1]*gh),y2=gy+gh-(int)(history[i]*gh);
    line(x1,y1,x2,y2,"#60E9C3",3);
   }
  }
  paint(rightX+22,by+35,"Optimization insight",Fonts::medium,"#E8F5FF");
  auto plan=mb::plan_optimization(snapshot);
  rounded(rightX+22,by+54,rightW-44,37,10,plan.can_apply?"#214936":"#193F41");
  paint(rightX+33,by+79,plan.can_apply?"OPTIONAL TUNING READY":"NO URGENT ACTION",Fonts::small,"#8DF3D6",rightW-64);
  paint(rightX+22,by+119,plan.headline,Fonts::regular,"#F4F7FF",rightW-41);
  paint(rightX+22,by+152,plan.can_apply?"Preview a temporary, verified profile.":"Monitor workload before changing tuning.",Fonts::small,"#9AB1C8",rightW-44);
  paint(rightX+22,by+181,"Nothing runs without your confirmation.",Fonts::small,"#9AB1C8",rightW-44);
 }
 void page_title(const char* title,const char* description){paint(cx()+7,144,title,Fonts::title,"#F3F8FF",content_w()-45);paint(cx()+7,177,description,Fonts::regular,"#90A7C2",content_w()-45);}
 void processes(){page_title("Know what's using your RAM","Explore the largest resident processes. Monitoring only - never force stopped.");
  int x=cx(),y=206,cw=content_w();panel(x,y,cw,height-y-72);
  paint(x+26,y+41,"PID",Fonts::small,"#7F9AB7");paint(x+166,y+41,"PROCESS",Fonts::small,"#7F9AB7");paint(x+cw-205,y+41,"RESIDENT MEMORY",Fonts::small,"#7F9AB7");
  line(x+20,y+60,x+cw-20,y+60,"#2D3E56");int row=y+93;
  for(size_t i=0;i<snapshot.processes.size()&&row<height-108;i++,row+=49){auto& p=snapshot.processes[i];
   if(i%2==0)rounded(x+18,row-29,cw-36,43,9,"#1B2A40");
   paint(x+28,row,std::to_string(p.pid),Fonts::regular,"#92ADC5");
   paint(x+166,row,p.name,Fonts::regular,"#EEF4FF",cw-400);
   paint(x+cw-200,row,std::to_string(p.rss_kib/1024)+" MB",Fonts::medium,"#79E3C9",174);
  }
 }
 void diagnosis(){page_title("Memory health intelligence","Actionable observations based on live PSI stalls and cgroup constraints.");
  int x=cx(),cw=content_w(),y=208;auto d=mb::diagnose(snapshot);
  int lw=(int)(cw*.53),rw=cw-lw-18;panel(x,y,lw,420);panel(x+lw+18,y,rw,420);
  paint(x+24,y+43,"CURRENT HEALTH",Fonts::small,"#9CB0C9");
  paint(x+24,y+111,d.level=="high"?"Attention needed":d.level=="elevated"?"Mild pressure":d.level=="normal"?"Looking good":"Data limited",Fonts::title,d.level=="high"?"#FF8891":"#86EDCE",lw-50);
  int pY=y+159;for(auto& finding:d.findings){paint(x+26,pY,"+",Fonts::medium,"#69DABB");paint(x+53,pY,finding,Fonts::regular,"#D3E3F3",lw-80);pY+=65;if(pY>y+379)break;}
  int rx=x+lw+18;paint(rx+24,y+43,"RECOMMENDED NEXT STEPS",Fonts::small,"#9CB0C9");
  pY=y+96;for(auto& advice:d.advice){disk(rx+30,pY-5,4,"#70E6C9");paint(rx+48,pY,advice,Fonts::regular,"#D0E1EF",rw-74);pY+=75;if(pY>y+375)break;}
  panel(x,648,cw,76);paint(x+24,695,"Linux page cache is useful: free RAM alone is not a reason to optimize.",Fonts::regular,"#E3B879",cw-50);
 }
 void telemetry(){page_title("Performance telemetry","Understand page faults, swap activity and out-of-memory events.");
  int x=cx(),cw=content_w(),y=210,half=(cw-18)/2;
  auto counter=[&](int xx,int yy,int ww,const char* name,const std::string& value,const char* caption){panel(xx,yy,ww,142);
   paint(xx+23,yy+34,name,Fonts::small,"#91ABC5");paint(xx+22,yy+89,value,Fonts::metric,"#DFFAF3",ww-48);paint(xx+24,yy+119,caption,Fonts::small,"#8C9FB7",ww-48);};
  counter(x,y,half,"MAJOR PAGE FAULTS",snapshot.vmstat.available?num(snapshot.vmstat.major_faults):"N/A","Cumulative since boot");
  counter(x+half+18,y,half,"SWAP OUT PAGES",snapshot.vmstat.available?num(snapshot.vmstat.swap_out):"N/A","Cumulative since boot");
  counter(x,y+164,half,"OOM EVENTS",snapshot.vmstat.oom_available?num(snapshot.vmstat.oom_kills):"N/A","Cumulative since boot");
  counter(x+half+18,y+164,half,"CGROUP HIGH EVENTS",snapshot.cgroup_events.available?num(snapshot.cgroup_events.high):"N/A","Current group cumulative");
  panel(x,y+349,cw,141);paint(x+25,y+389,"LATEST MEASURED INTERVAL",Fonts::small,"#91ABC5");
  const auto rate=[](uint64_t n,double sec){char out[30];std::snprintf(out,sizeof(out),"%.2f / sec",sec>0?n/sec:0.0);return std::string(out);};
  paint(x+25,y+441,delta.vm_available?"Major faults: "+rate(delta.major_faults,delta.seconds):"Waiting for the next valid sample...",Fonts::medium,"#72EBC9");
  paint(x+cw/2,y+441,delta.vm_available?"Swap out: "+rate(delta.swap_out,delta.seconds):"No rate inferred from one sample",Fonts::medium,"#ABC0DB");
 }
 void profiles(){page_title("Fine-tune with confidence","Profiles are temporary and require explicit administrator authorization.");
  int x=cx(),cw=content_w(),y=214;const auto profiles=mb::profiles();
  for(int i=0;i<3;i++){int yy=y+i*123;panel(x,yy,cw,108,profile==i?"#183C43":"#132238");
   disk(x+41,yy+51,14,profile==i?"#65E6BD":"#264258");
   if(profile==i)disk(x+41,yy+51,6,"#12392F");
   paint(x+73,yy+39,profiles[i].name,Fonts::medium,"#F1F8FF");
   paint(x+73,yy+68,profiles[i].explanation,Fonts::regular,"#A1B7CD",cw-365);
   paint(x+cw-267,yy+61,"SWAP "+std::to_string(profiles[i].swappiness)+"  /  VFS "+std::to_string(profiles[i].vfs_cache_pressure),Fonts::small,"#77DBBF",250);
  }
  rounded(x,y+391,256,55,14,"#5CDCB8");paint(x+25,y+427,"PREVIEW PROFILE CHANGE",Fonts::medium,"#0C2C2F");
  paint(x+287,y+422,"No settings are changed until you confirm.",Fonts::small,"#92ADC4");
 }
 void guide(){page_title("Optimization playbook","A safer way to improve responsiveness: inspect, measure, then act.");
  const char* steps[]={"Monitor freeable RAM, not only used RAM.","Check PSI memory stalls over a representative workload.","Find unusually large resident processes without force killing.","Review zram or other compression configured by your distribution.","Only apply reversible settings after explicit confirmation."};
  int x=cx(),cw=content_w();for(int i=0;i<5;i++){int yy=210+i*100;panel(x,yy,cw,86);
   rounded(x+22,yy+20,45,45,13,"#164448");paint(x+37,yy+52,std::to_string(i+1),Fonts::subtitle,"#8DEED7");
   paint(x+88,yy+52,steps[i],Fonts::regular,"#E8F5FF",cw-122);
  }
 }
 void activity(){page_title("Session activity","Privacy-first insights: no cloud account, tracking or background service.");
  int x=cx(),cw=content_w();panel(x,210,cw,375);
  paint(x+30,260,"CURRENT SESSION",Fonts::small,"#8FA7BE");paint(x+30,312,message,Fonts::medium,"#7EE8CF",cw-60);
  const char* facts[]={"Live measurements run with normal user permissions.","The Optimize button opens an explanation and consent dialog.","Only two reversible kernel preferences can be adjusted.","No cache drops, swapoff, service restarts or process termination."};
  for(int i=0;i<4;i++){disk(x+38,364+i*48,5,"#6BE0BB");paint(x+59,371+i*48,facts[i],Fonts::regular,"#C1D2E3",cw-95);}
 }
 int hero_button_x()const{return cx()+content_w()-314;}
 void wrap(int x,int first_y,const std::string& text,Fonts::Size size,const char* color,int max_width,int line_height,int max_lines){
  std::istringstream stream(text);std::string word,line;int lines=0;
  while(stream>>word){auto candidate=line.empty()?word:line+" "+word;
   if(!line.empty()&&fonts.width(d,candidate,size)>max_width){
    paint(x,first_y+lines*line_height,line,size,color,max_width);lines++;line=word;
    if(lines>=max_lines)return;
   }else line=candidate;
  }
  if(!line.empty()&&lines<max_lines)paint(x,first_y+lines*line_height,line,size,color,max_width);
 }
 void dialog(){// A full in-app preview is ALWAYS required before an elevated action.
  rounded(0,0,width,height,0,"#07101F");
  int mw=675,mh=366,mx=(width-mw)/2,my=(height-mh)/2;
  panel(mx,my,mw,mh,"#16283D");rounded(mx+27,my+25,55,55,17,"#204B47");
  paint(mx+42,my+61,"+",Fonts::title,"#86F0D1");
  const bool opt=modal==1;const auto plan=mb::plan_optimization(snapshot);
  bool actionable=opt?plan.can_apply:true;
  paint(mx+102,my+53,opt?"Smart optimization preview":"Review kernel profile change",Fonts::subtitle,"#F4FAFF");
  paint(mx+102,my+76,"NO SYSTEM CHANGES HAVE BEEN MADE",Fonts::tiny,"#93EACF");
  const auto p=mb::profiles().at(profile);
  paint(mx+32,my+133,opt?plan.headline:"Selected: "+p.name,Fonts::medium,"#E9F3FF",mw-56);
  wrap(mx+32,my+168,opt?plan.explanation:"A temporary kernel adjustment will be verified, with rollback attempted on failure.",Fonts::regular,"#A8BED3",mw-63,24,2);
  paint(mx+32,my+225,opt?(actionable?"Proposed: swappiness 20, VFS cache pressure 75":"System managed caches and processes remain untouched."):
    "Change vm.swappiness to "+std::to_string(p.swappiness)+" and vm.vfs_cache_pressure to "+std::to_string(p.vfs_cache_pressure),Fonts::regular,"#B9D6E2",mw-60);
  rounded(mx+31,my+241,mw-62,43,11,"#1D3546");
  paint(mx+49,my+268,"No forced cleanups, data removal, or guaranteed speed boost.",Fonts::small,"#C5EBDD",mw-92);
  rounded(mx+31,my+294,225,48,12,"#263B51");paint(mx+95,my+325,"CANCEL",Fonts::medium,"#DAE8F9");
  if(actionable){rounded(mx+273,my+294,mw-304,48,12,"#66E5BF");
   paint(mx+305,my+325,"AUTHORIZE SAFE TUNING",Fonts::medium,"#103832",mw-335);}
  else{rounded(mx+273,my+294,mw-304,48,12,"#214E48");
   paint(mx+339,my+325,"NO CHANGES NEEDED",Fonts::medium,"#A9F2DB",mw-345);}
 }
 void draw(){rect(0,0,width,height,"#0A1425");sidebar();topbar();
  switch(tab){case 0:dashboard();break;case 1:processes();break;case 2:diagnosis();break;
   case 3:telemetry();break;case 4:profiles();break;case 5:guide();break;default:activity();break;}
  footer();if(modal)dialog();XFlush(d);
 }
 void sample(){auto now=std::chrono::steady_clock::now();auto next=mb::collect();
  if(sampled)delta=mb::telemetry_delta(snapshot,next,std::chrono::duration<double>(now-sample_time).count());
  snapshot=std::move(next);sample_time=now;sampled=true;
  const auto& m=snapshot.memory;double share=m.total?double(m.total-m.available)/m.total:0.;
  history.push_back(std::clamp(share,0.,1.));if(history.size()>120)history.pop_front();
 }
 void apply(bool smart){if(child>0){message="Another operation is running";return;}
  if(smart&&!mb::plan_optimization(snapshot).can_apply){message="Optimization review: no safe tuning needed.";return;}
  auto p=mb::profiles().at(profile);
  if(::geteuid()==0){auto res=smart?mb::apply_profile(mb::profiles().at(1)):mb::apply_profile(p);
   message=res.message;sample();return;}
  pid_t pid=fork();if(pid<0){message="Could not request administrator authorization.";return;}
  if(pid==0){char executable[4096];auto n=readlink("/proc/self/exe",executable,sizeof(executable)-1);
   if(n<0)_exit(127);executable[n]='\0';
   if(smart)execlp("pkexec","pkexec",executable,"--optimize","--yes",(char*)nullptr);
   else execlp("pkexec","pkexec",executable,"--apply",p.name.c_str(),"--yes",(char*)nullptr);
   _exit(127);
  }
  child=pid;message="Requesting administrator authorization...";
 }
 void click(int x,int y){if(modal){int mw=675,mh=366,mx=(width-mw)/2,my=(height-mh)/2;
   if(y>=my+294&&y<my+344){if(x>=mx+31&&x<mx+256){modal=0;message="Optimization cancelled. No changes made.";return;}
    if(x>=mx+273&&x<mx+mw-31){bool smart=modal==1;const bool ok=smart?mb::plan_optimization(snapshot).can_apply:true;
     modal=0;if(ok)apply(smart);else message="No changes needed. System remains unchanged.";return;}}
   return;
  }
  if(x>=14&&x<=SB-14){for(int i=0;i<7;i++)if(y>=nav_y+i*nav_step-24&&y<nav_y+i*nav_step+23){tab=i;return;}}
  if(tab==0&&x>=hero_button_x()&&x<hero_button_x()+270&&y>=216&&y<280){modal=1;return;}
  if(tab==4){int yy=214;
   for(int i=0;i<3;i++)if(y>=yy+i*123&&y<yy+i*123+108&&x>=cx()&&x<cx()+content_w()){profile=i;return;}
   if(x>=cx()&&x<cx()+256&&y>=yy+391&&y<yy+446){modal=2;return;}
  }
 }
 int run(){d=XOpenDisplay(nullptr);if(!d){std::fprintf(stderr,"Cannot open X11 display. Use memboost --diagnose for headless mode.\n");return 3;}
  const int screen=DefaultScreen(d);win=XCreateSimpleWindow(d,RootWindow(d,screen),std::max(0,(DisplayWidth(d,screen)-W)/2),std::max(0,(DisplayHeight(d,screen)-H)/2),W,H,0,0,0);
  XStoreName(d,win,"MemBoost Pro | Smart Memory Optimizer");
  XSelectInput(d,win,ExposureMask|StructureNotifyMask|ButtonPressMask|KeyPressMask);
  close_atom=XInternAtom(d,"WM_DELETE_WINDOW",False);XSetWMProtocols(d,win,&close_atom,1);
  gc=XCreateGC(d,win,0,nullptr);fonts.initialize(d,win);
  XSizeHints hints{};hints.flags=PMinSize;hints.min_width=1100;hints.min_height=760;XSetWMNormalHints(d,win,&hints);
  XMapWindow(d,win);sample();draw();redraw_time=std::chrono::steady_clock::now();
  while(alive){struct pollfd f{ConnectionNumber(d),POLLIN,0};(void)poll(&f,1,150);
   while(XPending(d)){XEvent e{};XNextEvent(d,&e);
    if(e.type==ClientMessage&&static_cast<Atom>(e.xclient.data.l[0])==close_atom)alive=false;
    else if(e.type==ConfigureNotify){width=e.xconfigure.width;height=e.xconfigure.height;draw();}
    else if(e.type==Expose&&e.xexpose.count==0)draw();
    else if(e.type==ButtonPress&&e.xbutton.button==Button1){click(e.xbutton.x,e.xbutton.y);draw();}
    else if(e.type==KeyPress){char buf[16]{};KeySym key=0;XLookupString(&e.xkey,buf,sizeof(buf),&key,nullptr);
     if(key==XK_Escape)modal=0;else if((key==XK_o||key==XK_O)&&tab==0&&modal==0)modal=1;
     draw();}
   }
   if(child>0){int status=0;pid_t r=waitpid(child,&status,WNOHANG);
    if(r==child){child=-1;message=WIFEXITED(status)&&WEXITSTATUS(status)==0?
      "Tuning completed and verified. Changes are temporary.":"Tuning cancelled or failed. No success was claimed.";draw();}}
   auto now=std::chrono::steady_clock::now();if(now-redraw_time>=std::chrono::seconds(2)){
     sample();draw();redraw_time=now;}
  }
  fonts.finish();XFreeGC(d,gc);XDestroyWindow(d,win);XCloseDisplay(d);return 0;
 }
};
}
int gui_main(int,char**){App app;return app.run();}
