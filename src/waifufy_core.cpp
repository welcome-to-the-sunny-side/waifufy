#include "waifufy_core.hpp"

#include <string>
#include <algorithm>
#include <assert.h>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <unordered_set>

namespace waifufy {

AsciiDensity default_ascii_density_01() {
    AsciiDensity m;
    m.v.assign(128, 1.0);
    m.v[static_cast<unsigned char>(' ')] = 0.0;
    return m;
}

static bool is_ident_start(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

static bool is_ident_char(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::string read_file(const std::string& path) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) return {};
    std::string data;
    ifs.seekg(0, std::ios::end);
    auto sz = ifs.tellg();
    if (sz > 0) data.reserve(static_cast<size_t>(sz));
    ifs.seekg(0, std::ios::beg);
    data.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return data;
}

Art parse_art_to_density(std::string_view art_text,
                         std::optional<int> width_override,
                         std::optional<int> height_override,
                         const AsciiDensity* map) {
    AsciiDensity local_map = map ? *map : default_ascii_density_01();

    // Split lines
    std::vector<std::string> lines;
    size_t start = 0;
    while (start <= art_text.size()) {
        size_t pos = art_text.find('\n', start);
        if (pos == std::string_view::npos) {
            lines.emplace_back(art_text.substr(start));
            break;
        } else {
            lines.emplace_back(art_text.substr(start, pos - start));
            start = pos + 1;
        }
    }
    if (!lines.empty() && lines.back().empty() && art_text.size() && art_text.back() == '\n') {
        // trailing newline produced an extra empty line which is fine
    }

    Art A;
    int H = static_cast<int>(lines.size());
    if (height_override) {
        H = *height_override;
        if (static_cast<int>(lines.size()) > H) {
            lines.resize(H);
        } else if (static_cast<int>(lines.size()) < H) {
            lines.resize(H);
        }
    }
    int W = 0;
    for (auto& s : lines) {
        if ((int)s.size() > W) W = (int)s.size();
    }
    if (!width_override && lines.empty()) W = 80;
    if (width_override) W = *width_override;

    A.W = W;
    A.H = H;
    A.density.assign(H, std::vector<double>(W, 0.0));
    for (int i = 0; i < H; ++i) {
        const std::string& row = (i < (int)lines.size() ? lines[i] : std::string());
        for (int j = 0; j < W; ++j) {
            unsigned char ch = (j < (int)row.size() ? (unsigned char)row[j] : (unsigned char)' ');
            double d = 1.0;
            if (static_cast<size_t>(ch) < local_map.v.size()) d = local_map.v[static_cast<size_t>(ch)];
            else d = 1.0;
            A.density[i][j] = d;
        }
    }
    return A;
}

std::string strip_comments_preserve_literals(std::string_view code) {
    enum State { Normal, InBlock, InLine, InStr, InChar, InRaw };
    State st = Normal;
    std::string out;
    out.reserve(code.size());

    // Raw string delimiter when in raw string
    std::string raw_delim;

    bool esc = false;
    for (size_t i = 0; i < code.size();) {
        char c = code[i];
        char n = (i + 1 < code.size() ? code[i + 1] : '\0');
        switch (st) {
            case Normal: {
                // Detect raw string start: R"delim( ... )delim"
                if (c == 'R' && n == '"') {
                    // parse delim up to '(' after R"
                    size_t j = i + 2;
                    raw_delim.clear();
                    while (j < code.size() && code[j] != '(') {
                        char d = code[j];
                        if (d == ')' || d == '\\' || std::isspace((unsigned char)d) || raw_delim.size() > 16) break;
                        raw_delim.push_back(d);
                        ++j;
                    }
                    if (j < code.size() && code[j] == '(') {
                        // Emit as-is until we find )delim"
                        st = InRaw;
                        out.append("R\"");
                        out.append(raw_delim);
                        out.push_back('(');
                        i = j + 1;
                        break;
                    }
                }
                // Prefixed raw strings: u8R" .., uR" .., UR" .., LR" ..
                if ((c == 'u' && n == '8' && i + 2 < code.size() && code[i + 2] == 'R' && i + 3 < code.size() && code[i + 3] == '"') ||
                    (c == 'u' && n == 'R' && i + 2 < code.size() && code[i + 2] == '"') ||
                    (c == 'U' && n == 'R' && i + 2 < code.size() && code[i + 2] == '"') ||
                    (c == 'L' && n == 'R' && i + 2 < code.size() && code[i + 2] == '"')) {
                    size_t j = (c == 'u' && n == '8') ? i + 4 : i + 3;
                    raw_delim.clear();
                    while (j < code.size() && code[j] != '(') {
                        char d = code[j];
                        if (d == ')' || d == '\\' || std::isspace((unsigned char)d) || raw_delim.size() > 16) break;
                        raw_delim.push_back(d);
                        ++j;
                    }
                    if (j < code.size() && code[j] == '(') {
                        st = InRaw;
                        out.append(std::string(code.substr(i, (c=='u'&&n=='8')?4:3)));
                        out.append(raw_delim);
                        out.push_back('(');
                        i = j + 1;
                        break;
                    }
                }
                if (c == '/' && n == '*') { st = InBlock; i += 2; break; }
                if (c == '/' && n == '/') { st = InLine;  i += 2; break; }
                if (c == '"') { st = InStr; out.push_back(c); ++i; esc = false; break; }
                if (c == '\'') { st = InChar; out.push_back(c); ++i; esc = false; break; }
                out.push_back(c); ++i; break;
            }
            case InBlock: {
                if (c == '*' && n == '/') { st = Normal; i += 2; }
                else { ++i; }
                break;
            }
            case InLine: {
                if (c == '\n') { st = Normal; out.push_back(c); }
                ++i; break;
            }
            case InStr: {
                out.push_back(c);
                if (esc) { esc = false; }
                else if (c == '\\') { esc = true; }
                else if (c == '"') { st = Normal; }
                ++i; break;
            }
            case InChar: {
                out.push_back(c);
                if (esc) { esc = false; }
                else if (c == '\\') { esc = true; }
                else if (c == '\'') { st = Normal; }
                ++i; break;
            }
            case InRaw: {
                // look for )delim"
                if (c == ')' && !raw_delim.empty()) {
                    size_t need = raw_delim.size();
                    if (i + 1 + need < code.size() && std::string_view(code).substr(i + 1, need) == std::string_view(raw_delim) && i + 1 + need < code.size() && code[i + 1 + need] == '"') {
                        out.push_back(')');
                        out.append(raw_delim);
                        out.push_back('"');
                        i += 2 + need;
                        st = Normal;
                        break;
                    }
                }
                out.push_back(c);
                ++i; break;
            }
        }
    }
    return out;
}

static bool starts_with(std::string_view s, std::string_view p) {
    return s.size() >= p.size() && s.substr(0, p.size()) == p;
}

static bool parse_raw_string(std::string_view s, size_t i, size_t& j) {
    // s[i] must be 'R' and s[i+1] == '"'
    if (!(i + 1 < s.size() && s[i] == 'R' && s[i + 1] == '"')) return false;
    size_t k = i + 2;
    std::string delim;
    while (k < s.size() && s[k] != '(') {
        char d = s[k];
        if (d == ')' || d == '\\' || std::isspace((unsigned char)d) || delim.size() > 16) return false;
        delim.push_back(d);
        ++k;
    }
    if (k >= s.size() || s[k] != '(') return false;
    // find )delim"
    size_t pos = k + 1;
    while (pos < s.size()) {
        if (s[pos] == ')' && pos + 1 + delim.size() < s.size() && std::string_view(s).substr(pos + 1, delim.size()) == std::string_view(delim) && pos + 1 + delim.size() < s.size() && s[pos + 1 + delim.size()] == '"') {
            j = pos + 2 + delim.size();
            return true;
        }
        ++pos;
    }
    j = s.size();
    return true; // treat as till end
}

static bool parse_prefixed_raw(std::string_view s, size_t i, size_t& j) {
    auto test = [&](std::string_view pfx, size_t off) -> bool {
        if (starts_with(s.substr(i), pfx) && i + off < s.size() && s[i + off] == 'R' && i + off + 1 < s.size() && s[i + off + 1] == '"') {
            size_t jj;
            if (parse_raw_string(s, i + off, jj)) { j = jj; return true; }
        }
        return false;
    };
    if (test("u8", 2)) return true; // u8R".."
    if (test("u", 1)) return true;  // uR".."
    if (test("U", 1)) return true;  // UR".."
    if (test("L", 1)) return true;  // LR".."
    return false;
}

std::string minimal_separator() { return " "; }

std::string join_tokens_min_sep(const std::vector<Token>& toks) {
    if (toks.empty()) return {};
    std::string out;
    out.reserve(1024);
    std::string_view prev;
    for (size_t i = 0; i < toks.size(); ++i) {
        std::string_view cur{toks[i]};
        if (!prev.empty() && needs_separator(prev, cur)) out.append(minimal_separator());
        out.append(cur);
        prev = cur;
    }
    return out;
}

std::vector<Token> tokenize_minimal_cpp(std::string_view code) {
    std::vector<Token> toks;
    size_t i = 0, n = code.size();
    auto push = [&](size_t b, size_t e) { toks.emplace_back(std::string(code.substr(b, e - b))); };

    const std::vector<std::string> PUNCTS = {
        ">>=", "<<=", "->*", "::", "->", "++", "--", "<<", ">>", "&&", "||",
        "==", "!=", "<=", ">=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
        "##"
    };

    auto match_from = [&](size_t pos, const std::vector<std::string>& arr) -> size_t {
        for (auto& p : arr) {
            if (pos + p.size() <= n && std::string_view(code).substr(pos, p.size()) == p) return pos + p.size();
        }
        return 0;
    };

    while (i < n) {
        // whitespace
        if (std::isspace(static_cast<unsigned char>(code[i]))) { ++i; continue; }

        // raw string
        size_t j;
        if (parse_prefixed_raw(code, i, j)) { push(i, j); i = j; continue; }
        if (parse_raw_string(code, i, j)) { if (j > i) { push(i, j); i = j; continue; } }

        // string (with optional prefixes u8, u, U, L)
        if (code[i] == '"' || (code[i] == 'u' && i + 1 < n && (code[i + 1] == '8' || code[i + 1] == '"')) || (code[i] == 'U' && i + 1 < n && code[i + 1] == '"') || (code[i] == 'L' && i + 1 < n && code[i + 1] == '"')) {
            size_t b = i;
            // move i to the opening quote
            if (code[i] == 'u' && i + 1 < n && code[i + 1] == '8') i += 2;
            if ((code[i] == 'u' || code[i] == 'U' || code[i] == 'L') && i + 1 < n && code[i + 1] == '"') ++i;
            if (code[i] == '"') {
                ++i;
                bool esc2 = false;
                while (i < n) {
                    char ch = code[i++];
                    if (esc2) { esc2 = false; continue; }
                    if (ch == '\\') { esc2 = true; continue; }
                    if (ch == '"') break;
                }
                push(b, i);
                continue;
            } else {
                i = b; // fallback
            }
        }

        // char literal (optional prefix u/U/L)
        if (code[i] == '\'' || ((code[i] == 'u' || code[i] == 'U' || code[i] == 'L') && i + 1 < n && code[i + 1] == '\'')) {
            size_t b = i;
            if ((code[i] == 'u' || code[i] == 'U' || code[i] == 'L') && i + 1 < n && code[i + 1] == '\'') ++i;
            if (code[i] == '\'') {
                ++i;
                bool esc2 = false;
                while (i < n) {
                    char ch = code[i++];
                    if (esc2) { esc2 = false; continue; }
                    if (ch == '\\') { esc2 = true; continue; }
                    if (ch == '\'') break;
                }
                push(b, i);
                continue;
            } else {
                i = b;
            }
        }

        // identifier
        if (is_ident_start(code[i])) {
            size_t b = i++;
            while (i < n && is_ident_char(code[i])) ++i;
            push(b, i);
            continue;
        }

        // number (very permissive)
        if (std::isdigit(static_cast<unsigned char>(code[i]))) {
            size_t b = i++;
            while (i < n && (std::isalnum(static_cast<unsigned char>(code[i])) || code[i] == '.' || code[i] == '_' || code[i] == '\'')) ++i;
            push(b, i);
            continue;
        }

        // multi-char punctuators first
        size_t m = match_from(i, PUNCTS);
        if (m) { push(i, m); i = m; continue; }

        // single char
        push(i, i + 1);
        ++i;
    }
    return toks;
}

bool needs_separator(std::string_view a, std::string_view b) {
    if (a.empty() || b.empty()) return false;
    char ca = a.back();
    char cb = b.front();

    auto is_alnum_ = [](char c){ return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };

    // 1) Merge of identifiers/numbers
    if (is_alnum_(ca) && is_alnum_(cb)) return true;

    // 2) Comment hazards and block close hazards
    if ((ca == '/' && cb == '/') || (ca == '/' && cb == '*') || (ca == '*' && cb == '/')) return true;

    // 3) Multi-char punctuators formed across boundary
    const std::unordered_set<std::string> multi = {
        ">>=", "<<=", "->*", "::", "->", "++", "--", "<<", ">>", "&&", "||",
        "==", "!=", "<=", ">=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
        "##", "..."
    };
    std::string x;
    x.reserve(3);
    // last 2 of a + first 1 of b
    if (a.size() >= 2) {
        x.assign(a.substr(a.size() - 2));
        x.push_back(cb);
        if (multi.count(x)) return true;
    }
    // last 1 of a + first 1 or 2 of b
    x.clear();
    x.push_back(ca);
    x.push_back(cb);
    if (multi.count(x)) return true;
    if (b.size() >= 2) {
        x.push_back(b[1]);
        if (multi.count(x)) return true;
    }
    // explicit check for ellipsis across 3 tokens like '.' '.' '.'
    if (ca == '.' && b.size() >= 2 && b[0] == '.' && b[1] == '.') return true;
    if (a.size() >= 2 && a[a.size()-2] == '.' && a[a.size()-1] == '.' && cb == '.') return true;

    // 4) Literal + UDL hazards
    auto ends_with_quote = [&](std::string_view s){ return !s.empty() && (s.back() == '"' || s.back() == '\''); };
    if ((ends_with_quote(a) || std::isdigit(static_cast<unsigned char>(ca))) && (std::isalpha(static_cast<unsigned char>(cb)) || cb == '_')) return true;

    // 5) Floating-literal adjacency across boundary
    if ((ca == '.' && std::isdigit(static_cast<unsigned char>(cb))) || (std::isdigit(static_cast<unsigned char>(ca)) && cb == '.')) return true;

    return false;
}

std::string convert_layout(const std::vector<Token>& tokens,
                           int W,
                           int H,
                           const std::vector<std::vector<double>>& target_density,
                           const AsciiDensity& density_map) {
    // Basic, safe layout:
    // - Preserve token order
    // - Insert spaces when needs_separator() says so
    // - Optionally add extra spaces to align with zero-density cells
    // - Fill end-of-line remainder with at most one comment block to approximate 1-density targets
    // Assumptions: no preprocessor directives/macros per user note.

    assert(W >= min_width);

    auto is_one = [&](int r, int c) -> bool {
        if (r < 0 || r >= H || c < 0 || c >= W) return false;
        return target_density[r][c] > 0.5;
    };

    auto pick_dense_char = [&]() -> char {
        unsigned char cand = (unsigned char)'.';
        if ((size_t)cand < density_map.v.size() && density_map.v[cand] > 0.5) return '.';
        cand = (unsigned char)'*';
        if ((size_t)cand < density_map.v.size() && density_map.v[cand] > 0.5) return '*';
        cand = (unsigned char)'#';
        if ((size_t)cand < density_map.v.size() && density_map.v[cand] > 0.5) return '#';
        return 'X';
    };
    const char dense_ch = pick_dense_char();

    std::string out;
    out.reserve(static_cast<size_t>(std::max(1, H)) * static_cast<size_t>(W + 1) + tokens.size() * 2);

    std::string line;
    line.reserve(static_cast<size_t>(W) + 64);
    int row = 0;
    int col = 0;
    std::string prev;

    auto fill_remainder = [&](int r, int c_start, std::string& dst) {
        if (c_start >= W) return;
        int width0 = W - c_start;
        if (r < 0 || r >= H) {
            dst.append(width0, ' ');
            return;
        }
        int ones = 0;
        for (int j = c_start; j < W; ++j) if (is_one(r, j)) ++ones;

        // Heuristic: only use a comment if there are enough 1-cells to justify the overhead of /* */
        const int overhead = 4; // '/*' + '*/'
        const int threshold = 8; // keep comments sparse

        if (width0 >= overhead + 1 && ones >= threshold) {
            // Leading space for safety at boundary
            dst.push_back(' ');
            int rem = width0 - 1;
            if (rem >= overhead) {
                dst.append("/*");
                rem -= 2;
                int content_len = rem - 2; // reserve for closing */
                for (int k = 0; k < content_len; ++k) {
                    int j = c_start + 1 + 2 + k; // +1 leading space, +2 for /*
                    char ch = (is_one(r, j) ? dense_ch : ' ');
                    // Avoid placing '*' immediately before the closing '/' to not form '*/' prematurely.
                    if (k == content_len - 1 && ch == '*') ch = (dense_ch == '*' ? '.' : dense_ch);
                    dst.push_back(ch);
                }
                dst.append("*/");
                return;
            }
            // if not enough space after adding the leading space, fall through to spaces
            // (rare due to the guard)
        }

        // Fallback: spaces only
        dst.append(width0, ' ');
    };

    for (size_t i = 0; i < tokens.size(); ) {
        const std::string& tok = tokens[i];
        int tlen = static_cast<int>(tok.size());

        // Required minimal separator
        int sep = (!prev.empty() && needs_separator(prev, tok)) ? 1 : 0;

        // Adaptive extra spaces to match zero-density cells before token
        int extra = 0;
        if (row < H) {
            const int cap = 6; // don't overdo spaces
            while (col + sep + extra < W && extra < cap && !is_one(row, col + sep + extra)) {
                ++extra;
            }
        }

        // Wrap if token doesn't fit
        if (col + sep + extra + tlen > W) {
            fill_remainder(row, col, line);
            out.append(line);
            out.push_back('\n');
            line.clear();
            col = 0;
            ++row;
            prev.clear();
            continue; // reprocess current token on next line
        }

        // Emit spaces and token
        if (sep + extra > 0) { line.append(static_cast<size_t>(sep + extra), ' '); col += sep + extra; }
        line.append(tok); col += tlen;
        prev = tok;

        // Auto-wrap if we hit end of line
        if (col >= W) {
            out.append(line);
            out.push_back('\n');
            line.clear();
            col = 0;
            ++row;
            prev.clear();
        }
        ++i;
    }

    // Flush the last partially filled line
    if (col > 0 || row < H) {
        fill_remainder(row, col, line);
        out.append(line);
        out.push_back('\n');
        line.clear();
        ++row;
    }

    // Pad remaining rows up to H
    while (row < H) {
        fill_remainder(row, 0, line);
        out.append(line);
        out.push_back('\n');
        line.clear();
        ++row;
    }

    return out;
}
}