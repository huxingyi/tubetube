call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
rmdir /s /q tmp
del bin\tubetube.exe
nmake
cd bin
tubetube.exe