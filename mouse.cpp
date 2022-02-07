#include <windows.h>
#include <math.h>
#include "deps/glfw3.h"

// TODO:
// - go fullscreen, currently even at 1080 the window doesn't fully cover the start bar for some reason
// - once proper fullscreen works use the monitor resolution instead of hardcoding values
// - get rid of window flash when first loading up / have process always running in background?

const double PI = 3.14159265359;
const double TAU = PI * 2;
const double dim_amount = 0.7;
const int spotlight_segments = 60*4;
double dim_target = dim_amount;
double dim_current = 0;
double spotlight_target_radius = 0.1;
double spotlight_current_radius = 1;
bool closing = false;

double lerp(double a, double b, double t) { return a + t * (b - a); }

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char *, int nShowCmd)
{
    int width = 1920;
    int height = 1020;
    float ratio = (float)width / (float)height;

    // initialize the library
    if (!glfwInit())
        return -1;

    // reset the window hints to default
    glfwDefaultWindowHints();

    // say that we want transparency
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // say that we want always on top
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    // create a windowed mode window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(width, height, "Mouse Highlighter", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // remove window borders
    glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);

    // move window to top left
    glfwSetWindowPos(window, 0, 0);

    // listen for escape key
    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            closing = true;
            dim_target = 0;
            spotlight_target_radius = 1;
        }
    });

    // listen for mouse wheel scrolling
    glfwSetScrollCallback(window, [](GLFWwindow *window, double xoffset, double yoffset) {
        if (closing) return;
        spotlight_target_radius += yoffset * -0.05;
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

        // top right quadrant
        for (int i = 0*(spotlight_segments/4); i < 1*(spotlight_segments/4); ++i)
        {
            double x0 = x + sin(i*TAU/spotlight_segments) * rx;
            double y0 = y + cos(i*TAU/spotlight_segments) * ry;
            double x1 = x + sin((i+1)%spotlight_segments*TAU/spotlight_segments) * rx;
            double y1 = y + cos((i+1)%spotlight_segments*TAU/spotlight_segments) * ry;
            glVertex2f(x0, 1);
            glVertex2f(x1, 1);
            glVertex2f(x1, y1);
            glVertex2f(x0, y0);
            glVertex2f(1, y0);
            glVertex2f(1, y1);
            glVertex2f(x1, y1);
            glVertex2f(x0, y0);
        }
        // bottom right quadrant
        for (int i = 1*(spotlight_segments/4); i < 2*(spotlight_segments/4); ++i)
        {
            double x0 = x + sin(i*TAU/spotlight_segments) * rx;
            double y0 = y + cos(i*TAU/spotlight_segments) * ry;
            double x1 = x + sin((i+1)%spotlight_segments*TAU/spotlight_segments) * rx;
            double y1 = y + cos((i+1)%spotlight_segments*TAU/spotlight_segments) * ry;
            glVertex2f(x0, -1);
            glVertex2f(x1, -1);
            glVertex2f(x1, y1);
            glVertex2f(x0, y0);
            glVertex2f(1, y0);
            glVertex2f(1, y1);
            glVertex2f(x1, y1);
            glVertex2f(x0, y0);
        }
        // bottom left quadrant
        for (int i = 2*(spotlight_segments/4); i < 3*(spotlight_segments/4); ++i)
        {
            double x0 = x + sin(i*TAU/spotlight_segments) * rx;
            double y0 = y + cos(i*TAU/spotlight_segments) * ry;
            double x1 = x + sin((i+1)%spotlight_segments*TAU/spotlight_segments) * rx;
            double y1 = y + cos((i+1)%spotlight_segments*TAU/spotlight_segments) * ry;
            glVertex2f(x0, -1);
            glVertex2f(x1, -1);
            glVertex2f(x1, y1);
            glVertex2f(x0, y0);
            glVertex2f(-1, y0);
            glVertex2f(-1, y1);
            glVertex2f(x1, y1);
            glVertex2f(x0, y0);
        }
        // top left quadrant
        for (int i = 3*(spotlight_segments/4); i < 4*(spotlight_segments/4); ++i)
        {
            double x0 = x + sin(i*TAU/spotlight_segments) * rx;
            double y0 = y + cos(i*TAU/spotlight_segments) * ry;
            double x1 = x + sin((i+1)%spotlight_segments*TAU/spotlight_segments) * rx;
            double y1 = y + cos((i+1)%spotlight_segments*TAU/spotlight_segments) * ry;
            glVertex2f(x0, 1);
            glVertex2f(x1, 1);
            glVertex2f(x1, y1);
            glVertex2f(x0, y0);
            glVertex2f(-1, y0);
            glVertex2f(-1, y1);
            glVertex2f(x1, y1);
            glVertex2f(x0, y0);
        }

        // top bar
        glVertex2f(-1, 1);
        glVertex2f(1, 1);
        glVertex2f(1, y + ry);
        glVertex2f(-1, y + ry);
        // bottom bar
        glVertex2f(-1, -1);
        glVertex2f(1, -1);
        glVertex2f(1, y - ry);
        glVertex2f(-1, y - ry);
        // left chunk
        glVertex2f(-1, y + ry);
        glVertex2f(x - rx, y + ry);
        glVertex2f(x - rx, y - ry);
        glVertex2f(-1, y - ry);
        // right chunk
        glVertex2f(x + rx, y + ry);
        glVertex2f(1, y + ry);
        glVertex2f(1, y - ry);
        glVertex2f(x + rx, y - ry);

        glEnd();

        // swap front and back buffers
        glfwSwapBuffers(window);

        // poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}