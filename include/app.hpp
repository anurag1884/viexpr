#ifndef APP_HPP
#define APP_HPP

#include <webview.h>

namespace viexpr
{
struct app
{
    webview_t view;

    // Constructs the app given the title, width and height of the window
    app(const char *title, int w, int h);

    // Destroys the app object
    virtual ~app();

    // Sets up the webview window to interoperate with C functions and integrate
    // frontend code
    void setup();

    // Run the app by displaying the frontend UI
    void run();
};
} // namespace viexpr

#endif /* APP_HPP */
