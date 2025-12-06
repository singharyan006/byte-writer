#ifndef SYNTAX_H
#define SYNTAX_H

#include <string>
#include <vector>

// Highlighting types
enum class Highlight {
    NORMAL = 0,
    COMMENT,
    MLCOMMENT,      // Multi-line comment
    KEYWORD1,       // Keywords like if, while, for
    KEYWORD2,       // Type keywords like int, void, char
    STRING,
    NUMBER,
    MATCH           // Search match highlighting
};

// Syntax highlighting flags
#define HL_HIGHLIGHT_NUMBERS (1 << 0)
#define HL_HIGHLIGHT_STRINGS (1 << 1)

// Syntax definition for a language
struct EditorSyntax {
    std::string filetype;                    // Display name
    std::vector<std::string> filematch;      // File extensions (.c, .cpp, etc.)
    std::vector<std::string> keywords;       // Keywords (type keywords end with '|')
    std::string singlelineCommentStart;      // e.g., "//"
    std::string multilineCommentStart;       // e.g., "/*"
    std::string multilineCommentEnd;         // e.g., "*/"
    int flags;                               // HL_HIGHLIGHT_* flags
};

class SyntaxHighlighter {
public:
    SyntaxHighlighter();
    
    // Get syntax definition by file extension
    const EditorSyntax* getSyntaxForFilename(const std::string& filename);
    
    // Convert highlight type to ANSI color code
    static int syntaxToColor(Highlight hl);
    
private:
    std::vector<EditorSyntax> database;
    
    void initDatabase();
};

#endif // SYNTAX_H

