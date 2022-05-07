#include <dust3d/document_window.h>

int main(int argc, char* argv[])
{
    new DocumentWindow();
    
    Window::mainLoop();
    
    return 0;
}