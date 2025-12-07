#include "Row.h"
#include "../utils/Helpers.h"
#include <algorithm>

Row::Row(int idx) : index(idx), hlOpenComment(false) {
}

Row::Row(int idx, const std::string& text) 
    : index(idx), chars(text), hlOpenComment(false) {
}

void Row::insertChar(int at, char c) {
    if (at < 0 || at > static_cast<int>(chars.size())) {
        at = chars.size();
    }
    chars.insert(at, 1, c);
}

void Row::deleteChar(int at) {
    if (at < 0 || at >= static_cast<int>(chars.size())) {
        return;
    }
    chars.erase(at, 1);
}

void Row::appendString(const std::string& s) {
    chars += s;
}

void Row::update(const EditorSyntax* syntax, const std::vector<Row>& allRows) {
    updateRender();
    updateSyntax(syntax, allRows);
}

void Row::updateRender() {
    // Count tabs to allocate proper size
    int tabs = std::count(chars.begin(), chars.end(), '\t');
    
    render.clear();
    render.reserve(chars.size() + tabs * (TAB_STOP - 1));
    
    int col = 0;
    for (char c : chars) {
        if (c == '\t') {
            // Add spaces until next tab stop
            render += ' ';
            col++;
            while (col % TAB_STOP != 0) {
                render += ' ';
                col++;
            }
        } else {
            render += c;
            col++;
        }
    }
}

void Row::updateSyntax(const EditorSyntax* syntax, const std::vector<Row>& allRows) {
    hl.clear();
    hl.resize(render.size(), Highlight::NORMAL);
    
    if (syntax == nullptr) return;
    
    const auto& keywords = syntax->keywords;
    const std::string& scs = syntax->singlelineCommentStart;
    const std::string& mcs = syntax->multilineCommentStart;
    const std::string& mce = syntax->multilineCommentEnd;
    
    int scs_len = scs.length();
    int mcs_len = mcs.length();
    int mce_len = mce.length();
    
    bool prev_sep = true;
    int in_string = 0;  // 0 = not in string, '"' or '\'' = in string
    bool in_comment = (index > 0 && allRows[index - 1].hasOpenComment());
    
    size_t i = 0;
    while (i < render.size()) {
        char c = render[i];
        Highlight prev_hl = (i > 0) ? hl[i - 1] : Highlight::NORMAL;
        
        // Single-line comments
        if (scs_len && !in_string && !in_comment) {
            if (render.substr(i, scs_len) == scs) {
                std::fill(hl.begin() + i, hl.end(), Highlight::COMMENT);
                break;
            }
        }
        
        // Multi-line comments
        if (mcs_len && mce_len && !in_string) {
            if (in_comment) {
                hl[i] = Highlight::MLCOMMENT;
                if (render.substr(i, mce_len) == mce) {
                    std::fill(hl.begin() + i, hl.begin() + i + mce_len, Highlight::MLCOMMENT);
                    i += mce_len;
                    in_comment = false;
                    prev_sep = true;
                    continue;
                } else {
                    i++;
                    continue;
                }
            } else if (render.substr(i, mcs_len) == mcs) {
                std::fill(hl.begin() + i, hl.begin() + i + mcs_len, Highlight::MLCOMMENT);
                i += mcs_len;
                in_comment = true;
                continue;
            }
        }
        
        // Strings
        if (syntax->flags & HL_HIGHLIGHT_STRINGS) {
            if (in_string) {
                hl[i] = Highlight::STRING;
                if (c == '\\' && i + 1 < render.size()) {
                    hl[i + 1] = Highlight::STRING;
                    i += 2;
                    continue;
                }
                if (c == in_string) in_string = 0;
                i++;
                prev_sep = true;
                continue;
            } else {
                if (c == '"' || c == '\'') {
                    in_string = c;
                    hl[i] = Highlight::STRING;
                    i++;
                    continue;
                }
            }
        }
        
        // Numbers
        if (syntax->flags & HL_HIGHLIGHT_NUMBERS) {
            if ((std::isdigit(c) && (prev_sep || prev_hl == Highlight::NUMBER)) ||
                (c == '.' && prev_hl == Highlight::NUMBER)) {
                hl[i] = Highlight::NUMBER;
                i++;
                prev_sep = false;
                continue;
            }
        }
        
        // Keywords
        if (prev_sep) {
            bool found = false;
            for (const auto& keyword : keywords) {
                int klen = keyword.length();
                bool is_type = (keyword.back() == '|');
                if (is_type) klen--;
                
                if (render.substr(i, klen) == keyword.substr(0, klen) &&
                    (i + klen >= render.size() || Helpers::isSeparator(render[i + klen]))) {
                    std::fill(hl.begin() + i, hl.begin() + i + klen,
                             is_type ? Highlight::KEYWORD2 : Highlight::KEYWORD1);
                    i += klen;
                    found = true;
                    break;
                }
            }
            if (found) {
                prev_sep = false;
                continue;
            }
        }
        
        prev_sep = Helpers::isSeparator(c);
        i++;
    }
    
    bool changed = (hlOpenComment != in_comment);
    hlOpenComment = in_comment;
    if (changed && index + 1 < static_cast<int>(allRows.size())) {
        // Need to re-highlight next row
        // This will be handled by the Editor class
    }
}

int Row::cxToRx(int cx) const {
    int rx = 0;
    for (int i = 0; i < cx && i < static_cast<int>(chars.size()); i++) {
        if (chars[i] == '\t') {
            rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        }
        rx++;
    }
    return rx;
}

int Row::rxToCx(int rx) const {
    int cur_rx = 0;
    int cx;
    for (cx = 0; cx < static_cast<int>(chars.size()); cx++) {
        if (chars[cx] == '\t') {
            cur_rx += (TAB_STOP - 1) - (cur_rx % TAB_STOP);
        }
        cur_rx++;
        
        if (cur_rx > rx) return cx;
    }
    return cx;
}

