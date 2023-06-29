/* textGL uses openGL to render text on the screen, 
it supports TTF and OTF and just kidding ha it only supports one font,
a custom font i made because i hate you */

#include "include/turtle.h"
#include <time.h>

typedef struct { // class variables
    int bezierPrez; // precision for bezier curves
    int charCount; // number of supported characters
    unsigned int *supportedCharReference; // array containing links from (int) unicode values of characters to an index from 0 to (charCount - 1)
    int *fontPointer; // array containing links from char indices (0 to (charCount - 1)) to their corresponding data position in fontData
    int *fontData; // array containing packaged instructions on how to draw each character in the character set
} class;

int textGLInit(class *selfp, const char *filename) { // initialise values, must supply a font file (tgl)
    class self = *selfp;
    self.bezierPrez = 10;

    /* load file */
    FILE *tgl = fopen(filename, "r");
    if (tgl == NULL) {
        printf("Error: could not open file %s\n", filename);
        return -1;
    }

    list_t *fontDataInit = list_init(); // these start as lists and become int arrays
    list_t *fontPointerInit = list_init();
    list_t *supportedCharReferenceInit = list_init();
    unsigned char parsedInt[12]; // max possible length of an int as a string is 11
    unsigned int fontPar;
    int oldptr;
    int strptr;

    char line[2048]; // maximum line length
    line[2047] = 0; // canary value
    self.charCount = 0;
    while (fgets(line, 2048, tgl) != NULL) { // fgets to prevent overflows
        if (line[2047] != 0) {
            printf("Error: line %d in file %s exceeded appropriate length\n", 0, filename);
            return -1;
        }

        oldptr = 0;
        strptr = 0;
        int loops = 0;
        int firstIndex = 0;
        
        while (line[strptr] != '\n' && line[strptr] != '\0') {
            while (line[strptr] != ',' && line[strptr] != '\n' && line[strptr] != '\0') {
                parsedInt[strptr - oldptr] = line[strptr];
                strptr += 1;
            }
            parsedInt[strptr - oldptr] = '\0';
            if (oldptr == 0) {
                fontPar = 0; // parse as unicode char (advanced tactic maps every unicode character to an unsigned int using big endian (must be UTF-8))
                for (int i = 0; i < strlen((char *) parsedInt); i++) {
                    fontPar += (unsigned int) parsedInt[i] << (i * 8);
                }
                if (fontPar == 0) { // exception for the comma character
                    fontPar = 44;
                }
                list_append(supportedCharReferenceInit, (unitype) (int) fontPar, 'i'); // adds as an int but will typecast back to unsigned later, this might not work correctly but it also doesn't really matter
                list_append(fontPointerInit, (unitype) (int) (fontDataInit -> length), 'i');
                firstIndex = fontDataInit -> length;
                list_append(fontDataInit, (unitype) 1, 'i');
            } else {
                sscanf((char *) parsedInt, "%i", &fontPar); // parse as integer
                if (strcmp((char *) parsedInt, "b") == 0) {
                    // printf("b found, fontPar: %d\n", fontPar);
                    list_append(fontDataInit, (unitype) 140894115, 'i'); // all b's get converted to the integer 140894115 (chosen semi-randomly)
                } else {
                    list_append(fontDataInit, (unitype) (int) (fontPar), 'i'); // fontPar will double count when it encounters a b (idk why) so if there's a b we ignore the second fontPar (which is a duplicate of the previous one)
                }
                loops += 1;
            }
            if (line[strptr] != '\n' && line[strptr] != '\0')
                strptr += 2;
            oldptr = strptr;
        }
        fontDataInit -> data[firstIndex] = (unitype) loops;
        firstIndex += 1; // using firstIndex as iteration variable
        int len1 = fontDataInit -> data[firstIndex].i;
        int maximums[4] = {-2147483648, -2147483648, 2147483647, 2147483647}; // for describing bounding box of a character

        /* good programmng alert*/
        #define CHECKS_EMB(ind) \
            if (fontDataInit -> data[ind].i > maximums[0]) {\
                maximums[0] = fontDataInit -> data[ind].i;\
            }\
            if (fontDataInit -> data[ind].i < maximums[3]) {\
                maximums[3] = fontDataInit -> data[ind].i;\
            }\
            if (fontDataInit -> data[ind + 1].i > maximums[1]) {\
                maximums[1] = fontDataInit -> data[ind + 1].i;\
            }\
            if (fontDataInit -> data[ind + 1].i < maximums[3]) {\
                maximums[3] = fontDataInit -> data[ind + 1].i;\
            }
        for (int i = 0; i < len1; i++) {
            firstIndex += 1;
            int len2 = fontDataInit -> data[firstIndex].i;
            for (int j = 0; j < len2; j++) {
                firstIndex += 1;
                if (fontDataInit -> data[firstIndex].i == 140894115) {
                    firstIndex += 1;
                    fontDataInit -> data[firstIndex] = (unitype) (fontDataInit -> data[firstIndex].i + 160);
                    fontDataInit -> data[firstIndex + 1] = (unitype) (fontDataInit -> data[firstIndex + 1].i + 100);
                    CHECKS_EMB(firstIndex);
                    firstIndex += 2;
                    fontDataInit -> data[firstIndex] = (unitype) (fontDataInit -> data[firstIndex].i + 160);
                    fontDataInit -> data[firstIndex + 1] = (unitype) (fontDataInit -> data[firstIndex + 1].i + 100);
                    CHECKS_EMB(firstIndex);
                    firstIndex += 1;
                    if (fontDataInit -> data[firstIndex + 1].i != 140894115) {
                        firstIndex += 1;
                        fontDataInit -> data[firstIndex] = (unitype) (fontDataInit -> data[firstIndex].i + 160);
                        fontDataInit -> data[firstIndex + 1] = (unitype) (fontDataInit -> data[firstIndex + 1].i + 100);
                        CHECKS_EMB(firstIndex);
                        firstIndex += 1;
                    }
                } else {
                    fontDataInit -> data[firstIndex] = (unitype) (fontDataInit -> data[firstIndex].i + 160);
                    fontDataInit -> data[firstIndex + 1] = (unitype) (fontDataInit -> data[firstIndex + 1].i + 100);
                    CHECKS_EMB(firstIndex);
                    firstIndex += 1;
                }
            }
        }
        if (maximums[0] < 0) {
            list_append(fontDataInit, (unitype) 90, 'i');
        } else {
            list_append(fontDataInit, (unitype) maximums[0], 'i');
        }
        if (maximums[3] > 0) {
            list_append(fontDataInit, (unitype) 0, 'i');
        } else {
            list_append(fontDataInit, (unitype) maximums[3], 'i');
        }
        if (maximums[1] < 0) {
            if (self.charCount == 0)
                list_append(fontDataInit, (unitype) 0, 'i');
            else
                list_append(fontDataInit, (unitype) 120, 'i');
        } else {
            list_append(fontDataInit, (unitype) maximums[1], 'i');
        }
        if (maximums[2] > 0) {
            list_append(fontDataInit, (unitype) 0, 'i');
        } else {
            list_append(fontDataInit, (unitype) maximums[2], 'i');
        }
        self.charCount += 1;
    }
    list_append(fontPointerInit, (unitype) (int) (fontDataInit -> length), 'i'); // last pointer
    self.fontData = malloc(sizeof(int) * fontDataInit -> length); // convert lists to arrays (could be optimised cuz we already have the -> data arrays but who really cares this runs once)
    for (int i = 0; i < fontDataInit -> length; i++) {
        self.fontData[i] = fontDataInit -> data[i].i;
    }
    self.fontPointer = malloc(sizeof(int) * fontPointerInit -> length);
    for (int i = 0; i < fontPointerInit -> length; i++) {
        self.fontPointer[i] = fontPointerInit -> data[i].i;
    }
    self.supportedCharReference = malloc(sizeof(int) * supportedCharReferenceInit -> length);
    for (int i = 0; i < supportedCharReferenceInit -> length; i++) {
        self.supportedCharReference[i] = supportedCharReferenceInit -> data[i].i;
    }

    printf("%d characters loaded from %s\n", self.charCount, filename);

    list_free(fontDataInit);
    list_free(fontPointerInit);
    list_free(supportedCharReferenceInit);
    *selfp = self;
    return 0;
}

/* render functions */

void renderBezier(double x1, double y1, double x2, double y2, double x3, double y3, int prez) { // renders a quadratic bezier curve on the screen
    turtleGoto(x1, y1);
    turtlePenDown();
    double iter1 = 1;
    double iter2 = 0;
    for (int i = 0; i < prez; i++) {
        iter1 -= (double) 1 / prez;
        iter2 += (double) 1 / prez;
        double t1 = iter1 * iter1;
        double t2 = iter2 * iter2;
        double t3 = 2 * iter1 * iter2;
        turtleGoto(t1 * x1 + t3 * x2 + t2 * x3, t1 * y1 + t3 * y2 + t2 * y3);
    }
    turtleGoto(x3, y3);
}

void renderChar(class *selfp, int index, double x, double y, double size) { // renders a single character
    class self = *selfp;
    index += 1;
    int len1 = self.fontData[index];
    for (int i = 0; i < len1; i++) {
        index += 1;
        if (turtools.pen == 1)
            turtlePenUp();
        int len2 = self.fontData[index];
        for (int j = 0; j < len2; j++) {
            index += 1;
            if (self.fontData[index] == 140894115) { // 140894115 is the b value (reserved)
                index += 4;
                if (self.fontData[index + 1] != 140894115) {
                    renderBezier(x + self.fontData[index - 3] * size, y + self.fontData[index - 2] * size, x + self.fontData[index - 1] * size, y + self.fontData[index] * size, x + self.fontData[index + 1] * size, y + self.fontData[index + 2] * size, self.bezierPrez);
                    index += 2;
                } else {
                    renderBezier(x + self.fontData[index - 3] * size, y + self.fontData[index - 2] * size, x + self.fontData[index - 1] * size, y + self.fontData[index] * size, x + self.fontData[index + 2] * size, y + self.fontData[index + 3] * size, self.bezierPrez);
                }
            } else {
                index += 1;
                turtleGoto(x + self.fontData[index - 1] * size, y + self.fontData[index] * size);
            }
            turtlePenDown();
        }
    }
    turtlePenUp();
    // no variables in self are changed
}

void write(class *selfp, const unsigned int *text, int textLength, double x, double y, double size, double align) {
    class self = *selfp;
    self.bezierPrez = (int) ceil(sqrt(size * 1)); // change the 1 for higher or lower bezier precision
    double xTrack = x;
    size /= 175;
    y -= size * 70;
    turtlePenSize(30 * size);
    turtlePenShape("connected");
    list_t* xvals = list_init();
    list_t* dataIndStored = list_init();
    for (int i = 0; i < textLength; i++) {
        int currentDataAddress = 0;
        for (int j = 0; j < self.charCount; j++) { // change to hashmap later
            if (self.supportedCharReference[j] == text[i]) {
                currentDataAddress = j;
                break;
            }
        }
        list_append(xvals, (unitype) xTrack, 'd');
        list_append(dataIndStored, (unitype) currentDataAddress, 'i');
        xTrack += (self.fontData[self.fontPointer[currentDataAddress + 1] - 4] + 40) * size;
    }
    xTrack -= 40 * size;
    for (int i = 0; i < textLength; i++) {
        renderChar(&self, (double) self.fontPointer[dataIndStored -> data[i].i], xvals -> data[i].d - ((xTrack - x) * (align / 100)), y, size);
    }
    list_free(dataIndStored);
    list_free(xvals);
    // no variables in self are changed
}

/* transfer functions */

void writeString(class *selfp, const char *str, double x, double y, double size, double align) { // wrapper function for writing strings easier
    int len = strlen(str);
    unsigned int converted[len];
    for (int i = 0; i < len; i++) {
        converted[i] = (unsigned int) str[i];
    }
    write(selfp, converted, len, x, y, size, align);
}

int main(int argc, char *argv[]) {
    int tps = 60; // ticks per second (locked to fps in this case)
    clock_t start, end;
    class obj;
    if (argc > 1) { // parse args
        if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "-f") == 0) {
            if (argc > 2) {
                if (textGLInit(&obj, argv[2]) == -1) { // initialise the class
                    return -1;
                }
            } else {
                printf("Error: must supply font file\n");
                return -1;
            }
        } else {
            if (textGLInit(&obj, argv[1]) == -1) { // initialise the class
                    return -1;
                }
        }
    } else {
        printf("Error: must supply font file\n");
        return -1;
    }

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

    /* initialize turtools */
    turtoolsInit(window, -240, -180, 240, 180);
    turtlePenShape("connected");
    // turtlePenShape("circle");
    turtlePenColor(0, 0, 0);
    turtlePenPrez(5); // for slight speed improvements at the cost of some prettiness

    // renderBezier(0, 0, 50, 50, 100, 0, 25); // bezier test
    // turtleMainLoop();

    // for (int i = 0; i < 500; i++) {
    //     printf("%d: %d\n", i, obj.fontData[i]);
    // }

    
    /* showcase english alphabet */
    // turtleClear();
    // writeString(&obj, "ABCDEFGHIJKLMN", -230, 60, 35, 0);
    // writeString(&obj, "OPQRSTUVWXYZ", -230, 20, 35, 0);
    // writeString(&obj, "abcdefghijklmnopqrstuvwxyz", -230, -50, 24, 0);
    // turtleMainLoop();
    
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
        writeString(&obj, "ABCDEFGHIJKLMN", turtools.mouseX, turtools.mouseY + writeSize * (60.0 / 35), writeSize, 50);
        writeString(&obj, "OPQRSTUVWXYZ", turtools.mouseX, turtools.mouseY + writeSize * (20.0 / 35), writeSize, 50);
        writeString(&obj, "abcdefghijklmnopqrstuvwxyz", turtools.mouseX, turtools.mouseY - writeSize * (30.0 / 35), writeSize * (24.0 / 35), 50);

        // writeString(&obj, "L", 0, 0, writeSize, 50);

        // writeString(&obj, "ABCDEFGHIJKLMN", 0, 0 + writeSize * (60.0 / 35), writeSize, 50);
        // writeString(&obj, "OPQRSTUVWXYZ", 0, 0 + writeSize * (20.0 / 35), writeSize, 50);
        // writeString(&obj, "abcdefghijklmnopqrstuvwxyz", 0, 0 - writeSize * (30.0 / 35), writeSize * (24.0 / 35), 50);

        turtleUpdate(); // update the screen
        end = clock();
        while ((double) (end - start) / CLOCKS_PER_SEC < (1 / (double) tps)) {
            end = clock();
        }
        // return -1;
    }
}