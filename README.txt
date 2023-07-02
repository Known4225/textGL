This library is built for windows 64 bit, but can be used with linux and win32

gcc textGL.c -L./Windows -lglfw3 -lopengl32 -lgdi32 -lglad -O3 -o textGL.exe

textGL.exe -f fontBez.tgl

textGL is a simple text renderer that uses openGL to draw text on the screen.
textGL.c is a showcase of some of the features (use the mouse and scroll wheel)
The full character set includes all ASCII characters as well as the full european latin alphabet, greek, and cyrillic alphabets, and some miscellaneous characters.
In order to write those characters, you'll need to turn them into unsigned integers (cast from UTF-8 but reverse the bits but only in bytes that have been set, I should probably include a function to do the conversion)

The library is textGL.h
textGLHeaderTest.c gives an example on how to use the library. I mean nobody is going to use it except me so it doesn't really matter, but in case you were curious.

Functions:
writeString(const char *str, double x, double y, double size, double align)
write(const unsigned int *text, int textLength, double x, double y, double size, double align)

to change colour of the text, do turtlePenColor(r, g, b) before a write call
the r g and b values are doubles from 0 to 255

use writeString in most cases, but if you need to write some greek letters you'll need to use write with the weird unsigned int encoding or whatever. I should include at least a reference of characters to their number and i guess i will when i get around to it


Use on linux:

gcc textGL.c -L./Linux -lglfw3 -ldl -lm -lX11 -lglad -lGL -lGLU -lpthread -O3 -o textGL.o

if it doesn't work you'll probably need to install glad and glfw and compile the libraries (glad and glfw).
once you've obtained the libglad.a and libglfw3.a files, 
replace the ones in the folder called "Linux" and recompile