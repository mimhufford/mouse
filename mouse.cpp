#include <windows.h>
#include <math.h>
#include "deps/glfw3.h"

// TODO:
// - why does exact resolution cause transparency to break? See :ExactResolution:
// - have process always running in background?

const double TAU = 6.283185307179586;
const double dim_amount = 0.7;
const int spotlight_segments = 60*4;
double dim_target = dim_amount;
double dim_current = 0;
double spotlight_target_radius = 0.1;
double spotlight_current_radius = 1;
bool closing = false;

double lerp(double a, double b, double t) { return a + t * (b - a); }

void begin_closing()
{
    closing = true;
    dim_target = 0;
    spotlight_target_radius = 1;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, char *, int)
{
    // initialize the library
    if (!glfwInit()) return -1;

    // get resolution information
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // calculate aspect ratio
    int width = mode->width;
    int height = mode->height + 1; // see :ExactResolution:
    float ratio = (float)mode->width / (float)mode->height;

    // say that we want transparency
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // say that we want always on top
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    // say that we want a borderless window
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    // create a windowed mode window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(width, height, "Mouse Highlighter", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    // listen for key press
    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) begin_closing();
    });

    // listen for click
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        if (action == GLFW_PRESS) begin_closing();
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