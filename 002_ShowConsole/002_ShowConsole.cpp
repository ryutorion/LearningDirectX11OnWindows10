// 002_ShowConsole.cpp
#include <Windows.h>
#include <tchar.h>
#include <iostream>
// _wfreopen_s�̂��߂ɕK�v
#include <cstdio>
// _wsetlocale�̂��߂ɕK�v
#include <clocale>

using namespace std;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    // �R���\�[�������蓖�Ă�
    BOOL ConsoleAllocated = AllocConsole();
    if(!ConsoleAllocated)
    {
        return 0;
    }

    // �W�����o�͂�V�����R���\�[���Ɋ֘A�t����
    FILE * pFile = nullptr;
    _wfreopen_s(&pFile, L"CONOUT$", L"w", stdout);
    _wfreopen_s(&pFile, L"CONIN$", L"r", stdin);

    // ���P�[����ݒ肷��
    _wsetlocale(LC_ALL, L".OCP");


    cout << "Hello" << endl;
    wcout << L"����ɂ���" << endl;

    cin.get();

    // �R���\�[�����������
    FreeConsole();

    return 0;
}
