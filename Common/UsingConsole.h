// UsingConsole.h
#pragma once

#ifndef USING_CONSOLE_H_INCLUDED
#define USING_CONSOLE_H_INCLUDED

class UsingConsole
{
public:
    UsingConsole();
    UsingConsole(const UsingConsole &) = delete;
    UsingConsole(const UsingConsole &&) = delete;

    ~UsingConsole();

    UsingConsole & operator=(const UsingConsole &) = delete;
    UsingConsole & operator=(const UsingConsole &&) = delete;

    bool isConsoleAllocated() const { return mConsoleAllocated; }

private:
    bool mConsoleAllocated;
};

#endif