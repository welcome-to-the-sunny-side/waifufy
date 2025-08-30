# waifufy: C++ → ASCII art (semantics preserved)

This document describes the design, constraints, and usage of the waifufy tool in this repository. The goal: format an input C++ program so that, when displayed in a monospaced font, it visually approximates a target ASCII art image while preserving program semantics.

**This is a pure C++ implementation with no Python dependencies.**

## Inputs and outputs
- Input code: `input_code.cpp` (or any path via CLI `--code`)
- Target art: `input_art.txt` (rectangular; width W = line length, height H = number of lines)
- Output code: a transformed C++ file that should be semantically identical to the original, written via CLI `--out`.
- Optional: heatmap (same W×H) marking matches (.) and mismatches (^) for the image region.

## Invariants and allowances
- Semantics preserved: token order and text of non-comment tokens are kept. Only whitespace and comments are inserted/removed.
- Comments: original comments may be removed entirely. We generate new `/* ... */` block comments as “filler” to sculpt the image. We sanitize comment contents to avoid accidental `*/` sequences.
- No includes/macros: your inputs contain no `#include` directives and no macros. Preprocessing is effectively used only to strip comments. The pipeline tolerates macros if present in other inputs; expansion would be acceptable but is irrelevant here.
- Strings/chars: may appear in inputs; we don’t modify them and we don’t insert inside them.
- Font: assume a monospaced font (e.g., DejaVu Sans Mono). We optimize per cell, not kerning.
- Overflow/underflow:
  - If code is “short”: we fill remaining image cells/rows with comment-only art to match the target.
  - If code is “long”: remaining tokens appear as lines below the image; each line is padded to the same width with a solid filler.
- Compilation note: when compiling any file, manually ensure `#include <bits/stdc++.h>` is present (either by editing a copy or by compiling with an injected header). The tool itself does not inject includes.

## Architecture (C++ modules)
- `src/waifufy_core.cpp` / `include/waifufy_core.hpp`
  - **Comment stripping**: `strip_comments_preserve_literals()` removes `//` and `/* */` comments while preserving string/char literals (including raw strings)
  - **Tokenization**: `tokenize_minimal_cpp()` performs minimal C++ tokenization preserving strings/chars, numbers, identifiers, and common punctuators without splitting multi-char operators
  - **Art parsing**: `parse_art_to_density()` converts ASCII art text into W×H density grid with configurable character-to-density mapping
  - **Layout engine**: `convert_layout()` implements greedy per-row layout with adaptive spacing and comment-based filler generation
  - **Safety helpers**: `needs_separator()` enforces mandatory whitespace rules to prevent token merging and operator formation
- `src/main.cpp`
  - CLI front-end that wires together: file I/O → comment stripping → tokenization → art parsing → layout → output writing

## Visual model
- Current cost: per-character equality. Cost 0 if characters match, 1 if not.
- Prefix shift: we try all possible left-padding widths for each line to best align code span with the target.
- Future improvement (not yet implemented): incorporate a per-character density table to better approximate grayscale ramps.

## Algorithm details
- Mandatory separators: between adjacent tokens we may need a single space to avoid merging identifiers/numbers (e.g., `intx` vs `int x`). The layout respects this. In the current baseline, between-token filler is not added; only prefix and suffix fillers per line are used.
- Sanitization: generated block comments are sanitized to never include `*/` in content.
- Safety against very long tokens: if a single token exceeds the image width, we emit a solid-only line and then print that token on a dedicated line below to avoid stalling the layout. Such cases are rare unless width is tiny or raw strings are very long.

## Build and Usage

### Build
```bash
cmake -S . -B build
cmake --build build -j
```

### Usage
```bash
./build/waifufy \
  --code input_code.cpp \
  --art input_art.txt \
  --out output_code.cpp \
  --dump-meta
```

- **Width/height**: Automatically inferred from `input_art.txt`. Override with `--width N`/`--height N` if needed.
- **Debug info**: `--dump-meta` prints W, H, and token count to stderr.

## Semantics and compilation
- The transformed program preserves token order and contents (strings, numbers, identifiers, and operators) and only inserts/removes whitespace/comments.
- To compile the output, you may need to ensure `#include <bits/stdc++.h>` is present:

```bash
# create a buildable copy with the include injected at the top
(echo '#include <bits/stdc++.h>'; cat output_code.cpp) > build.cpp

# compile
g++ -std=c++17 -O2 build.cpp -o program
```

## Limitations and next steps
- Visual similarity uses strict equality only; density-aware matching is marked as a future enhancement.
- The current layout inserts only a left prefix and right suffix filler per image row. Extending to inter-token filler blocks would significantly improve fidelity.
- Very large preprocessed code (due to includes) is not applicable for your inputs (no includes). The preprocessor path is kept for general robustness.
- No Tree-sitter integration yet; relying on system `g++`/`clang++` when available.

## Repository structure
- `src/` - C++ source files
- `include/` - C++ header files  
- `build/` - CMake build directory
- Sample inputs: `input_code.cpp`, `input_art.txt`
- Outputs written where you specify via `--out`

## Requirements
- C++17 compatible compiler
- CMake 3.16+
- No external dependencies
