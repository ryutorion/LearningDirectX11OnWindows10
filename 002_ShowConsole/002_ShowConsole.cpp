// 002_ShowConsole.cpp
#include <Windows.h>
#include <tchar.h>
#include <iostream>
// _wfreopen_sのために必要
#include <cstdio>
// _wsetlocaleのために必要
#include <clocale>

using namespace std;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    // コンソールを割り当てる
    BOOL ConsoleAllocated = AllocConsole();
    if(!ConsoleAllocated)
    {
        return 0;
    }

    // 標準入出力を新しいコンソールに関連付ける
    FILE * pFile = nullptr;
    _wfreopen_s(&pFile, L"CONOUT$", L"w", stdout);
    _wfreopen_s(&pFile, L"CONIN$", L"r", stdin);

    // ロケールを設定する
    _wsetlocale(LC_ALL, L".OCP");


    cout << "Hello" << endl;
    wcout << L"こんにちわ" << endl;

    cin.get();

    // コンソールを解放する
    FreeConsole();

    return 0;
}
