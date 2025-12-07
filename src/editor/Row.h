#ifndef ROW_H
#define ROW_H

#include <string>
#include <vector>
#include "Syntax.h"

// Represents one row (line) of text in the editor
class Row {
public:
    Row(int idx = 0);
    Row(int idx, const std::string& text);
    
    // Accessors
    int getIndex() const { return index; }
    int getSize() const { return chars.size(); }
    int getRenderSize() const { return render.size(); }
    const std::string& getChars() const { return chars; }
    const std::string& getRender() const { return render; }
    const std::vector<Highlight>& getHighlight() const { return hl; }
    bool hasOpenComment() const { return hlOpenComment; }
    
    // Mutators
    void setIndex(int idx) { index = idx; }
    void setOpenComment(bool open) { hlOpenComment = open; }
    
    // Operations
    void insertChar(int at, char c);
    void deleteChar(int at);
    void appendString(const std::string& s);
    
    // Rendering
    void update(const EditorSyntax* syntax, const std::vector<Row>& allRows);
    int cxToRx(int cx) const;  // Convert cursor x to render x (handle tabs)
    int rxToCx(int rx) const;  // Convert render x to cursor x
    
private:
    int index;                          // Row index in file
    std::string chars;                  // Actual characters
    std::string render;                 // Rendered characters (tabs expanded)
    std::vector<Highlight> hl;          // Highlight info for each render char
    bool hlOpenComment;                 // Is there an unclosed multi-line comment?
    
    void updateRender();
    void updateSyntax(const EditorSyntax* syntax, const std::vector<Row>& allRows);
    
    static constexpr int TAB_STOP = 8;
};

#endif // ROW_H

