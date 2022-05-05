EXECUTABLE_NAME = tubetube.exe
OBJ_DIRECTORY = tmp
BIN_DIRECTORY = bin

OBJ_FILES = \
	$(OBJ_DIRECTORY)\tubetube.obj \
	$(OBJ_DIRECTORY)\dust3d\base\image.obj \
	$(OBJ_DIRECTORY)\dust3d\gles\icon_map.obj \
	$(OBJ_DIRECTORY)\dust3d\gles\win32\window.obj \
	$(OBJ_DIRECTORY)\dust3d\data\dust3d_vertical_png.obj \
	$(OBJ_DIRECTORY)\dust3d\widget\widget.obj

INCLUDE_DIRECTORIES_OPTIONS = \
	/I "C:\\Libraries\\freetype-windows-binaries-2.11.1\\include" \
	/I "C:\\Users\\Jeremy\\Repositories\\angle\\include" \
	/I "C:\\Users\\Jeremy\\Repositories\\msdfgen" \
	/I "third_party\\nanosvg" \
	/I "third_party" \
	/I "."

LIB_DIRECTORIES_OPTIONS = \
	/LIBPATH:"C:\Users\Jeremy\Repositories\angle\out\Release" User32.lib libEGL.dll.lib libGLESv2.dll.lib \
	/LIBPATH:"C:\Libraries\freetype-windows-binaries-2.11.1\release dll\win64" freetype.lib \
	/LIBPATH:"C:\Users\Jeremy\Repositories\msdfgen\x64\Release Library" msdfgen.lib

COMPILE_OPTIONS = \
	/std:c++latest \
	/source-charset:utf-8 \
	/execution-charset:utf-8 \
	/EHsc \
	/DUNICODE \
	/D_UNICODE \
	$(INCLUDE_DIRECTORIES_OPTIONS)

LINK_OPTIONS = \
	/DEBUG:FULL	\
	$(LIB_DIRECTORIES_OPTIONS)

{}.cc{$(OBJ_DIRECTORY)}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\ $(COMPILE_OPTIONS) $<

{dust3d\base\}.cc{$(OBJ_DIRECTORY)\dust3d\base\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\dust3d\base\ $(COMPILE_OPTIONS) $<

{dust3d\data\}.cc{$(OBJ_DIRECTORY)\dust3d\data\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\dust3d\data\ $(COMPILE_OPTIONS) $<

{dust3d\gles\}.cc{$(OBJ_DIRECTORY)\dust3d\gles\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\dust3d\gles\ $(COMPILE_OPTIONS) $<

{dust3d\gles\win32\}.cc{$(OBJ_DIRECTORY)\dust3d\gles\win32\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\dust3d\gles\win32\ $(COMPILE_OPTIONS) $<

{dust3d\mesh\}.cc{$(OBJ_DIRECTORY)\dust3d\mesh\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\dust3d\mesh\ $(COMPILE_OPTIONS) $<

{dust3d\widget\}.cc{$(OBJ_DIRECTORY)\dust3d\widget\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\dust3d\widget\ $(COMPILE_OPTIONS) $<

$(EXECUTABLE_NAME): $(OBJ_FILES)
	@if not exist $(BIN_DIRECTORY) mkdir $(BIN_DIRECTORY)
	@link /out:$(BIN_DIRECTORY)\$(EXECUTABLE_NAME) $(OBJ_FILES) $(LINK_OPTIONS)

all: $(EXECUTABLE_NAME)