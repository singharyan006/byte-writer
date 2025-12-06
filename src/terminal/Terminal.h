#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

// Special key codes (values > 255 to avoid conflicts with ASCII)
enum class Key {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DELETE_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
    BACKSPACE = 127,
    ESCAPE = 27,
    ENTER = 13
};

class Terminal {
public:
    Terminal();
    ~Terminal();

    // Terminal mode control
    void enableRawMode();
    void disableRawMode();

    // Input
    int readKey();  // Returns ASCII for normal keys, or static_cast<int>(Key::XXX) for special keys

    // Output
    void clearScreen();
    void setCursorPosition(int row, int col);
    void hideCursor();
    void showCursor();
    void write(const std::string& data);

    // Terminal info
    bool getWindowSize(int& rows, int& cols);

    // Utility
    static int ctrlKey(int k) { return k & 0x1f; }

private:
    bool rawModeEnabled;

#ifdef _WIN32
    HANDLE hStdin;
    HANDLE hStdout;
    DWORD originalInputMode;
    DWORD originalOutputMode;
#else
    struct termios originalTermios;
#endif

    bool getCursorPosition(int& rows, int& cols);
};

#endif // TERMINAL_H

