// UsingConsole.cpp
#include <Windows.h>
#include <tchar.h>
#include <cstdio>
#include <cstdlib>
#include <clocale>
#include "UsingConsole.h"

UsingConsole::UsingConsole() :
    mConsoleAllocated(AllocConsole() != FALSE)
{
    if(isConsoleAllocated())
    {
        FILE * pFile = nullptr;
        _wfreopen_s(&pFile, L"CONOUT$", L"w", stdout);
        _wfreopen_s(&pFile, L"CONIN$", L"r", stdin);
        _wsetlocale(LC_ALL, L".OCP");
    }
}

UsingConsole::~UsingConsole()
{
    if(isConsoleAllocated())
    {
        system("pause");
        FreeConsole();
    }
}
