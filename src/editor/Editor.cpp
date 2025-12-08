#include "Editor.h"
#include "FileIO.h"
#include "../utils/Helpers.h"
#include <iostream>
#include <algorithm>
#include <functional>

Editor::Editor() 
    : cx(0), cy(0), rx(0), rowOffset(0), colOffset(0),
      dirty(false), syntax(nullptr), statusMsgTime(0) {
    terminal = std::make_unique<Terminal>();
}

Editor::~Editor() {
}

void Editor::init() {
    terminal->enableRawMode();
    
    if (!terminal->getWindowSize(screenRows, screenCols)) {
        std::cerr << "Failed to get window size\n";
        exit(1);
    }
    
    // Reserve 2 lines for status bar and message bar
    screenRows -= 2;
    
    setStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
}

void Editor::openFile(const std::string& fname) {
    filename = fname;
    
    if (FileIO::loadFile(filename, rows)) {
        // Update row indices
        for (size_t i = 0; i < rows.size(); i++) {
            rows[i].setIndex(i);
        }
        dirty = false;
    }
    
    selectSyntaxHighlight();
    updateSyntax(0);
}

void Editor::run() {
    while (true) {
        refreshScreen();
        processKeypress();
    }
}

void Editor::refreshScreen() {
    scroll();
    
    Buffer buf;
    
    terminal->hideCursor();
    buf.append("\x1b[H");  // Move cursor to top-left
    
    drawRows(buf);
    drawStatusBar(buf);
    drawMessageBar(buf);
    
    // Position cursor
    std::string cursorPos = "\x1b[" + 
        std::to_string((cy - rowOffset) + 1) + ";" +
        std::to_string((rx - colOffset) + 1) + "H";
    buf.append(cursorPos);
    
    terminal->showCursor();
    
    terminal->write(buf.getData());
}

void Editor::scroll() {
    rx = 0;
    if (cy < static_cast<int>(rows.size())) {
        rx = rows[cy].cxToRx(cx);
    }
    
    // Vertical scroll
    if (cy < rowOffset) {
        rowOffset = cy;
    }
    if (cy >= rowOffset + screenRows) {
        rowOffset = cy - screenRows + 1;
    }
    
    // Horizontal scroll
    if (rx < colOffset) {
        colOffset = rx;
    }
    if (rx >= colOffset + screenCols) {
        colOffset = rx - screenCols + 1;
    }
}

void Editor::drawRows(Buffer& buf) {
    for (int y = 0; y < screenRows; y++) {
        int filerow = y + rowOffset;
        
        if (filerow >= static_cast<int>(rows.size())) {
            // Empty row
            if (rows.empty() && y == screenRows / 3) {
                // Welcome message
                std::string welcome = "ByteWriter -- version " + std::string(VERSION);
                if (welcome.length() > static_cast<size_t>(screenCols)) {
                    welcome = welcome.substr(0, screenCols);
                }
                
                int padding = (screenCols - welcome.length()) / 2;
                if (padding) {
                    buf.append("~");
                    padding--;
                }
                while (padding-- > 0) buf.append(" ");
                buf.append(welcome);
            } else {
                buf.append("~");
            }
        } else {
            // Actual file row
            const Row& row = rows[filerow];
            int len = row.getRenderSize() - colOffset;
            if (len < 0) len = 0;
            if (len > screenCols) len = screenCols;
            
            const std::string& render = row.getRender();
            const auto& hl = row.getHighlight();
            
            int currentColor = -1;
            for (int j = 0; j < len; j++) {
                int idx = j + colOffset;
                char c = render[idx];
                
                if (std::iscntrl(c)) {
                    char sym = (c <= 26) ? '@' + c : '?';
                    buf.append("\x1b[7m");  // Invert colors
                    buf.append(std::string(1, sym));
                    buf.append("\x1b[m");   // Reset
                    if (currentColor != -1) {
                        std::string colorSeq = "\x1b[" + std::to_string(currentColor) + "m";
                        buf.append(colorSeq);
                    }
                } else if (hl[idx] == Highlight::NORMAL) {
                    if (currentColor != -1) {
                        buf.append("\x1b[39m");  // Default color
                        currentColor = -1;
                    }
                    buf.append(std::string(1, c));
                } else {
                    int color = SyntaxHighlighter::syntaxToColor(hl[idx]);
                    if (color != currentColor) {
                        currentColor = color;
                        std::string colorSeq = "\x1b[" + std::to_string(color) + "m";
                        buf.append(colorSeq);
                    }
                    buf.append(std::string(1, c));
                }
            }
            buf.append("\x1b[39m");  // Reset to default color
        }
        
        buf.append("\x1b[K");    // Clear rest of line
        buf.append("\r\n");
    }
}

void Editor::drawStatusBar(Buffer& buf) {
    buf.append("\x1b[7m");  // Invert colors
    
    std::string status = (filename.empty() ? "[No Name]" : filename) + 
                        " - " + std::to_string(rows.size()) + " lines" +
                        (dirty ? " (modified)" : "");
    
    std::string rstatus = (syntax ? syntax->filetype : "no ft") + " | " +
                         std::to_string(cy + 1) + "/" + std::to_string(rows.size());
    
    int len = std::min(static_cast<int>(status.length()), screenCols);
    buf.append(status.substr(0, len));
    
    while (len < screenCols) {
        if (screenCols - len == static_cast<int>(rstatus.length())) {
            buf.append(rstatus);
            break;
        } else {
            buf.append(" ");
            len++;
        }
    }
    
    buf.append("\x1b[m");   // Reset colors
    buf.append("\r\n");
}

void Editor::drawMessageBar(Buffer& buf) {
    buf.append("\x1b[K");  // Clear line
    
    int msglen = std::min(static_cast<int>(statusMsg.length()), screenCols);
    if (msglen && std::time(nullptr) - statusMsgTime < 5) {
        buf.append(statusMsg.substr(0, msglen));
    }
}

void Editor::processKeypress() {
    static int quitTimes = QUIT_TIMES;
    
    int c = terminal->readKey();
    if (c == -1) return;  // No key available
    
    if (c == '\r' || c == static_cast<int>(Key::ENTER)) {
        insertNewline();
    } else if (c == Terminal::ctrlKey('q')) {
        if (dirty && quitTimes > 0) {
            setStatusMessage("WARNING!!! File has unsaved changes. Press Ctrl-Q " +
                           std::to_string(quitTimes) + " more times to quit.");
            quitTimes--;
            return;
        }
        terminal->clearScreen();
        terminal->setCursorPosition(1, 1);
        exit(0);
    } else if (c == Terminal::ctrlKey('s')) {
        save();
    } else if (c == Terminal::ctrlKey('f')) {
        find();
    } else if (c == static_cast<int>(Key::HOME_KEY)) {
        cx = 0;
    } else if (c == static_cast<int>(Key::END_KEY)) {
        if (cy < static_cast<int>(rows.size())) {
            cx = rows[cy].getSize();
        }
    } else if (c == static_cast<int>(Key::BACKSPACE) || 
               c == Terminal::ctrlKey('h') || 
               c == 127) {
        deleteChar();
    } else if (c == static_cast<int>(Key::DELETE_KEY)) {
        moveCursor(static_cast<int>(Key::ARROW_RIGHT));
        deleteChar();
    } else if (c == static_cast<int>(Key::PAGE_UP) || 
               c == static_cast<int>(Key::PAGE_DOWN)) {
        if (c == static_cast<int>(Key::PAGE_UP)) {
            cy = rowOffset;
        } else {
            cy = rowOffset + screenRows - 1;
            if (cy > static_cast<int>(rows.size())) {
                cy = rows.size();
            }
        }
        int times = screenRows;
        while (times--) {
            moveCursor(c == static_cast<int>(Key::PAGE_UP) ? 
                      static_cast<int>(Key::ARROW_UP) : 
                      static_cast<int>(Key::ARROW_DOWN));
        }
    } else if (c == static_cast<int>(Key::ARROW_UP) || 
               c == static_cast<int>(Key::ARROW_DOWN) ||
               c == static_cast<int>(Key::ARROW_LEFT) || 
               c == static_cast<int>(Key::ARROW_RIGHT)) {
        moveCursor(c);
    } else if (c == Terminal::ctrlKey('l') || c == static_cast<int>(Key::ESCAPE)) {
        // Refresh / Escape - do nothing
    } else if (c >= 0 && c < 128 && !std::iscntrl(c)) {
        insertChar(c);
    }
    
    quitTimes = QUIT_TIMES;
}

void Editor::moveCursor(int key) {
    Row* row = (cy >= static_cast<int>(rows.size())) ? nullptr : &rows[cy];
    
    if (key == static_cast<int>(Key::ARROW_LEFT)) {
        if (cx != 0) {
            cx--;
        } else if (cy > 0) {
            cy--;
            cx = rows[cy].getSize();
        }
    } else if (key == static_cast<int>(Key::ARROW_RIGHT)) {
        if (row && cx < row->getSize()) {
            cx++;
        } else if (row && cx == row->getSize()) {
            cy++;
            cx = 0;
        }
    } else if (key == static_cast<int>(Key::ARROW_UP)) {
        if (cy != 0) {
            cy--;
        }
    } else if (key == static_cast<int>(Key::ARROW_DOWN)) {
        if (cy < static_cast<int>(rows.size())) {
            cy++;
        }
    }
    
    // Snap cursor to end of line if past it
    row = (cy >= static_cast<int>(rows.size())) ? nullptr : &rows[cy];
    int rowlen = row ? row->getSize() : 0;
    if (cx > rowlen) {
        cx = rowlen;
    }
}

void Editor::insertChar(char c) {
    if (cy == static_cast<int>(rows.size())) {
        rows.emplace_back(rows.size(), "");
    }
    rows[cy].insertChar(cx, c);
    rows[cy].update(syntax, rows);
    updateSyntax(cy);
    cx++;
    dirty = true;
}

void Editor::insertNewline() {
    if (cx == 0) {
        rows.insert(rows.begin() + cy, Row(cy, ""));
    } else {
        Row& row = rows[cy];
        std::string rest = row.getChars().substr(cx);
        row.deleteChar(cx);  // This will be handled differently
        
        // Truncate current row
        std::string currentChars = row.getChars();
        rows[cy] = Row(cy, currentChars.substr(0, cx));
        rows.insert(rows.begin() + cy + 1, Row(cy + 1, rest));
    }
    
    // Update indices
    for (size_t i = cy; i < rows.size(); i++) {
        rows[i].setIndex(i);
        rows[i].update(syntax, rows);
    }
    
    cy++;
    cx = 0;
    dirty = true;
    updateSyntax(cy - 1);
}

void Editor::deleteChar() {
    if (cy == static_cast<int>(rows.size())) return;
    if (cx == 0 && cy == 0) return;
    
    Row& row = rows[cy];
    if (cx > 0) {
        row.deleteChar(cx - 1);
        row.update(syntax, rows);
        updateSyntax(cy);
        cx--;
    } else {
        cx = rows[cy - 1].getSize();
        rows[cy - 1].appendString(row.getChars());
        rows.erase(rows.begin() + cy);
        
        // Update indices
        for (size_t i = cy - 1; i < rows.size(); i++) {
            rows[i].setIndex(i);
            rows[i].update(syntax, rows);
        }
        
        cy--;
        updateSyntax(cy);
    }
    
    dirty = true;
}

void Editor::save() {
    if (filename.empty()) {
        filename = prompt("Save as: %s (ESC to cancel)");
        if (filename.empty()) {
            setStatusMessage("Save aborted");
            return;
        }
        selectSyntaxHighlight();
    }
    
    if (FileIO::saveFile(filename, rows)) {
        dirty = false;
        setStatusMessage(std::to_string(FileIO::rowsToString(rows).length()) + " bytes written to disk");
    } else {
        setStatusMessage("Can't save! I/O error");
    }
}

void Editor::find() {
    int saved_cx = cx;
    int saved_cy = cy;
    int saved_coloff = colOffset;
    int saved_rowoff = rowOffset;
    
    auto callback = [this](const std::string& query, int key) {
        this->findCallback(query, key);
    };
    
    std::string query = prompt("Search: %s (Use ESC/Arrows/Enter)", callback);
    
    if (query.empty()) {
        cx = saved_cx;
        cy = saved_cy;
        colOffset = saved_coloff;
        rowOffset = saved_rowoff;
    }
}

void Editor::findCallback(const std::string& query, int key) {
    static int lastMatch = -1;
    static int direction = 1;
    
    if (key == '\r' || key == static_cast<int>(Key::ESCAPE)) {
        lastMatch = -1;
        direction = 1;
        return;
    } else if (key == static_cast<int>(Key::ARROW_RIGHT) || 
               key == static_cast<int>(Key::ARROW_DOWN)) {
        direction = 1;
    } else if (key == static_cast<int>(Key::ARROW_LEFT) || 
               key == static_cast<int>(Key::ARROW_UP)) {
        direction = -1;
    } else {
        lastMatch = -1;
        direction = 1;
    }
    
    if (lastMatch == -1) direction = 1;
    int current = lastMatch;
    
    for (int i = 0; i < static_cast<int>(rows.size()); i++) {
        current += direction;
        if (current == -1) current = rows.size() - 1;
        else if (current == static_cast<int>(rows.size())) current = 0;
        
        const std::string& render = rows[current].getRender();
        size_t match = render.find(query);
        if (match != std::string::npos) {
            lastMatch = current;
            cy = current;
            cx = rows[current].rxToCx(match);
            rowOffset = rows.size();
            break;
        }
    }
}

std::string Editor::prompt(const std::string& promptText, 
                           std::function<void(const std::string&, int)> callback) {
    std::string buf;
    
    while (true) {
        setStatusMessage(Helpers::format(promptText.c_str(), buf.c_str()));
        refreshScreen();
        
        int c = terminal->readKey();
        if (c == -1) continue;
        
        if (c == static_cast<int>(Key::DELETE_KEY) || 
            c == Terminal::ctrlKey('h') || 
            c == static_cast<int>(Key::BACKSPACE)) {
            if (!buf.empty()) {
                buf.pop_back();
            }
        } else if (c == static_cast<int>(Key::ESCAPE)) {
            setStatusMessage("");
            if (callback) callback(buf, c);
            return "";
        } else if (c == '\r') {
            if (!buf.empty()) {
                setStatusMessage("");
                if (callback) callback(buf, c);
                return buf;
            }
        } else if (c >= 0 && c < 128 && !std::iscntrl(c)) {
            buf += static_cast<char>(c);
        }
        
        if (callback) callback(buf, c);
    }
}

void Editor::setStatusMessage(const std::string& msg) {
    statusMsg = msg;
    statusMsgTime = std::time(nullptr);
}

void Editor::selectSyntaxHighlight() {
    syntax = nullptr;
    if (!filename.empty()) {
        syntax = syntaxHighlighter.getSyntaxForFilename(filename);
    }
}

void Editor::updateSyntax(int startRow) {
    if (startRow < 0 || startRow >= static_cast<int>(rows.size())) return;
    
    for (size_t i = startRow; i < rows.size(); i++) {
        bool prevOpenComment = (i > 0) ? rows[i - 1].hasOpenComment() : false;
        rows[i].update(syntax, rows);
        bool currentOpenComment = rows[i].hasOpenComment();
        
        // If comment state didn't change, we can stop
        if (i > static_cast<size_t>(startRow) && prevOpenComment == currentOpenComment) {
            break;
        }
    }
}

