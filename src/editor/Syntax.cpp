#include "Syntax.h"
#include <algorithm>

SyntaxHighlighter::SyntaxHighlighter() {
    initDatabase();
}

void SyntaxHighlighter::initDatabase() {
    // C/C++ syntax
    EditorSyntax cppSyntax;
    cppSyntax.filetype = "c/c++";
    cppSyntax.filematch = {".c", ".h", ".cpp", ".hpp", ".cc", ".cxx"};
    cppSyntax.keywords = {
        // Control flow keywords
        "switch", "if", "while", "for", "break", "continue", "return", "else",
        "struct", "union", "typedef", "static", "enum", "class", "case",
        "const", "sizeof", "volatile", "auto", "register", "goto", "do",
        "namespace", "using", "template", "typename", "try", "catch", "throw",
        "public", "private", "protected", "virtual", "override", "final",
        "explicit", "inline", "extern", "friend", "operator", "new", "delete",
        "this", "nullptr", "true", "false",
        
        // Type keywords (ending with |)
        "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
        "void|", "bool|", "short|", "size_t|", "uint8_t|", "uint16_t|", 
        "uint32_t|", "uint64_t|", "int8_t|", "int16_t|", "int32_t|", "int64_t|",
        "std::string|", "std::vector|", "std::map|", "string|", "vector|", "map|"
    };
    cppSyntax.singlelineCommentStart = "//";
    cppSyntax.multilineCommentStart = "/*";
    cppSyntax.multilineCommentEnd = "*/";
    cppSyntax.flags = HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS;
    
    database.push_back(cppSyntax);
    
    // Python syntax
    EditorSyntax pythonSyntax;
    pythonSyntax.filetype = "python";
    pythonSyntax.filematch = {".py", ".pyw"};
    pythonSyntax.keywords = {
        "and", "as", "assert", "break", "class", "continue", "def", "del",
        "elif", "else", "except", "finally", "for", "from", "global", "if",
        "import", "in", "is", "lambda", "nonlocal", "not", "or", "pass",
        "raise", "return", "try", "while", "with", "yield", "async", "await",
        
        // Type keywords
        "int|", "float|", "str|", "bool|", "list|", "dict|", "tuple|", "set|",
        "None|", "True|", "False|"
    };
    pythonSyntax.singlelineCommentStart = "#";
    pythonSyntax.multilineCommentStart = "\"\"\"";
    pythonSyntax.multilineCommentEnd = "\"\"\"";
    pythonSyntax.flags = HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS;
    
    database.push_back(pythonSyntax);
}

const EditorSyntax* SyntaxHighlighter::getSyntaxForFilename(const std::string& filename) {
    if (filename.empty()) return nullptr;
    
    // Find file extension
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) return nullptr;
    
    std::string ext = filename.substr(dotPos);
    
    // Search database for matching syntax
    for (const auto& syntax : database) {
        for (const auto& pattern : syntax.filematch) {
            if (pattern[0] == '.') {
                // Extension match
                if (ext == pattern) {
                    return &syntax;
                }
            } else {
                // Filename substring match
                if (filename.find(pattern) != std::string::npos) {
                    return &syntax;
                }
            }
        }
    }
    
    return nullptr;
}

int SyntaxHighlighter::syntaxToColor(Highlight hl) {
    switch (hl) {
        case Highlight::COMMENT:
        case Highlight::MLCOMMENT:
            return 36;  // Cyan
        case Highlight::KEYWORD1:
            return 33;  // Yellow
        case Highlight::KEYWORD2:
            return 32;  // Green
        case Highlight::STRING:
            return 35;  // Magenta
        case Highlight::NUMBER:
            return 31;  // Red
        case Highlight::MATCH:
            return 34;  // Blue
        default:
            return 37;  // White
    }
}

