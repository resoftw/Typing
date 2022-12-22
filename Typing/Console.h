#pragma once
#include <iostream>
#include <Windows.h>

namespace Con {
    namespace Box {
        const std::wstring Double = L"╔═╗║ ║╚═╝";
        const std::wstring Single = L"┌─┐│ │└─┘";
        const std::wstring SingleRounded = L"╭─╮│ │╰─╯";
    }

    class Window {
    private:
        CHAR_INFO* buf=nullptr;
        int nlines;
        SMALL_RECT reg;
        COORD cp;
        COORD sz;
        int bufsize;
        int wpos;
        WORD catt;
        void draw();
        void _moveright();
        void _moveleft();
        void _movedown();
        void _scrollup();
        void _write(char* s);
        void _write(wchar_t* s);
    public:
        Window(int x, int y, int w, int h, int nl = 4096);
        ~Window();
        void Clear();
        void Clear(wchar_t c);
        void Clear(wchar_t c, WORD att);
        void SetAttr(WORD att);
        void Write(const char* fmt, ...);
        void Write(const wchar_t* fmt, ...);
    };


    void Init(bool save=false);
    void Restore();
    unsigned EventsAvailable();

    void GotoXY(int x, int y);
    void SetAttr(WORD attr);
    int Width();
    int Height();
    void Cls(WORD att = 0x07);
    void Fill(int x, int y, char ch, WORD att, int len);
    void Print(int x, int y, std::string s, WORD att = 0);
    void Print(int x, int y, std::wstring s, WORD att = 0);
    void Printf(int x, int y, const char* fmt, ...);
    void HideCursor();
    void ShowCursor();
    bool IsVtSupported();
    void SetSize(int x, int y);
    HWND GetWindow();
    void SetFixedWindow();
    void DrawFrame(int x, int y, int w, int h, WORD att, std::wstring pat, bool filled=false);

}
