#ifndef WINDOW_HPP
#define WINDOW_HPP

#define NOMINMAX

#include<Windows.h>
#include<iostream>
#include<d2d1.h>
#include<wrl.h>
#include<thread>
#include<chrono>
#include<cmath>
#include<bitset>
#include<fstream>

int 
    windowWidth,
    windowHeight
;

unsigned int 
    bitmapWidth, 
    bitmapHeight
;

HINSTANCE hInst = 0;

HWND hWnd = nullptr;
Microsoft::WRL::ComPtr<ID2D1Factory> pFactory = nullptr;
Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> pTarget = nullptr;

D2D1_POINT_2U mousePos = {};
unsigned int LMB, RMB, MMB;
std::bitset<256u> keys;

RECT proc_rect;
POINTS proc_point;
LRESULT WINAPI WindowProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam){
    switch(_msg){
        case WM_DESTROY:
        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        case WM_MOUSEMOVE:
            proc_point = MAKEPOINTS(_lParam);
            mousePos.x = proc_point.x;
            mousePos.y = proc_point.y;
            break;

        case WM_LBUTTONDOWN:
            LMB = 1;
            break;
        case WM_LBUTTONUP:
            LMB = 0;
            break;
        case WM_RBUTTONDOWN:
            RMB = 1;
            break;
        case WM_RBUTTONUP:
            RMB = 0;
            break;
        case WM_MBUTTONDOWN:
            MMB = 1;
            break;
        case WM_MBUTTONUP:
            MMB = 0;
            break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            keys[(unsigned char)_wParam] = true;
            std::cout << _wParam << '\n';
            break;

        case WM_SYSKEYUP:
        case WM_KEYUP:
            keys[(unsigned char)_wParam] = false;
            break;
    }
    return DefWindowProcA(_hWnd, _msg, _wParam, _lParam);
}
void InitWindow(){
    std::ifstream file;
    file.open("Settings.txt", std::ios::binary | std::ios::in);

    std::string input;
    file >> input;

    std::cout << input << '\n';


    unsigned long long ctr = 0;
    std::string token = "";

    while(input[ctr] != '@'){
        token.push_back(input[ctr++]);
    } 
    ++ctr;
    windowWidth = std::stoi(token);
    token = "";

    while(input[ctr] != '#'){
        token.push_back(input[ctr++]);
    }
    ++ctr;
    windowHeight = std::stoi(token);
    token = "";

    while(input[ctr] != ':'){
        token.push_back(input[ctr++]);
    } 
    ++ctr;
    bitmapWidth = std::stoi(token);
    token = "";

    while(input[ctr] != '~'){
        token.push_back(input[ctr++]);
    }
    ++ctr;
    bitmapHeight = std::stoi(token);
    token = "";

    file.close();

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = "DrawWindowClass";
    wc.hIcon = static_cast<HICON>(LoadImageA(hInst, "draw_icon.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE));
    
    if(RegisterClassExA(&wc) == 0){
        throw "Failed creating window class";
    }

    RECT csRect = {0};
    csRect.right = windowWidth;
    csRect.bottom = windowHeight;
    AdjustWindowRect(&csRect, WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX, false);

    hWnd = CreateWindowExA(
        0, wc.lpszClassName, "Draw Window",
        WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        csRect.right - csRect.left , csRect.bottom - csRect.top,
        nullptr, 0, nullptr, nullptr
    );
    if(hWnd == nullptr){
        throw "Failed creating window";
    }

    

    ShowWindow(hWnd, SW_SHOW);

    D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        __uuidof(ID2D1Factory),
        &pFactory
    );

    pFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_HARDWARE,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
        ),
        D2D1::HwndRenderTargetProperties(
            hWnd, D2D1::SizeU(windowWidth, windowHeight)
        ),
        &pTarget
    );
}
void DeleteWindow(){
    DestroyWindow(hWnd);
    UnregisterClassA("DrawWindowClass", nullptr);
}
MSG msg;
int ShouldClose(){
    if(PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)){
        if(msg.message == WM_QUIT){
            return true;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return false;
}

std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();
void ClearScreen(){
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timeStart);

    float x = dur.count() / 500.0f;

    pTarget->BeginDraw();
    pTarget->Clear({0.0f, (sinf(x - (3.14159f / 3.0f)) + 0.5f) / 8.0f, (sinf(x) + 0.5f) / 8.0f, 1.0f});
}
void PresentScreen(){
    pTarget->EndDraw();
}

D2D1_COLOR_F* pixels;
Microsoft::WRL::ComPtr<ID2D1Bitmap> pBitmap = nullptr;

void PutPixel(D2D1_POINT_2U _pos, D2D1_COLOR_F _col = {1.0f, 1.0f, 1.0f, 1.0f}){
    pixels[bitmapWidth * _pos.y + _pos.x] = _col;
}

void CreateApp(){
    pixels = new D2D1_COLOR_F[bitmapWidth * bitmapHeight];
    for(unsigned int y = 0; y < bitmapHeight; ++y){
        for(unsigned int x = 0; x < bitmapWidth; ++x){
            PutPixel({x, y}, {0.0f, 0.0f, 0.0f, 0.0f});
        }
    }
}

D2D1_COLOR_F curColour = {1.0f, 1.0f, 1.0f, 1.0f};
D2D1_COLOR_F lastColour = curColour;
void UpdateApp(){
    if(LMB){
        PutPixel(
            {(mousePos.x * bitmapWidth) / windowWidth, (mousePos.y * bitmapHeight) / windowHeight},
            curColour
        );
    }
    if(MMB){
        curColour = pixels[bitmapWidth * ((mousePos.y * bitmapHeight) / windowHeight) + ((mousePos.x * bitmapWidth) / windowWidth)];
    }
    if(RMB){
        std::cout << "enter new colour (rgba in vals between 0.0f and 1.0f)\n";
        float r, g, b, a;
        std::cin >> r;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin >> g; 
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin >> b; 
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin >> a; 
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "creating colour\n";
        lastColour = curColour = {r, g, b, a};
        std::cout << "done\n";

        RMB = false;
    }

    if(keys['B'] == true){
        curColour = lastColour;
    }
    else if(keys['E'] == true){
        curColour = {0.0f, 0.0f, 0.0f, 0.0f};
    }
    else if(keys['1'] == true){
        lastColour = curColour = {1.0f, 1.0f, 1.0f, 1.0f};
    }
    else if(keys['2'] == true){
        lastColour = curColour = {0.0f, 0.0f, 0.0f, 1.0f};
    }
    else if(keys['3'] == true){
        lastColour = curColour = {1.0f, 0.0f, 0.0f, 1.0f};
    }
    else if(keys['4'] == true){
        lastColour = curColour = {0.0f, 1.0f, 0.0f, 1.0f};
    }
    else if(keys['5'] == true){
        lastColour = curColour = {0.0f, 0.0f, 1.0f, 1.0f};
    }
    
    else if(keys['C']){
        for(unsigned int y = 0; y < bitmapHeight; ++y){
            for(unsigned int x = 0; x < bitmapWidth; ++x){
                PutPixel({x, y}, {0.0f, 0.0f, 0.0f, 0.0f});
            }
        }
    }

    else if(keys['S']){
        std::ofstream file;
        std::string fileName;
        
        std::cout << "save file to: ";
        std::getline(std::cin, fileName);
        fileName.append(".dxBitmap");
        file.open(fileName, std::ios::binary | std::ios::out);

        file << bitmapWidth << ':' << bitmapHeight << '~';
        for(unsigned int y = 0; y < bitmapHeight; ++y){
            for(unsigned int x = 0; x < bitmapWidth; ++x){
                D2D1_COLOR_F col = pixels[bitmapWidth * y + x];
                file << 
                    int(col.r * 255) << ',' <<
                    int(col.g * 255) << ',' <<
                    int(col.b * 255) << ',' <<
                    int(col.a * 255) << '|'
                ;
            }
        }
        file << '?';
        file.close();

        keys['S'] = false;
    }
    else if(keys['R']){
        std::ifstream file;
        std::string fileName;

        std::cout << "read from: ";
        std::getline(std::cin, fileName);
        fileName.append(".dxBitmap");
        file.open(fileName, std::ios::binary | std::ios::in);

        //char input[std::numeric_limits<long>::max()];
        std::string input;
        file >> input;

        std::cout << input << '\n';


        unsigned long long ctr = 0;
        std::string token = "";
        while(input[ctr] != ':'){
            token.push_back(input[ctr++]);
        } 
        ++ctr;
        bitmapWidth = std::stoi(token);
        token = "";

        while(input[ctr] != '~'){
            token.push_back(input[ctr++]);
        }
        ++ctr;
        bitmapHeight = std::stoi(token);
        token = "";


        delete[] pixels;
        pixels = new D2D1_COLOR_F[bitmapWidth * bitmapHeight];
        //D2D1_COLOR_F* t_pixels = new D2D1_COLOR_F[t_bitmapWidth * t_bitmapHeight];

        D2D1_COLOR_F col;
        unsigned long long pixelPos = 0; 
        while(input[ctr] != '?'){

            while(input[ctr] != ','){
                token.push_back(input[ctr++]);
            }
            ++ctr;
            col.r = float(std::stoi(token))/255.0f;
            token = "";
            
            while(input[ctr] != ','){
                token.push_back(input[ctr++]);
            }
            ++ctr;
            col.g = float(std::stoi(token))/255.0f;
            token = "";

            while(input[ctr] != ','){
                token.push_back(input[ctr++]);
            }
            ++ctr;
            col.b = float(std::stoi(token))/255.0f;
            token = "";

            while(input[ctr] != '|'){
                token.push_back(input[ctr++]);
            }
            ++ctr;
            col.a = float(std::stoi(token))/255.0f;
            token = "";            

            pixels[pixelPos++] = col;
        }

        file.close();
        keys['R'] = false;
    }

    if(FAILED(pTarget->CreateBitmap(
        D2D1::SizeU(bitmapWidth, bitmapHeight),
        pixels, sizeof(D2D1_COLOR_F) * bitmapWidth,
        D2D1::BitmapProperties(
            D2D1::PixelFormat(
                DXGI_FORMAT_R32G32B32A32_FLOAT, D2D1_ALPHA_MODE_PREMULTIPLIED
            )
        ),
        &pBitmap
    ))){
        throw "Failed creating bitmap";
    }
    pTarget->DrawBitmap(
        pBitmap.Get(),
        D2D1::RectF(0, 0, windowWidth, windowHeight),
        1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
    );

}
void DeleteApp(){
    delete[] pixels;
}

#endif