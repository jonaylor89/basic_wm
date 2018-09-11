
extern "C" {
    #include <X11/Xlib.h>
}

#include <memory>

class WindowManager {
    public:
        // Establish connection with X server
        static ::std::unique_ptr<WindowManager> Create();
        // Xlib error handler
        static int OnXError(Display* display, XErrorEvent* e);
        // Determine if another window manager is running
        static int OnWMDetected(Display* display, XErrorEvent* e);
        // Whether another window manager is detected
        static bool wm_detected_;
        // Disconnects from X server
        ~WindowManager();
        // Entry point to this class to main event loop
        void Run();

    private:
        // INvoke internally by Create()
        WindowManager(Display* display);
        // Handle to the underlying Xlib display struct
        Display* display_;
        // Handle to root Window
        const Window root_;
};
