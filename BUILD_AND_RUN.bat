call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
del tubetube.exe
cl /I "C:\Users\Jeremy\Repositories\angle\include" /I "C:\Users\Jeremy\Repositories\tubetube" /EHsc tubetube.cc third_party/tga_utils/tga_utils.cpp /link /LIBPATH:"C:\Users\Jeremy\Repositories\angle\out\Release" User32.lib libEGL.dll.lib libGLESv2.dll.lib
tubetube.exe