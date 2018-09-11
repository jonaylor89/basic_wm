
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
            default:
                LOG(WARNING) << "Ignored event";
        }
    }
}

int WindowManager::OnWMDetected(DIsplay* display, XErrorEvent* e) {
    CHECK_EQ(static_cast<int>(e->error_code), BadAccess);

    wm_deteced_ = true;

    return 0;
}

int WindowManager::OnXError(Display* display, XErrorEvent* e) { /* print e */}

