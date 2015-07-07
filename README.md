#How to Settings

1. Download SDL2
     -SDL2 (http://www.libsdl.org/download-2.0.php) - SDL2-devel-2.0.3-mingw.tar.gz (MinGW 32/64-bit)
     -SDL2_image (http://www.libsdl.org/projects/SDL_image/) - SDL2_image-devel-2.0.0-mingw.tar.gz (MinGW 32/64-bit)
     -SDL2_mixer (http://www.libsdl.org/projects/SDL_mixer/) - SDL2_mixer-devel-2.0.0-mingw.tar.gz (MinGW 32/64-bit)
   Extract File to C:/SDL/* - SDL2-2.0.3, SDL2_image-2.0.0, SDL2_mixer-2.0.0
   
2. Download Glew
     -GLEW (http://glew.sourceforge.net/) - Source TGZ
   Extract File Glew to C:/* - glew-1.12.0
      Create Compile.bat in Directory and Copy and Paste Code to Compile.bat
::: CODE :::
gcc -DGLEW_NO_GLU -O2 -Wall -W -Iinclude  -DGLEW_BUILD -o src/glew.o -c src/glew.c
gcc -shared -Wl,-soname,libglew32.dll -Wl,--out-implib,lib/libglew32.dll.a    -o lib/glew32.dll src/glew.o -L/mingw/lib -lglu32 -lopengl32 -lgdi32 -luser32 -lkernel32
ar cr lib/libglew32.a src/glew.o
gcc -DGLEW_NO_GLU -DGLEW_MX -O2 -Wall -W -Iinclude  -DGLEW_BUILD -o src/glew.mx.o -c src/glew.c
gcc -shared -Wl,-soname,libglew32mx.dll -Wl,--out-implib,lib/libglew32mx.dll.a -o lib/glew32mx.dll src/glew.mx.o -L/mingw/lib -lglu32 -lopengl32 -lgdi32 -luser32 -lkernel32
ar cr lib/libglew32mx.a src/glew.mx.o
::::::::::::
      Double-Click Compile.bat for Building Library
      
3. Download GLM
     -GLM (http://glm.g-truc.net)
   Extract File to C:/SDL/* - glm
   
4. Compile Sample - Double-Click CompileAndRun.bat
   You can change File for Compile - CompileAndRun.bat : Line 10

Enjoy.
