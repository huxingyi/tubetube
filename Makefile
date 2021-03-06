EXECUTABLE_NAME = tubetube.exe
OBJ_DIRECTORY = tmp
BIN_DIRECTORY = bin

OBJ_FILES = \
	$(OBJ_DIRECTORY)\hu\base\image.obj \
	$(OBJ_DIRECTORY)\hu\gles\icon_map.obj \
	$(OBJ_DIRECTORY)\hu\gles\win32\window.obj \
	$(OBJ_DIRECTORY)\hu\widget\widget.obj \
	$(OBJ_DIRECTORY)\dust3d\desktop\document_window.obj \
	$(OBJ_DIRECTORY)\dust3d\desktop\reference_image_edit_window.obj \
	$(OBJ_DIRECTORY)\dust3d\desktop\main.obj \
    $(OBJ_DIRECTORY)\dust3d\document\ds3_file.obj \
    $(OBJ_DIRECTORY)\dust3d\document\document.obj \
    $(OBJ_DIRECTORY)\dust3d\document\snapshot_xml.obj \
	$(OBJ_DIRECTORY)\dust3d\data\dust3d_vertical_png.obj

INCLUDE_DIRECTORIES_OPTIONS = \
	/I "C:\\Libraries\\freetype-windows-binaries-2.11.1\\include" \
	/I "C:\\Users\\Jeremy\\Repositories\\angle\\include" \
	/I "C:\\Users\\Jeremy\\Repositories\\msdfgen" \
	/I "third_party\\nanosvg" \
	/I "third_party\\rapidxml-1.13" \
	/I "third_party" \
	/I "."

LIB_DIRECTORIES_OPTIONS = \
	User32.lib Comdlg32.lib \
	/LIBPATH:"C:\Users\Jeremy\Repositories\angle\out\Release" libEGL.dll.lib libGLESv2.dll.lib \
	/LIBPATH:"C:\Libraries\freetype-windows-binaries-2.11.1\release dll\win64" freetype.lib \
	/LIBPATH:"C:\Users\Jeremy\Repositories\msdfgen\x64\Release Library" msdfgen.lib

COMPILE_OPTIONS = \
	/std:c++latest \
	/source-charset:utf-8 \
	/execution-charset:utf-8 \
	/EHsc \
	/DUNICODE \
	/D_UNICODE \
	/nologo \
	/MP \
	$(INCLUDE_DIRECTORIES_OPTIONS)

LINK_OPTIONS = \
	/DEBUG:FULL \
	/LTCG \
	/nologo \
	$(LIB_DIRECTORIES_OPTIONS)

{}.cc{$(OBJ_DIRECTORY)}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\ $(COMPILE_OPTIONS) $<

{hu\base\}.cc{$(OBJ_DIRECTORY)\hu\base\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\hu\base\ $(COMPILE_OPTIONS) $<

{dust3d\data\}.cc{$(OBJ_DIRECTORY)\dust3d\data\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\dust3d\data\ $(COMPILE_OPTIONS) $<
    
{dust3d\document\}.cc{$(OBJ_DIRECTORY)\dust3d\document\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\dust3d\document\ $(COMPILE_OPTIONS) $<

{dust3d\desktop\}.cc{$(OBJ_DIRECTORY)\dust3d\desktop\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\dust3d\desktop\ $(COMPILE_OPTIONS) $<

{hu\gles\}.cc{$(OBJ_DIRECTORY)\hu\gles\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\hu\gles\ $(COMPILE_OPTIONS) $<

{hu\gles\win32\}.cc{$(OBJ_DIRECTORY)\hu\gles\win32\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\hu\gles\win32\ $(COMPILE_OPTIONS) $<

{hu\mesh\}.cc{$(OBJ_DIRECTORY)\hu\mesh\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\hu\mesh\ $(COMPILE_OPTIONS) $<

{hu\widget\}.cc{$(OBJ_DIRECTORY)\hu\widget\}.obj::
	@for %%a in ($(OBJ_DIRECTORY)\$<) do @if not exist "%~dpa" mkdir "%~dpa"
	@cl /c /Fo$(OBJ_DIRECTORY)\hu\widget\ $(COMPILE_OPTIONS) $<

$(EXECUTABLE_NAME): $(OBJ_FILES)
	@if not exist $(BIN_DIRECTORY) mkdir $(BIN_DIRECTORY)
    rc /fo"$(OBJ_DIRECTORY)\dust3d.res" dust3d.rc
	@link /out:$(BIN_DIRECTORY)\$(EXECUTABLE_NAME) $(OBJ_FILES) $(OBJ_DIRECTORY)\dust3d.res $(LINK_OPTIONS)

all: $(EXECUTABLE_NAME)