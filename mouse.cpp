#define GLFW_EXPOSE_NATIVE_WIN32
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "deps/glfw3.h"
#include "deps/glfw3native.h"

// TODO:
// - why does exact resolution cause transparency to break? See :ExactResolution:

const double TAU = 6.283185307179586;
const double dim_amount = 0.7;
const int spotlight_segments = 60*4;
double dim_target;
double dim_current;
double spotlight_target_radius;
double spotlight_current_radius;
bool closing;

double lerp(double a, double b, double t) { return a + t * (b - a); }

void begin_closing()
{
    closing = true;
    dim_target = 0;
    spotlight_target_radius = 1;
}

int show_spotlight()
{
    // reset values
    dim_target = dim_amount;
    dim_current = 0;
    spotlight_target_radius = 0.1;
    spotlight_current_radius = 1;
    closing = false;

    // initialize the library
    if (!glfwInit()) return 1;

    // get resolution information
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // calculate aspect ratio
    int width = mode->width;
    int height = mode->height + 1; // see :ExactResolution:
    float ratio = (float)mode->width / (float)mode->height;

    // set the window type
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // transparent
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);                // always on top
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);              // borderless

    // create a window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(width, height, "Mouse Highlighter", NULL, NULL);
    if (!window) { glfwTerminate(); return 2; }

    // remove the taskbar icon - feels a little hacky, the other option is to explore
    // having the main entry point be a graphical application that has no visuals except
    // living in the system tray, then make that the parent of this spotlight window
    HWND hwnd = glfwGetWin32Window(window);
    DWORD style = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, (style & ~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW);

    // listen for key press
    glfwSetKeyCallback(window, [](GLFWwindow*, int, int, int action, int) {
        if (action == GLFW_PRESS) begin_closing();
    });

    // listen for click
    glfwSetMouseButtonCallback(window, [](GLFWwindow*, int, int action, int) {
        if (action == GLFW_PRESS) begin_closing();
    });

    // listen for mouse wheel scrolling
    glfwSetScrollCallback(window, [](GLFWwindow*, double, double scroll) {
        if (closing) return;
        spotlight_target_radius += scroll * -0.05;
        if (spotlight_target_radius < 0.05) spotlight_target_radius = 0.05;
        if (spotlight_target_radius > 0.60) spotlight_target_radius = 0.60;
    });

    // make the window's context current
    glfwMakeContextCurrent(window);

    // turn on vsync
    glfwSwapInterval(1);

    // show the window
    glfwShowWindow(window);

    // used to keep track of delta time
    double last = glfwGetTime();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // calculate delta time
        double dt = glfwGetTime() - last;
        last = glfwGetTime();

        // get mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        double y = 2 * (1 - ypos / height) - 1;
        double x = 2 * (xpos / width) - 1;

        // calculate spotlight radius
        double rx = spotlight_current_radius;
        double ry = spotlight_current_radius * ratio;

        // update dim and spotlight amounts
        dim_current = lerp(dim_current, dim_target, dt * 10);
        spotlight_current_radius = lerp(spotlight_current_radius, spotlight_target_radius, dt * 10);
        if (closing && dim_current <= 0.01) glfwSetWindowShouldClose(window, GLFW_TRUE);

        // render
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_QUADS);
        glColor4f(0, 0, 0, dim_current);

        auto draw_quad = [](double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
        {
            glVertex2f(x0, y0);
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
            glVertex2f(x3, y3);
        };

        for (int i = 0; i < spotlight_segments; ++i)
        {
            double x0 = x + sin((i+0)%spotlight_segments*TAU/spotlight_segments) * rx;
            double y0 = y + cos((i+0)%spotlight_segments*TAU/spotlight_segments) * ry;
            double x1 = x + sin((i+1)%spotlight_segments*TAU/spotlight_segments) * rx;
            double y1 = y + cos((i+1)%spotlight_segments*TAU/spotlight_segments) * ry;

            if (i < spotlight_segments / 4 * 1) // top right quadrant
            {
                draw_quad(x0, 1, x1, 1, x1, y1, x0, y0);
                draw_quad(1, y0, 1, y1, x1, y1, x0, y0);
            }
            else if (i < spotlight_segments / 4 * 2) // bottom right quadrant
            {
                draw_quad(x0, -1, x1, -1, x1, y1, x0, y0);
                draw_quad(1, y0, 1, y1, x1, y1, x0, y0);
            }
            else if (i < spotlight_segments / 4 * 3) // bottom left quadrant
            {
                draw_quad(x0, -1, x1, -1, x1, y1, x0, y0);
                draw_quad(-1, y0, -1, y1, x1, y1, x0, y0);
            }
            else // top left quadrant
            {
                draw_quad(x0, 1, x1, 1, x1, y1, x0, y0);
                draw_quad(-1, y0, -1, y1, x1, y1, x0, y0);
            }
        }

        auto draw_rectangle = [](double x0, double y0, double x1, double y1)
        {
            glVertex2f(x0, y0);
            glVertex2f(x1, y0);
            glVertex2f(x1, y1);
            glVertex2f(x0, y1);
        };

        draw_rectangle(    -1,  1,  1, y + ry); // top
        draw_rectangle(    -1, -1,  1, y - ry); // bottom
        draw_rectangle(x - rx, -1, -1,      1); // left
        draw_rectangle(x + rx, -1,  1,      1); // right

        glEnd();

        // swap front and back buffers
        glfwSwapBuffers(window);

        // poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

int main()
{           
    if (RegisterHotKey(NULL, 1, MOD_ALT | MOD_NOREPEAT, 'S') &&
        RegisterHotKey(NULL, 2, MOD_ALT | MOD_NOREPEAT, 'Q'))
    {
        printf("'ALT+s' registered for spotlight\n");
        printf("'ALT+q' registered for quitting\n");
    }
    else
    {
        printf("ERROR: Couldn't register keyboard shortcuts.");
        return 1;
    }
 
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        if (msg.message == WM_HOTKEY && msg.wParam == 1)
        {
            int result = show_spotlight();
            if (result) printf("ERROR: %d\n", result);
        }

        if (msg.message == WM_HOTKEY && msg.wParam == 2)
        {
            return 0;
        }
    } 
 
    return 0;
}