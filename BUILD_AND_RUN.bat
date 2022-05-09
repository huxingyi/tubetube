call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
REM rmdir /s /q tmp
nmake
cd bin
tubetube.exe