#include "Console.h"

namespace Con {
    namespace {
        HANDLE _oh = 0;
        HANDLE _ih = 0;
        DWORD ocim;
        DWORD ocom;
        DWORD ows;
        CONSOLE_SCREEN_BUFFER_INFOEX _ocsbiex;
        WORD _catt = 0x07;
        CONSOLE_CURSOR_INFO _Ci;

        CHAR_INFO* _ss=nullptr;
        unsigned _sssz;
        HANDLE _getInputHandle()
        {
            return _ih;
        }
        HANDLE _getOutputHandle()
        {
            return _oh;
        }
    }

    void Init(bool save)
    {
        _ih = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(_ih, &ocim);
        SetConsoleMode(_ih, ocim | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
        _oh = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleCursorInfo(_oh, &_Ci);
        GetConsoleMode(_oh, &ocom);
        _ocsbiex.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
        GetConsoleScreenBufferInfoEx(_oh, &_ocsbiex);
        if (save)
        {
            _sssz = _ocsbiex.dwSize.X * _ocsbiex.dwSize.Y;
            _ss = new CHAR_INFO[_sssz];
            SMALL_RECT r;
            r.Left = 0;
            r.Top = 0;
            r.Right = _ocsbiex.dwSize.X - 1;
            r.Bottom = _ocsbiex.dwSize.Y - 1;
            if (!ReadConsoleOutput(_oh, _ss, _ocsbiex.dwSize, { 0,0 }, &r))
            {
                std::cout << "ReadConsoleOuput failed: " << GetLastError() << '\n';
            }
            std::cout << "Success.\n";
        }
        HWND cw = GetWindow();
        ows = GetWindowLong(cw, GWL_STYLE);
    }

    void Restore()
    {
        HWND cw = GetWindow();
        SetWindowLong(cw, GWL_STYLE, ows);
        SetConsoleMode(_ih, ocim);
        SetConsoleMode(_oh, ocom);
        SetConsoleCursorInfo(_oh, &_Ci);
        SetConsoleScreenBufferInfoEx(_oh, &_ocsbiex);
        if (_sssz > 0 && _ss != nullptr) {
            SMALL_RECT r;
            r.Left = 0;
            r.Top = 0;
            r.Right = _ocsbiex.dwSize.X - 1;
            r.Bottom = _ocsbiex.dwSize.Y - 1;
            WriteConsoleOutput(_oh, _ss, _ocsbiex.dwSize, { 0,0 }, &r);
            delete[]_ss;
            _ss = nullptr;
        }
        SetConsoleCursorPosition(_oh, _ocsbiex.dwCursorPosition);
        SetAttr(0x07);
    }

    unsigned EventsAvailable()
    {
        DWORD n;
        GetNumberOfConsoleInputEvents(_getInputHandle(), &n);
        return (unsigned)n;
    }

    void GotoXY(int x, int y)
    {
        COORD c;
        c.X = x;
        c.Y = y;
        SetConsoleCursorPosition(_getOutputHandle(), c);
    }
    void SetAttr(WORD attr)
    {
        SetConsoleTextAttribute(_getOutputHandle(), attr);
        _catt = attr;
    }

    int Width()
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(_getOutputHandle(), &csbi);
        return csbi.dwSize.X;
    }

    int Height()
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(_getOutputHandle(), &csbi);
        return csbi.dwSize.Y;
    }

    void Cls(WORD att)
    {
        COORD c;
        c.X = 0;
        c.Y = 0;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(_getOutputHandle(), &csbi);
        int len = (csbi.srWindow.Right - csbi.srWindow.Left + 1) * (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
        //int len = csbi.dwSize.X * csbi.dwSize.Y;
        DWORD wrtn;
        FillConsoleOutputCharacter(_getOutputHandle(), L' ', len, c, &wrtn);
        FillConsoleOutputAttribute(_getOutputHandle(), att, len, c, &wrtn);
        SetConsoleCursorPosition(_getOutputHandle(), c);
    }
    void Fill(int x, int y, char ch, WORD att, int len)
    {
        COORD c;
        c.X = (short)x;
        c.Y = (short)y;
        DWORD wrtn;
        WCHAR chw = ch;
        FillConsoleOutputAttribute(_getOutputHandle(), att, len, c, &wrtn);
        FillConsoleOutputCharacter(_getOutputHandle(), chw, len, c, &wrtn);
    }

    void Print(int x, int y, std::string s, WORD att) {
        CHAR_INFO buf[256];
        WORD cc = att;
        if (cc == 0)cc = _catt;
        int bi = 0;
        for (unsigned i = 0; i < s.length();)
        {
            if (s[i] == 0x1b) {
                i++;
                cc = *(unsigned char*)&s[i];
                //i++;
            }
            else {
                buf[bi].Char.UnicodeChar = s[i];
                buf[bi].Attributes = cc;
                bi++;
            }
            i++;
        }
        SMALL_RECT r;
        r.Left = x;
        r.Top = y;
        r.Right = x + bi;
        r.Bottom = y;
        WriteConsoleOutput(_getOutputHandle(), buf, { (short)bi,1 }, { 0,0 }, &r);
    }
    void Print(int x, int y, std::wstring s, WORD att)
    {
        CHAR_INFO buf[256];
        WORD cc = att;
        if (cc == 0)cc = _catt;
        int bi = 0;
        for (unsigned i = 0; i < s.length();)
        {
            if (s[i] == 0x1b) {
                ++i;
                cc = *(unsigned*)&s[i];
            }
            else {
                buf[bi].Char.UnicodeChar = s[i];
                buf[bi].Attributes = cc;
                ++bi;
            }
            ++i;
        }
        SMALL_RECT r;
        r.Left = x;
        r.Top = y;
        r.Right = x + bi;
        r.Bottom = y;
        WriteConsoleOutput(_getOutputHandle(), buf, { (short)bi,1 }, { 0,0 }, &r);
    }
    void Printf(int x, int y, const char* fmt, ...) {
        char buf[4096];
        va_list args;
        va_start(args, fmt);
        vsprintf_s(buf, fmt, args);
        Print(x, y, std::string(buf));
        va_end(args);
    }

    void HideCursor() { 
        HANDLE h = _getOutputHandle();
        CONSOLE_CURSOR_INFO info = _Ci;
        info.bVisible = false;
        SetConsoleCursorInfo(h, &info); 
    }
    void ShowCursor() {
        HANDLE h = _getOutputHandle();
        CONSOLE_CURSOR_INFO info = _Ci;
        info.bVisible = true;
        SetConsoleCursorInfo(h, &info);
    }

    bool IsVtSupported()
    {
        static const bool Result = [&]
        {
            DWORD cmode;
            if (!GetConsoleMode(_getOutputHandle(), &cmode))return false;
            if (cmode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) return true;
            if (!SetConsoleMode(_getOutputHandle(), cmode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))return false;
            SetConsoleMode(_getOutputHandle(), cmode);
            return true;
        }();
        return Result;
    }

    void SetSize(int x, int y)
    {
        SMALL_RECT r{ 0,0,(short)(x - 2),(short)(y - 2) };
        SetConsoleCursorPosition(_getOutputHandle(), { (short)0,(short)0 });
        SetConsoleWindowInfo(_getOutputHandle(),true,&r);
        SetConsoleScreenBufferSize(_getOutputHandle(), { (short)x,(short)y });
        r.Right = x - 1;
        r.Bottom = y - 1;
        SetConsoleWindowInfo(_getOutputHandle(), true, &r);
    }

    HWND GetWindow()
    {
        return GetConsoleWindow();
    }

    void SetFixedWindow()
    {
        HWND cw = GetWindow();
        DWORD sty = GetWindowLong(cw, GWL_STYLE);
        sty &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        SetWindowLong(cw, GWL_STYLE, sty);
    }

    void DrawFrame(int x, int y, int w, int h, WORD att, std::wstring pat, bool filled)
    {
        int wc = Width();
        int hc = Height();
        if (x < 0)x = 0;
        if (y < 0)y = 0;
        if (x + w > wc) w = wc - x;
        if (y + h > hc) h = hc - y;
        if (w <= 0)return;
        if (h <= 0)return;
        CHAR_INFO* buf = new CHAR_INFO[(ULONGLONG)w * (ULONGLONG)h];
        SMALL_RECT r = { (short)x,(short)y,(short)(x + w - 1),(short)(y + h - 1) };
        ReadConsoleOutput(_getOutputHandle(), buf, { (short)w,(short)h }, { 0,0 }, &r);
        // pat = "╔═╗║ ║╚═╝"
        buf[0].Char.UnicodeChar = pat[0];
        buf[0].Attributes = att;
        buf[w-1].Char.UnicodeChar = pat[2];
        buf[w-1].Attributes = att;
        buf[(h - 1) * w].Char.UnicodeChar = pat[6];
        buf[(h - 1) * w].Attributes = att;
        buf[(h - 1) * w + w - 1].Char.UnicodeChar = pat[8];
        buf[(h - 1) * w + w - 1].Attributes = att;
        for (int i = 1; i < w-1; ++i)
        {
            buf[i].Char.UnicodeChar = pat[1];
            buf[i].Attributes = att;
            buf[(h - 1) * w + i].Char.UnicodeChar = pat[7];
            buf[(h - 1) * w + i].Attributes = att;
        }
        for (int j = 1; j < h - 1; ++j)
        {
            buf[j * w].Char.UnicodeChar = pat[3];
            buf[j * w].Attributes = att;
            buf[j * w + w - 1].Char.UnicodeChar = pat[5];
            buf[j * w + w - 1].Attributes = att;
            if (filled) {
                for (int i = 1; i < w - 1; ++i)
                {
                    buf[j * w + i].Char.UnicodeChar = pat[4];
                    buf[j * w + i].Attributes = att;
                }
            }
        }
        r = { (short)x,(short)y,(short)(x + w - 1),(short)(y + h - 1) };
        WriteConsoleOutput(_getOutputHandle(), buf, { (short)w,(short)h }, { 0,0 }, &r);
    }

    void Window::draw()
    {
        SMALL_RECT r = reg;
        COORD bc;
        bc.X = 0;
        bc.Y = wpos;
        WriteConsoleOutput(_getOutputHandle(), buf, sz, bc, &r);
    }

    void Window::_moveright()
    {
        int hh = reg.Bottom - reg.Top + 1;
        cp.X++;
        if (cp.X >= sz.X) {
            cp.X = 0;
            cp.Y++;
            if (cp.Y >= wpos + hh)
            {
                if (wpos + hh < nlines)
                {
                    wpos++;
                }
                else {
                    cp.Y--;
                    _scrollup();
                }
            }
        }
    }

    void Window::_moveleft()
    {
        if (cp.X > 0)
        {
            cp.X--;
        }
    }

    void Window::_movedown()
    {
        int hh = reg.Bottom - reg.Top + 1;
        cp.Y++;
        if (cp.Y >= wpos + hh)
        {
            if (wpos + hh < nlines)
            {
                wpos++;
            }
            else {
                cp.Y--;
                _scrollup();
            }
        }
    }

    void Window::_scrollup()
    {
        memcpy((void*)&buf[0], (void*)&buf[sz.X], (bufsize - sz.X) * sizeof(CHAR_INFO));
        for (int i = 0; i < sz.X; ++i)
        {
            buf[(sz.Y - 1) * sz.X + i].Char.UnicodeChar = L' ';
            buf[(sz.Y - 1) * sz.X + i].Attributes = catt;
        }
    }

    void Window::_write(char* s)
    {
        int hh = reg.Bottom - reg.Top + 1;
        int tb;
        while (*s)
        {
            switch (*s)
            {
                case 0x1b:
                    ++s;
                    catt = (unsigned)*s;
                    break;
                case '\n':
                    _movedown();
                    cp.X = 0; // with /r !!
                    break;
                case '\r':
                    cp.X = 0;
                    break;
                case '\t':
                    tb = (cp.X / 8 + 1) * 8;
                    while (cp.X < tb) {
                        buf[cp.Y * sz.X + cp.X].Char.UnicodeChar = L' ';
                        buf[cp.Y * sz.X + cp.X].Attributes = catt;
                        _moveright();
                        if (cp.X == 0)
                        {
                            if (tb > sz.X) {
                                tb -= sz.X;
                            }
                        }
                    }
                    break;
                case 0x08:
                    _moveleft();
                    buf[cp.Y * sz.X + cp.X].Char.UnicodeChar = L' ';
                    buf[cp.Y * sz.X + cp.X].Attributes = catt;
                    break;
                default:
                    buf[cp.Y * sz.X + cp.X].Char.UnicodeChar = (wchar_t)*s;
                    buf[cp.Y * sz.X + cp.X].Attributes = catt;
                    _moveright();
                    break;
            }
            ++s;
        }
        draw();
        int cx=reg.Left+cp.X;
        int cy=reg.Top+(cp.Y-wpos);
        GotoXY(cx, cy);
    }

    void Window::_write(wchar_t* s)
    {
        int hh = reg.Bottom - reg.Top + 1;
        int tb;
        while (*s)
        {
            switch (*s)
            {
            case 0x1b:
                ++s;
                catt = (unsigned)*s;
                break;
            case '\n':
                _movedown();
                cp.X = 0; // with /r !!
                break;
            case '\r':
                cp.X = 0;
                break;
            case '\t':
                tb = (cp.X / 8 + 1) * 8;
                while (cp.X < tb) {
                    buf[cp.Y * sz.X + cp.X].Char.UnicodeChar = L' ';
                    buf[cp.Y * sz.X + cp.X].Attributes = catt;
                    _moveright();
                    if (cp.X == 0)
                    {
                        if (tb > sz.X) {
                            tb -= sz.X;
                        }
                    }
                }
                break;
            case 0x08:
                _moveleft();
                buf[cp.Y * sz.X + cp.X].Char.UnicodeChar = L' ';
                buf[cp.Y * sz.X + cp.X].Attributes = catt;
                break;
            default:
                buf[cp.Y * sz.X + cp.X].Char.UnicodeChar = (wchar_t)*s;
                buf[cp.Y * sz.X + cp.X].Attributes = catt;
                _moveright();
                break;
            }
            ++s;
        }
        draw();
        int cx = reg.Left + cp.X;
        int cy = reg.Top + (cp.Y - wpos);
        GotoXY(cx, cy);
    }

    Window::Window(int x, int y, int w, int h, int nl)
    {
        int wc = Width();
        int hc = Height();
        if (x < 0)x = 0;
        if (y < 0)y = 0;
        if (x + w > wc) w = wc - x;
        if (y + h > hc) h = hc - y;
        if (w <= 0 || h <= 0) {
            throw std::exception("Invalid Window size!");
        }
        reg.Left = x;
        reg.Top = y;
        reg.Right = x + w - 1;
        reg.Bottom = y + h - 1;
        cp.X = 0;
        cp.Y = 0;
        nlines = nl;
        bufsize = w * nlines;
        sz.X = w;
        sz.Y = nlines;
        buf = new CHAR_INFO[bufsize];
        for (int i = 0; i < bufsize; ++i) {
            buf[i].Attributes = 0x07;
            buf[i].Char.UnicodeChar = L' ';
        }
        wpos = 0;
        catt = 0x07;
        draw();
    }

    Window::~Window()
    {
        delete[]buf;
    }

    void Window::Clear()
    {
        Clear(L' ', catt);
    }

    void Window::Clear(wchar_t c)
    {
        Clear(c, catt);
    }

    void Window::Clear(wchar_t c, WORD att)
    {
        for (int i = 0; i < bufsize; ++i) {
            buf[i].Attributes = att;
            buf[i].Char.UnicodeChar = c;
        }
        catt = att;
        wpos = 0;
        cp.X = 0;
        cp.Y = 0;
        draw();
    }

    void Window::SetAttr(WORD att)
    {
        catt = att;
    }

    void Window::Write(const char* fmt, ...) {
        char b[4096];
        va_list args;
        va_start(args, fmt);
        vsprintf_s(b, fmt, args);
        _write(b);
        va_end(args);
    }

    void Window::Write(const wchar_t* fmt, ...) {
        wchar_t b[4096];
        va_list args;
        va_start(args, fmt);
        vswprintf_s(b, fmt, args);
        _write(b);
        va_end(args);
    }


}

