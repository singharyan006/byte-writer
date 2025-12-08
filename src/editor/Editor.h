#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include "../terminal/Terminal.h"
#include "../utils/Buffer.h"
#include "Row.h"
#include "Syntax.h"

class Editor {
public:
    Editor();
    ~Editor();
    
    // Initialization
    void init();
    void openFile(const std::string& filename);
    
    // Main loop
    void run();
    void refreshScreen();
    void processKeypress();
    
    // Status messages
    void setStatusMessage(const std::string& msg);
    
private:
    // Terminal
    std::unique_ptr<Terminal> terminal;
    int screenRows;
    int screenCols;
    
    // Cursor position
    int cx, cy;          // Cursor x, y in chars
    int rx;              // Cursor x in render space
    int rowOffset;       // Scroll offset
    int colOffset;
    
    // File data
    std::vector<Row> rows;
    std::string filename;
    bool dirty;          // Has unsaved changes
    
    // Syntax highlighting
    SyntaxHighlighter syntaxHighlighter;
    const EditorSyntax* syntax;
    
    // Status bar
    std::string statusMsg;
    std::time_t statusMsgTime;
    
    // Constants
    static constexpr int QUIT_TIMES = 3;
    static constexpr const char* VERSION = "0.0.1";
    
    // Rendering
    void scroll();
    void drawRows(Buffer& buf);
    void drawStatusBar(Buffer& buf);
    void drawMessageBar(Buffer& buf);
    
    // Input handling
    void moveCursor(int key);
    void insertChar(char c);
    void insertNewline();
    void deleteChar();
    
    // File operations
    void save();
    
    // Search
    void find();
    void findCallback(const std::string& query, int key);
    
    // Prompts
    std::string prompt(const std::string& promptText, 
                       std::function<void(const std::string&, int)> callback = nullptr);
    
    // Helpers
    void selectSyntaxHighlight();
    void updateSyntax(int startRow);
};

#endif // EDITOR_H

