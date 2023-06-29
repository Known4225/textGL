#include "textGL.h"
#include <time.h>

int main(int argc, char *argv[]) {
    GLFWwindow* window;
    /* Initialize glfw */
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA (Anti-Aliasing) with 4 samples (must be done before window is created (?))

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(960, 720, "textGL", NULL, NULL);
    if (!window) {
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, GLFW_DONT_CARE, GLFW_DONT_CARE, 960, 720);
    gladLoadGL();

    /* initialise textGL */
    textGLInit(window, "fontBez.tgl");

    int tps = 60; // ticks per second (locked to fps in this case)
    clock_t start, end;

    double writeSize = 35;
    while (turtools.close == 0) { // main loop
        start = clock();
        turtleClear();
        turtleGetMouseCoords(); // get the mouse coordinates (turtools.mouseX, turtools.mouseY)
        double mouseWheel = turtleMouseWheel(); // behavior is a bit different for the scroll wheel
        // unsigned int text[1] = {67};
        // write(&obj, text, 1, turtools.mouseX, turtools.mouseY, 175, 50);

        if (mouseWheel > 0) {
            writeSize *= 1.1;
        }
        if (mouseWheel < 0) {
            writeSize /= 1.1;
        }
        writeString("ABCDEFGHIJKLMN", turtools.mouseX, turtools.mouseY + writeSize * (60.0 / 35), writeSize, 50);
        writeString("OPQRSTUVWXYZ", turtools.mouseX, turtools.mouseY + writeSize * (20.0 / 35), writeSize, 50);
        writeString("abcdefghijklmnopqrstuvwxyz", turtools.mouseX, turtools.mouseY - writeSize * (30.0 / 35), writeSize * (24.0 / 35), 50);

        turtleUpdate(); // update the screen
        end = clock();
        while ((double) (end - start) / CLOCKS_PER_SEC < (1 / (double) tps)) {
            end = clock();
        }
    }
}