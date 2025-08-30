#pragma once
#include <vector>
#include <optional>
#include <string>
#include <string_view>

namespace waifufy {

using Token = std::string;

const int min_width = 80;

struct Art {
    int W = 0;
    int H = 0;
    std::vector<std::vector<double>> density; // H x W
};

struct AsciiDensity {
    std::vector<double> v; // size >= 128 for ASCII
};

// Simple 0/1 density: space -> 0.0, everything else -> 1.0
AsciiDensity default_ascii_density_01();

// Parse ASCII art into density grid. If width_override/height_override are not set, infer from art.
Art parse_art_to_density(
    std::string_view art_text,
    std::optional<int> width_override = std::nullopt,
    std::optional<int> height_override = std::nullopt,
    const AsciiDensity* map = nullptr
);

// Read a whole file as UTF-8-ish text (no strict validation)
std::string read_file(const std::string& path);

// Remove // and /* */ comments while preserving strings and char literals (including raw strings)
std::string strip_comments_preserve_literals(std::string_view code);

// Minimal C++ tokenizer: strings, chars, identifiers, numbers, punctuators. Whitespace is skipped.
std::vector<Token> tokenize_minimal_cpp(std::string_view code_no_comments);

// Whether a and b must be separated by at least one whitespace to avoid token merging or forming comments/operators.
bool needs_separator(std::string_view a, std::string_view b);

// Minimal safe whitespace separator between tokens (visually light); returns " ".
std::string minimal_separator();

// Join tokens inserting minimal separators only when needed by needs_separator().
std::string join_tokens_min_sep(const std::vector<Token>& toks);

// Placeholder conversion: you implement this. For now returns a minimal-joined token stream.
// W,H are the target width/height; target_density is HxW per-cell desired density.
std::string convert_layout(const std::vector<Token>& tokens,
                           int W,
                           int H,
                           const std::vector<std::vector<double>>& target_density,
                           const AsciiDensity& density_map);

} // namespace waifufy
