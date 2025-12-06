#include "Terminal.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdio>
#include <errno.h>
#endif

Terminal::Terminal() : rawModeEnabled(false) {
#ifdef _WIN32
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hStdin, &originalInputMode);
    GetConsoleMode(hStdout, &originalOutputMode);
#endif
}

Terminal::~Terminal() {
    if (rawModeEnabled) {
        disableRawMode();
    }
}

void Terminal::enableRawMode() {
#ifdef _WIN32
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
    SetConsoleMode(hStdin, mode);

    GetConsoleMode(hStdout, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hStdout, mode);
#else
    if (tcgetattr(STDIN_FILENO, &originalTermios) == -1) {
        perror("tcgetattr");
        exit(1);
    }

    struct termios raw = originalTermios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(1);
    }
#endif
    rawModeEnabled = true;
}

void Terminal::disableRawMode() {
    if (!rawModeEnabled) return;

#ifdef _WIN32
    SetConsoleMode(hStdin, originalInputMode);
    SetConsoleMode(hStdout, originalOutputMode);
#else
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios) == -1) {
        perror("tcsetattr");
    }
#endif
    rawModeEnabled = false;
}

int Terminal::readKey() {
#ifdef _WIN32
    // Windows implementation
    if (!_kbhit()) return -1;
    
    int c = _getch();
    
    // Handle extended keys (arrows, function keys, etc.)
    if (c == 0 || c == 0xE0) {
        c = _getch();
        switch (c) {
            case 72: return static_cast<int>(Key::ARROW_UP);
            case 80: return static_cast<int>(Key::ARROW_DOWN);
            case 75: return static_cast<int>(Key::ARROW_LEFT);
            case 77: return static_cast<int>(Key::ARROW_RIGHT);
            case 71: return static_cast<int>(Key::HOME_KEY);
            case 79: return static_cast<int>(Key::END_KEY);
            case 73: return static_cast<int>(Key::PAGE_UP);
            case 81: return static_cast<int>(Key::PAGE_DOWN);
            case 83: return static_cast<int>(Key::DELETE_KEY);
        }
    }
    
    return c;
#else
    // Unix/Linux implementation
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            perror("read");
            exit(1);
        }
        return -1;  // No key available
    }

    // Escape sequence handling
    if (c == '\x1b') {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) return static_cast<int>(Key::ESCAPE);
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return static_cast<int>(Key::ESCAPE);

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return static_cast<int>(Key::ESCAPE);
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return static_cast<int>(Key::HOME_KEY);
                        case '3': return static_cast<int>(Key::DELETE_KEY);
                        case '4': return static_cast<int>(Key::END_KEY);
                        case '5': return static_cast<int>(Key::PAGE_UP);
                        case '6': return static_cast<int>(Key::PAGE_DOWN);
                        case '7': return static_cast<int>(Key::HOME_KEY);
                        case '8': return static_cast<int>(Key::END_KEY);
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A': return static_cast<int>(Key::ARROW_UP);
                    case 'B': return static_cast<int>(Key::ARROW_DOWN);
                    case 'C': return static_cast<int>(Key::ARROW_RIGHT);
                    case 'D': return static_cast<int>(Key::ARROW_LEFT);
                    case 'H': return static_cast<int>(Key::HOME_KEY);
                    case 'F': return static_cast<int>(Key::END_KEY);
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return static_cast<int>(Key::HOME_KEY);
                case 'F': return static_cast<int>(Key::END_KEY);
            }
        }

        return static_cast<int>(Key::ESCAPE);
    }

    return c;
#endif
}

void Terminal::clearScreen() {
    write("\x1b[2J");      // Clear entire screen
    write("\x1b[H");       // Move cursor to top-left
}

void Terminal::setCursorPosition(int row, int col) {
    std::string seq = "\x1b[" + std::to_string(row) + ";" + std::to_string(col) + "H";
    write(seq);
}

void Terminal::hideCursor() {
    write("\x1b[?25l");
}

void Terminal::showCursor() {
    write("\x1b[?25h");
}

void Terminal::write(const std::string& data) {
#ifdef _WIN32
    DWORD written;
    WriteConsole(hStdout, data.c_str(), static_cast<DWORD>(data.length()), &written, nullptr);
#else
    ::write(STDOUT_FILENO, data.c_str(), data.length());
#endif
}

bool Terminal::getWindowSize(int& rows, int& cols) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
        cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return true;
    }
    return false;
#else
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // Fallback: move cursor to bottom-right and query position
        if (::write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return false;
        return getCursorPosition(rows, cols);
    } else {
        cols = ws.ws_col;
        rows = ws.ws_row;
        return true;
    }
#endif
}

bool Terminal::getCursorPosition(int& rows, int& cols) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
        cols = csbi.dwCursorPosition.X + 1;
        rows = csbi.dwCursorPosition.Y + 1;
        return true;
    }
    return false;
#else
    char buf[32];
    unsigned int i = 0;

    if (::write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return false;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return false;
    if (sscanf(&buf[2], "%d;%d", &rows, &cols) != 2) return false;

    return true;
#endif
}

