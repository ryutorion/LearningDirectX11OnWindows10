// 002_ShowConsole_RAII.cpp
#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include "UsingConsole.h"

using namespace std;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    UsingConsole uc;

    cout << "Hello" << endl;
    wcout << L"‚±‚ñ‚É‚¿‚í" << endl;

    cin.get();

    return 0;
}
