# Waifufy: C++ → ASCII Art

A C++ tool that formats C++ programs to visually approximate ASCII art while preserving program semantics.

What you get:
- H, W and art density grid via `waifufy::parse_art_to_density()`.
- ASCII char→density map (0/1 for now) via `waifufy::default_ascii_density_01()` (vector-backed; size ≥ 128).
- Comment stripping and minimal C++ tokenizer via `strip_comments_preserve_literals()` and `tokenize_minimal_cpp()`.
- Safe token join helpers: `needs_separator()`, `minimal_separator()`, `join_tokens_min_sep()`.
- A placeholder `convert_layout()` you can implement.
- CLI `waifufy_cli` to wire the above to files.

## Build

```sh
cmake -S . -B build
cmake --build build -j
```

## Run

```sh
./build/waifufy \
  --code input_code.cpp \
  --art input_art.txt \
  --out output_code.cpp \
  --width 80 --height 40 \
  --dump-meta
```

`--dump-meta` prints `W`, `H`, and token count to stderr.

## Implement your layout
Edit `convert_layout()` in `src/waifufy_core.cpp`.

Signature:

```cpp
std::string convert_layout(
    const std::vector<Token>& tokens,
    int W,
    int H,
    const std::vector<std::vector<double>>& target_density,
    const AsciiDensity& density_map);
```

You receive:
- `tokens`: vector<string> of code tokens (comments removed, strings/chars preserved)
- `W`, `H`: target output width and height in columns/rows
- `target_density`: HxW grid from the ASCII art
- `density_map`: ASCII density lookup (vector-backed; `v[' '] == 0.0`)

You can build output by distributing tokens and adding filler between them. Use `needs_separator(a, b)` to decide if a whitespace is mandatory between adjacent tokens to preserve semantics.

If you need different char-density mapping, replace `default_ascii_density_01()` or add your own.

## Whitespace safety rules
Two tokens `a` then `b` need at least one space if any of the following holds:
- Alnum/underscore merge: last of `a` and first of `b` are `[A-Za-z0-9_]` → would merge into an identifier/number.
- Comment/close hazards: `a.endswith('/') and b.startswith('/')` (forms `//`), `a.endswith('/') and b.startswith('*')` (forms `/*`), or `a.endswith('*') and b.startswith('/')` (forms `*/`).
- Multi-char operators formed across boundary: `>>=, <<=, ->*, ::, ->, ++, --, <<, >>, &&, ||, ==, !=, <=, >=, +=, -=, *=, /=, %=, &=, |=, ^=, ##, ...`.
- Literal + identifier (UDL): string/char/number followed immediately by identifier or `_`.
- Floating literal adjacency: `.` next to digit across boundary or digit next to `.`.

Call `needs_separator(a, b)` to enforce these.
