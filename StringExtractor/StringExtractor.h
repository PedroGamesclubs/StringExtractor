#pragma once

#include <windows.h>
#include <string>

class StringExtractor {
public:
    explicit StringExtractor(HINSTANCE hInstance);
    int run();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void registerClass();
    void createWindow();
    void createUI(HWND hwnd);
    void onBrowse();
    void onExtract();

    HINSTANCE hInstance;
    HWND hwndMain;
    HWND buttonBrowse;
    HWND buttonExtract;
    HWND editOutput;

    std::wstring selectedFile;
};
