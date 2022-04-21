call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if exist release\tubetube.exe del release\tubetube.exe
if not exist release mkdir release
cd release
cl /std:c++latest /Fe"tubetube.exe" /I "C:\Users\Jeremy\Repositories\angle\include" /I "C:\Libraries\freetype-windows-binaries-2.11.1\include" /I "..\third_party\nanosvg" /I "C:\Users\Jeremy\Repositories\msdfgen" /I "..\third_party" /I "C:\Users\Jeremy\Repositories\tubetube" /EHsc ..\tubetube.cc ..\dust3d\base\image.cc ..\third_party/tga_utils/tga_utils.cpp /link /LIBPATH:"C:\Users\Jeremy\Repositories\angle\out\Release" User32.lib libEGL.dll.lib libGLESv2.dll.lib /LIBPATH:"C:\Libraries\freetype-windows-binaries-2.11.1\release dll\win64" freetype.lib /LIBPATH:"C:\Users\Jeremy\Repositories\msdfgen\x64\Release Library" msdfgen.lib
tubetube.exe