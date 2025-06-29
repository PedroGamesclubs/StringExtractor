#include "StringExtractor.h"
#include "StringFileExtractor.h"

#include <commdlg.h>
#include <shlwapi.h>
#include <fstream>
#include <sstream>

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shlwapi.lib")

StringExtractor::StringExtractor(HINSTANCE hInst) : hInstance(hInst), hwndMain(nullptr) {}

int StringExtractor::run() {
    registerClass();
    createWindow();
    ShowWindow(hwndMain, SW_SHOW);
    UpdateWindow(hwndMain);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

void StringExtractor::registerClass() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"StringExtractorWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.cbWndExtra = sizeof(LONG_PTR);  // para armazenar ponteiro da instância

    RegisterClass(&wc);
}

void StringExtractor::createWindow() {
    hwndMain = CreateWindowEx(
        0,
        L"StringExtractorWindow",
        L"String Extractor",
        WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX), // tamanho fixo
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        nullptr, nullptr, hInstance, this
    );
}

void StringExtractor::createUI(HWND hwnd) {
    buttonBrowse = CreateWindow(L"BUTTON", L"Selecionar Arquivo",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, 20, 200, 30, hwnd, (HMENU)1, hInstance, nullptr);

    buttonExtract = CreateWindow(L"BUTTON", L"Extrair Strings",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        240, 20, 200, 30, hwnd, (HMENU)2, hInstance, nullptr);

    editOutput = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        20, 70, 440, 270, hwnd, nullptr, hInstance, nullptr);

    SendMessage(editOutput, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

LRESULT CALLBACK StringExtractor::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    StringExtractor* app = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        app = reinterpret_cast<StringExtractor*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)app);
    }
    else {
        app = reinterpret_cast<StringExtractor*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (app) {
        switch (msg) {
        case WM_CREATE:
            app->createUI(hwnd);
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case 1: app->onBrowse(); break;
            case 2: app->onExtract(); break;
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void StringExtractor::onBrowse() {
    wchar_t filePath[MAX_PATH] = {};
    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndMain;
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Executáveis\0*.exe\0Todos os arquivos\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        selectedFile = filePath;
        SetWindowText(editOutput, L"Arquivo selecionado: ");
        SetWindowText(editOutput, selectedFile.c_str());
    }
}

void StringExtractor::onExtract() {
    if (selectedFile.empty()) {
        SetWindowText(editOutput, L"Selecione um arquivo primeiro.");
        return;
    }

    StringFileExtractor extractor;
    std::wstring result = extractor.extract(selectedFile);

    SetWindowText(editOutput, result.c_str());
}
