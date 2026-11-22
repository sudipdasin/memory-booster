#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

static Window find_app(Display* d) {
    Window root = DefaultRootWindow(d), parent = 0, *children = nullptr;
    unsigned int count = 0;
    if (!XQueryTree(d, root, &root, &parent, &children, &count)) return 0;
    Window result = 0;
    for (unsigned i=0; i<count; i++) {
        char* name = nullptr;
        if (XFetchName(d, children[i], &name) && name) {
            if (std::strstr(name, "MemBoost Pro") != nullptr) result = children[i];
            XFree(name);
        }
        if (result) break;
    }
    if (children) XFree(children);
    return result;
}

static unsigned long long fingerprint(Display* d, Window w) {
    XWindowAttributes a;
    if (!XGetWindowAttributes(d,w,&a) || a.width<950 || a.height<680) throw std::runtime_error("invalid GUI window geometry");
    auto* img = XGetImage(d,w,0,0,a.width,a.height,AllPlanes,ZPixmap);
    if (!img) throw std::runtime_error("X11 framebuffer not accessible");
    unsigned long long h = 1469598103934665603ULL;
    for (int y=110; y<660; y+=13) for (int x=50; x<900; x+=13) {
        h = (h ^ XGetPixel(img,x,y))*1099511628211ULL;
    }
    XDestroyImage(img);
    return h;
}

static void click(Display* d, Window w, int x, int y) {
    XEvent e{};
    e.xbutton.type=ButtonPress;
    e.xbutton.display=d;
    e.xbutton.window=w;
    e.xbutton.root=DefaultRootWindow(d);
    e.xbutton.same_screen=True;
    e.xbutton.button=Button1;
    e.xbutton.x=x;
    e.xbutton.y=y;
    if (!XSendEvent(d,w,False,ButtonPressMask,&e)) throw std::runtime_error("XSendEvent failed");
    XFlush(d);
}

int main(int argc,char** argv) {
    if (argc!=2) { std::cerr<<"expected executable path\n"; return 2; }
    Display* d=XOpenDisplay(nullptr);
    if (!d) {std::cerr<<"no X11 display\n";return 2;}
    pid_t pid=fork();
    if (pid<0) return 2;
    if (pid==0) {execl(argv[1],argv[1],"--gui",(char*)nullptr);_exit(127);}
    int rc=1;
    try {
        Window w=0;
        for (int i=0;i<60 && !(w=find_app(d));i++) {
            int status=0;
            if (waitpid(pid,&status,WNOHANG)==pid) throw std::runtime_error("GUI exited during startup");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (!w) throw std::runtime_error("GUI window never appeared");
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
        auto previous=fingerprint(d,w);
        for (int i=1;i<5;i++) {
            click(d,w,349+i*137,35);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            auto next=fingerprint(d,w);
            if (next==previous) throw std::runtime_error("tab "+std::to_string(i)+" did not update the rendered view");
            previous=next;
        }
        // Test profile selection and confirmation cancellation without writing host settings.
        click(d,w,349+2*137,35);
        std::this_thread::sleep_for(std::chrono::milliseconds(280));
        click(d,w,90,273);  // Select second profile.
        click(d,w,110,473); // Open confirmation modal.
        std::this_thread::sleep_for(std::chrono::milliseconds(280));
        const auto modal=fingerprint(d,w);
        click(d,w,290,404); // Cancel (1040/2 - 280 .. -80; 720/2 + 40..+84)
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        if (fingerprint(d,w)==modal) throw std::runtime_error("confirmation modal did not close");
        int status=0;
        if (waitpid(pid,&status,WNOHANG)==pid) throw std::runtime_error("GUI exited unexpectedly");
        Atom close=XInternAtom(d,"WM_DELETE_WINDOW",False);
        XEvent e{}; e.xclient.type=ClientMessage; e.xclient.window=w;
        e.xclient.message_type=XInternAtom(d,"WM_PROTOCOLS",False); e.xclient.format=32;
        e.xclient.data.l[0]=close; e.xclient.data.l[1]=CurrentTime;
        XSendEvent(d,w,False,NoEventMask,&e);XFlush(d);
        for (int i=0;i<40;i++) {
            if (waitpid(pid,&status,WNOHANG)==pid) {pid=-1; break;}
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (pid!=-1) throw std::runtime_error("GUI did not exit on window close");
        if (!WIFEXITED(status) || WEXITSTATUS(status)!=0) throw std::runtime_error("GUI closed with error");
        std::cout<<"X11 GUI: four tabs, profile, confirmation cancel and WM close passed\n";
        rc=0;
    } catch (const std::exception& e) { std::cerr<<"GUI test failed: "<<e.what()<<"\n"; }
    if (pid>0) {kill(pid,SIGTERM);waitpid(pid,nullptr,0);}
    XCloseDisplay(d);
    return rc;
}
