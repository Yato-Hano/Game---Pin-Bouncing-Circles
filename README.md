# Game---Pin-Bouncing-Circles
Pin moving bouncing circles with mouse clicks but beware of bombs, the timer, and finite pins. 
<img width="802" height="838" alt="w" src="https://github.com/user-attachments/assets/940966f0-77c4-4731-aa4a-b3ddea059e2c" />

Art Autors:
- bomb100.png truezipp from opengameart
- pin.png unknown
  
Requirements:
- g++ with C++23 support (tested with GCC 16.2.0 from MSYS2 UCRT64)
- SFML 3.1.0
- mingw32-make on MSYS2
  
Build (Windows, MSYS2 UCRT64):
- `pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-sfml
mingw32-make`
- `cd` to the game folder (pins.h, pins.cpp and Makefile)
- `mingw32-make clean` (if error on building)
- `mingw32-make`

Run (from MSYS2 UCRT64 when current directory is the game folder(with pins.exe, bomb100.png etc.)):
- `./pins.exe`
  
Development environment:
- SFML 3.1.0
- Visual Studio 2026
- Microsoft Windows 11
