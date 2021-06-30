#include"App.hpp"

INT WINAPI WinMain(
    _In_ HINSTANCE _hInstance,
    _In_opt_ HINSTANCE _hPrevInstance,
    _In_ LPSTR _sCmdLine,
    _In_ INT _nShowCmd
){
    hInst = _hInstance;
    try{
        InitWindow();
        CreateApp();
        std::cout << bitmapWidth << " : " << bitmapHeight << '\n';
        while(!ShouldClose()){
            ClearScreen();
            UpdateApp();
            PresentScreen();
        }

        DeleteApp();
        DeleteWindow();
    }
    catch (const char* str){
        std::cout << str << '\n';
        system("pause");
    }
    catch(...){

    }
    return 0;
}