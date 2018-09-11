
#include <glog/logging.h>
#include "window_manager.hpp"

using ::std::unique_ptr;

unique_ptr<WindowManager> WindowManager::Create() {
    // Open X display
    Display* display = XOpenDisplay(nullptr);

    if (display == nullptr) {
        LOG(ERROR) << "Failed to open X display " << XDisplayName(nullptr);
    }

    return unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display* display)
    : display_(CHECK_NOTNULL(display)),
    root_(DefaultRootWindow(display_)) {
    
}

WindowManager::~WindowManager() {
    XCloseDisplay(display_);
}

void WindowManager::Run() {
    //Initialization

    wm_detected_ = false;
    XSetErrorHandler(&WindowManager::OnWMDetected);
    XSelectInput(
            display_,
            root_,
            SubtructureRedirectMask | SubstructureNotifyMask);
    XSync(display_, false);

    if (wm_detected_) {
        LOG(ERROR)  << "Detected another window manager on display "
                    << XDisplayString(display_);
        return;
    }

    XSetErrorHandler(&WindowManager::OnXError);

    //Event Loop
    
    for (;;) {
        XEvent e;
        XNextEvent(display_, &e);
        LOG(INFO) << "Recieved event: " << ToString(e);

        switch(e.type) {
            case CreateNotify:
                OnCreateNotify(e.xcreatewindow);
                break;
            case DestroyNotify:
                OnDestroyNotify(e.destroywindow);
                break;
            case ReparentNotify:
                OnReparentNotify(e.xreparent);
                break;
            case ConfigureRequest:
                OnConfigureRequest(e.confiurerequest);
                break;
            case MapRequest:
                OnMapRequest(e.xmaprequest);
                break;
            default:
                LOG(WARNING) << "Ignored event";
        }
    }
}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
    Frame(e.window);

    XMapWindow(display_, e.window);
}

void WindowManager::Frame(Window w) {
    const unsigned int BORDER_WIDTH = 3;
    const unsigned long BORDER_COLOR = 0xff0000;
    const unsigned long BG_COLOR = 0x0000ff;

    XWindowAttributes x_window_attrs;
    CHECK(XGetWindowAttributes(display_, w, &x_window_attrs));

    // TODO - see Framing Existing Top-Level Windows
    
    const Window Frame = XCreateSimpleWindow(
            display_,
            root_,
            x_window_attrs.x,
            x_window_attrs.y,
            x_window_attrs.width,
            x_window_attrs.height,
            BORDER_WIDTH,
            BORDER_COLOR,
            BG_COLOR);

    XSelectInput(
            display_,
            frame,
            SubtructureRedirectMask | SubstructureNotifyMask);
            
    XAddToSaveSet(display_, w);

    XReparentWindow(
            display_,
            w,
            frame,
            0, 0);

    XMapWindow(display_, frame);

    clients[w] = frame;

    XGrabButton(...);

    XGrabButton(...);

    XGrabKey(...);

    XGrabKey(...);

    LOG(INFO) << "Framed window " << w <<< " [" << frame << "]";
}

void WindowManager::OnConfigureRequest(const XConfigureRequestEvent& e) {
    XWinfowChanges changes;

    changes.x = e.x;
    changes.y = e.y;
    changes.width = e.width;
    changes.height = e.height;
    changes.border_mode = e.border_width;
    changes.sibling = e.above;
    changes.stack_mode = e.detail;

    XConfigureWindow(display_, e.window, e.value_mask, &changes);
    LOG(INFO) << "Resize " << e.window << " to " << Size<int>(e.width, e.height);
}

void WindowManager::OnCreateNotify(const XCreateWindowEvent& e) {}

int WindowManager::OnWMDetected(DIsplay* display, XErrorEvent* e) {
    CHECK_EQ(static_cast<int>(e->error_code), BadAccess);

    wm_deteced_ = true;

    return 0;
}

int WindowManager::OnXError(Display* display, XErrorEvent* e) { /* print e */}

