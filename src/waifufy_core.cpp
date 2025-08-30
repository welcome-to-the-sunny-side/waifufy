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
#include <iostream>
#include <random>
#include<array>
#include <cstdint>

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

    // Split by lines first
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
    // If the file ends with a newline, drop the final empty line so H matches visual rows
    if (!lines.empty() && lines.back().empty() && !height_override && !width_override && !art_text.empty() && art_text.back() == '\n') {
        lines.pop_back();
    }

    // UTF-8 -> code points for each line
    auto utf8_to_u32 = [](std::string_view s) -> std::u32string {
        std::u32string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size();) {
            unsigned char c0 = static_cast<unsigned char>(s[i]);
            if ((c0 & 0x80) == 0) {
                out.push_back(static_cast<char32_t>(c0));
                ++i;
            } else if ((c0 & 0xE0) == 0xC0 && i + 1 < s.size()) { // 2-byte
                unsigned char c1 = static_cast<unsigned char>(s[i + 1]);
                char32_t cp = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
                out.push_back(cp);
                i += 2;
            } else if ((c0 & 0xF0) == 0xE0 && i + 2 < s.size()) { // 3-byte
                unsigned char c1 = static_cast<unsigned char>(s[i + 1]);
                unsigned char c2 = static_cast<unsigned char>(s[i + 2]);
                char32_t cp = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
                out.push_back(cp);
                i += 3;
            } else if ((c0 & 0xF8) == 0xF0 && i + 3 < s.size()) { // 4-byte
                unsigned char c1 = static_cast<unsigned char>(s[i + 1]);
                unsigned char c2 = static_cast<unsigned char>(s[i + 2]);
                unsigned char c3 = static_cast<unsigned char>(s[i + 3]);
                char32_t cp = ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
                out.push_back(cp);
                i += 4;
            } else {
                // invalid byte, skip
                ++i;
            }
        }
        return out;
    };

    std::vector<std::u32string> u32lines;
    u32lines.reserve(lines.size());
    for (const auto& s : lines) u32lines.push_back(utf8_to_u32(s));

    Art A;
    int H = static_cast<int>(u32lines.size());
    if (height_override) {
        H = *height_override;
        if (static_cast<int>(u32lines.size()) > H) {
            u32lines.resize(H);
        } else if (static_cast<int>(u32lines.size()) < H) {
            u32lines.resize(H);
        }
    }
    int W = 0;
    for (auto& s : u32lines) {
        if ((int)s.size() > W) W = (int)s.size();
    }
    if (!width_override && u32lines.empty()) W = 80;
    if (width_override) W = *width_override;

    A.W = W;
    A.H = H;
    A.density.assign(H, std::vector<double>(W, 0.0));
    for (int i = 0; i < H; ++i) {
        const std::u32string& row = (i < (int)u32lines.size() ? u32lines[i] : std::u32string());
        for (int j = 0; j < W; ++j) {
            char32_t cp = (j < (int)row.size() ? row[j] : U' ');
            double d = 1.0;
            if (cp <= 127 && static_cast<size_t>(cp) < local_map.v.size()) d = local_map.v[static_cast<size_t>(cp)];
            else d = 1.0; // non-ASCII treated as filled
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

    const int shoot = 10;
    const int MN_TOKENS = 4;
    const int SCORE_RELAXATION_FACTOR = 10;
    const int SCORE_RELAXATION = W/SCORE_RELAXATION_FACTOR;
    const int MX_COMMENT_LENGTH = 20;
    const int W_BOUND = W + shoot;  //final width has to be strictly less than this
    
    int maxl = 0;
    for(auto t : tokens)
        maxl = std::max(maxl, int(t.size()));
    assert(std::max(min_width, maxl) < W_BOUND);

    int n = tokens.size();

    auto is_one = [&](double d) -> int
    {
        return d >= 0.5;
    };

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 25);
    std::uniform_int_distribution<int> coin(0, 1);
    auto rand_character = [&]() -> char 
    {
        return char('a' + dist(rng));
    };
    // Precompute char -> density(0/1) for ASCII range
    std::array<unsigned char, 128> char01{};
    for (int c = 0; c < 128; ++c) {
        double d = (static_cast<size_t>(c) < density_map.v.size() ? density_map.v[static_cast<size_t>(c)] : 1.0);
        char01[c] = static_cast<unsigned char>(is_one(d));
    }
    
    std::string waifu = "";
    int taken = 0, row = 0;

    // Precompute entire art density to 0/1 once
    std::vector<std::vector<unsigned char>> dens01;
    dens01.assign(H, std::vector<unsigned char>(W, 0));
    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            dens01[r][c] = static_cast<unsigned char>(is_one(target_density[r][c]));
        }
    }
    
    while(taken < n or row < H)
    {
        // std::cerr << "row: " << row << std::endl;
        // After finishing H rows (the art), compactly pack remaining tokens per line.
        if (row >= H) {
            // Overflow: pack greedily up to W + rand[0, shoot)
            std::uniform_int_distribution<int> w_extra_dist(0, shoot - 1);
            const int W_eff = W + w_extra_dist(rng);
            std::string line;
            int col = 0;
            std::string_view prev;
            while (taken < n) {
                const std::string &tok = tokens[taken];
                int sep = (!prev.empty() && needs_separator(prev, tok)) ? 1 : 0;
                int L = sep + (int)tok.size();
                if (col + L > W_eff) {
                    // If nothing placed yet, place this token anyway to avoid empty lines
                    if (col == 0) {
                        if (sep) { line.push_back(' '); ++col; }
                        line += tok; col += (int)tok.size();
                        prev = tok; ++taken;
                    }
                    break; // flush line
                }
                if (sep) { line.push_back(' '); ++col; }
                line += tok; col += (int)tok.size();
                prev = tok; ++taken;
            }
            waifu += line + "\n";
            ++row;
            continue;
        }
        /*
            we have four types of blobs we can place:
            1. spaces
            2. comments
            3. token that requires separation from the next
            4. token that doesn't require separation from the next

            define dp[position][number of tokens we used on this line][type of blob that ends at i] = maximum score we can get

            score = number of positions at which we match density

            as for the transitions:
            - placing a space is trivial
            - comments are multiline comments (/*) we can place spaces and characters in the contents of the comment (either place a space or a random character using rand_character)
            - the one restriction for state transitions are that we cannot go directly from blob type 3 -> blob type 4 or blob type 3 -> blob type 3, we must place either a comment or a space after them
            - this corresponds to replacing something like `int a` with either `int a` or `int<comment>a` (both of which are valid)

            oh fuck i'll have to store back-pointers for reconstruction, fml

            now letting this run as is would obviously just make it use comments everywhere
            potential remedies:
            1. track the number of comments used and limit them
            2. track the length of the row covered by comments and limit it
            3. require the usage of a minimum number of tokens on every line

            lets just go with #3 for now, as it doesnt require the introduction of an extra state
            this doesn't affect dp calculations, only the selection of the final optimal string
        */
        
        //solve the dp
        // Use heap-based buffers to avoid stack overflow for large W
        const int STATES = W_BOUND * W_BOUND * 4;
        auto IDX = [&](int i, int j, int k) -> int { return (i * W_BOUND + j) * 4 + k; };
        std::vector<int> dp(STATES, 0);
        std::vector<std::array<int,3>> back_point(STATES);

        // state type mapping:
        // 0 -> space
        // 1 -> comment (/* ... */)
        // 2 -> token (requires separation from the next token)
        // 3 -> token (does not require separation from the next token)

        const int NEG_INF = -1e9;

        // Precompute desired density for this row (0/1), padded to W_BOUND with zeros
        std::vector<unsigned char> want(W_BOUND, 0);
        if (row < H) {
            for (int col = 0; col < std::min(W, W_BOUND); ++col) {
                want[col] = dens01[row][col];
            }
        }

        // helpers
        auto desired_at = [&](int col) -> int {
            if (col >= 0 && col < W_BOUND) return (int)want[col];
            return 0; // outside treated as 0-density target
        };
        auto density_of = [&](char ch) -> int {
            unsigned char uch = static_cast<unsigned char>(ch);
            return (uch < 128) ? (int)char01[uch] : 1;
        };
        auto score_char = [&](int col, char ch) -> int {
            int want = desired_at(col);
            int got = density_of(ch);
            return (want == got) ? 1 : 0;
        };
        auto token_score = [&](int start_col, const std::string& tok) -> int {
            int s = 0;
            for (int t = 0; t < (int)tok.size(); ++t) s += score_char(start_col + t, tok[t]);
            return s;
        };
        auto comment_score = [&](int start_col, int len) -> int {
            // Layout: '/' '*' [content ...] '*' '/'
            // Interiors always match by choice of space/non-space: contribute (len-4) if len>=4
            int s = (len >= 4) ? (len - 4) : 0;
            s += score_char(start_col + 0, '/');
            s += score_char(start_col + 1, '*');
            s += score_char(start_col + len - 2, '*');
            s += score_char(start_col + len - 1, '/');
            return s;
        };

        // init DP and back pointers
        for (int i = 0; i < W_BOUND; ++i) {
            for (int j = 0; j < W_BOUND; ++j) {
                for (int k = 0; k < 4; ++k) {
                    dp[IDX(i,j,k)] = NEG_INF;
                    back_point[IDX(i,j,k)] = {-1, -1, -1};
                }
            }
        }
        dp[IDX(0,0,0)] = 0; // start with an imaginary space state

        int tokens_left_total = std::max(0, n - taken);
        int maxJThisRow = std::min(W_BOUND - 1, tokens_left_total);
        std::vector<unsigned char> need_sep(maxJThisRow + 1, 0);
        for (int j = 0; j <= maxJThisRow; ++j) {
            if (taken + j + 1 < n) need_sep[j] = (unsigned char)needs_separator(tokens[taken + j], tokens[taken + j + 1]);
        }

        // transitions
        for (int i = 0; i < W_BOUND; ++i) {
            for (int j = 0; j <= std::min(i, tokens_left_total); ++j) {
                for (int k = 0; k < 4; ++k) {
                    int cur = dp[IDX(i,j,k)];
                    if (cur == NEG_INF) continue;

                    // type 1 transition: place a space (consumes 1 column)
                    if (i + 1 < W_BOUND) {
                        int add = score_char(i, ' ');
                        int &ref = dp[IDX(i + 1, j, 0)];
                        int cand = cur + add;
                        if (cand > ref) {
                            ref = cand;
                            back_point[IDX(i + 1, j, 0)] = {i, j, k};
                        } else if (cand == ref && ref != NEG_INF) {
                            if (coin(rng)) back_point[IDX(i + 1, j, 0)] = {i, j, k};
                        }
                    }

                    // type 2 transition: place a comment of length L in [4, MX_COMMENT_LENGTH]
                    int Lmax = std::min(MX_COMMENT_LENGTH, W_BOUND - i - 1);
                    for (int L = 4; L <= Lmax; ++L) {
                        int add = comment_score(i, L);
                        int &ref = dp[IDX(i + L, j, 1)];
                        int cand = cur + add;
                        if (cand > ref) {
                            ref = cand;
                            back_point[IDX(i + L, j, 1)] = {i, j, k};
                        } else if (cand == ref && ref != NEG_INF) {
                            if (coin(rng)) back_point[IDX(i + L, j, 1)] = {i, j, k};
                        }
                    }

                    // type 3/4 transitions: place a token (if allowed)
                    if (j < tokens_left_total) {
                        // Enforce restriction: cannot transition from type 2 (requires sep) directly to another token
                        if (k != 2) {
                            const std::string &tok = tokens[taken + j];
                            int L = (int)tok.size();
                            if (i + L < W_BOUND) {
                                int add = token_score(i, tok);
                                bool req_sep = (j <= maxJThisRow) ? (need_sep[j] != 0) : false;
                                int nk = req_sep ? 2 : 3;
                                int &ref = dp[IDX(i + L, j + 1, nk)];
                                int cand = cur + add;
                                if (cand > ref) {
                                    ref = cand;
                                    back_point[IDX(i + L, j + 1, nk)] = {i, j, k};
                                } else if (cand == ref && ref != NEG_INF) {
                                    if (coin(rng)) back_point[IDX(i + L, j + 1, nk)] = {i, j, k};
                                }
                            }
                        }
                    }
                }
            }
        }


        // choose the optimal string with relaxed selection by token count
        std::array<int, 3> optimal_state = {0, 0, 0};
        const int jHi = std::min(tokens_left_total, W_BOUND - 1);
        const int iStart = std::max(0, W - shoot);

        bool selected = false;
        for (int minTok = std::min(MN_TOKENS, tokens_left_total); minTok >= 0 && !selected; --minTok) {
            // 1) Check existence of any valid state with j >= minTok
            bool any = false;
            for (int i = iStart; i < W_BOUND && !any; ++i) {
                for (int j = std::max(0, minTok); j <= jHi && !any; ++j) {
                    for (int k = 0; k < 4; ++k) {
                        if (dp[IDX(i, j, k)] != NEG_INF) { any = true; break; }
                    }
                }
            }
            if (!any) continue; // try smaller minimum tokens

            // 2) Find optimal value among states with j >= minTok
            int bestVal = NEG_INF; std::array<int,3> bestState = {0,0,0};
            for (int i = iStart; i < W_BOUND; ++i) {
                for (int j = std::max(0, minTok); j <= jHi; ++j) {
                    for (int k = 0; k < 4; ++k) {
                        int val = dp[IDX(i, j, k)];
                        if (val > bestVal) { bestVal = val; bestState = {i, j, k}; }
                    }
                }
            }

            // 3) Among states with j >= minTok and score >= bestVal - SCORE_RELAXATION,
            //    choose the one with maximum j. If multiple, prefer higher score.
            const int threshold = bestVal - SCORE_RELAXATION;
            std::array<int,3> chosen = bestState; // guaranteed to satisfy threshold
            bool found = false;
            for (int j = jHi; j >= std::max(0, minTok) && !found; --j) {
                int bestAtJ = NEG_INF; std::array<int,3> cand = {0,0,0};
                for (int i = iStart; i < W_BOUND; ++i) {
                    for (int k = 0; k < 4; ++k) {
                        int val = dp[IDX(i, j, k)];
                        if (val >= threshold && val > bestAtJ) { bestAtJ = val; cand = {i, j, k}; }
                    }
                }
                if (bestAtJ != NEG_INF) { chosen = cand; found = true; }
            }
            optimal_state = chosen;
            selected = true;
        }
        // Safety fallback (should not trigger because minTok=0 always has a state)
        if (!selected) {
            int bestVal = NEG_INF; std::array<int,3> bestState = {0,0,0};
            for (int i = iStart; i < W_BOUND; ++i) {
                for (int j = 0; j <= jHi; ++j) {
                    for (int k = 0; k < 4; ++k) {
                        int val = dp[IDX(i, j, k)];
                        if (val > bestVal) { bestVal = val; bestState = {i, j, k}; }
                    }
                }
            }
            optimal_state = bestState;
        }

        //reconstruct the optimal string using optimal_state and back_point
        std::string this_row = "";
        {
            int ci = optimal_state[0];
            int cj = optimal_state[1];
            int ck = optimal_state[2];

            // build segments backwards
            std::string built;
            while (ci > 0 || cj > 0) {
                std::array<int, 3> prev = back_point[IDX(ci, cj, ck)];
                if (prev[0] < 0) break; // no path
                int pi = prev[0], pj = prev[1], pk = prev[2];
                int len = ci - pi;

                std::string seg;
                if (ck == 0) {
                    // space
                    seg.push_back(' ');
                } else if (ck == 1) {
                    // comment
                    // layout '/' '*' [content] '*' '/'
                    if (len < 4) len = 4; // safety
                    for (int t = 0; t < len; ++t) {
                        int col = pi + t;
                        if (t == 0) seg.push_back('/');
                        else if (t == 1) seg.push_back('*');
                        else if (t == len - 2) seg.push_back('*');
                        else if (t == len - 1) seg.push_back('/');
                        else {
                            int wantbit = (row < H && col < W) ? dens01[row][col] : 0;
                            if (wantbit == 0) seg.push_back(' ');
                            else seg.push_back(rand_character());
                        }
                    }
                } else {
                    // token (ck == 2 or 3)
                    if (pj >= 0 && pj < W_BOUND && (taken + pj) < n) {
                        const std::string &tok = tokens[taken + pj];
                        seg = tok;
                    }
                }

                // prepend segment
                this_row = seg + this_row;

                ci = pi; cj = pj; ck = pk;
            }
        }

        // advance global token counter by number of tokens used on this row
        taken += optimal_state[1];

        // std::cerr << "this_row: " << this_row << std::endl;
        // std::cerr << this_row.length() << " " << W_BOUND << std::endl;

        waifu += this_row + "\n";

        ++ row;
    }

    return waifu;
}
}
