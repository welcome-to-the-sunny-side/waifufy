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

### Core DP Layout Engine
- **Dynamic Programming**: `convert_layout()` uses a 3D DP table with states `(i, j, k)` representing:
  - `i`: current column position (0 to W_BOUND-1)
  - `j`: number of tokens consumed (0 to tokens_left_total)
  - `k`: separator requirement state (0=space, 1=comment, 2=needs_sep, 3=no_sep)
- **Scoring**: Each character placement scores based on target density matching (0 for match, penalty for mismatch)
- **Transitions**: Three types of DP transitions:
  - **Space placement**: moves to next column with space character
  - **Comment generation**: places `/* ... */` blocks of length 4-20 with random content
  - **Token placement**: places actual code tokens with separator enforcement

### Advanced Features
- **Scarcity-aware budgeting**: Monitors token usage rate across rows and caps per-row token consumption when projection indicates early token exhaustion
- **Randomized tie-breaking**: When DP transitions have equal scores, randomly updates backpointers to encourage layout diversity
- **Relaxed selection**: Uses configurable relaxation factor (`SCORE_RELAXATION = W/10`) to prefer higher token counts within acceptable score range
- **Overflow handling**: Rows beyond image height use per-line random width caps (`W + rand[0, 10)`) for natural line length variation

### Safety and Constraints
- **Mandatory separators**: `needs_separator()` enforces whitespace rules to prevent token merging, operator formation, and comment hazards
- **Comment sanitization**: Generated block comments avoid `*/` sequences in content
- **Token preservation**: All original non-comment tokens preserved in exact order with original text

### Key Parameters
- `MN_TOKENS = 4`: Minimum preferred tokens per row
- `SCORE_RELAXATION_FACTOR = 10`: Relaxation divisor for token count preference
- `MX_COMMENT_LENGTH = 20`: Maximum generated comment block length
- `shoot = 10`: Width variance for overflow rows

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

## Recent Enhancements

### Layout Algorithm Improvements
- **Full DP implementation**: Complete 3D dynamic programming solution with state transitions for optimal token/comment/space placement
- **Scarcity management**: Automatic token budgeting prevents token starvation in later rows by monitoring usage rates
- **Diversity injection**: Randomized tie-breaking in DP transitions creates varied outputs across runs
- **Smart selection**: Relaxed scoring allows higher token density within acceptable quality bounds

### Bug Fixes

- **Fixed Accidental Single-Line Comment Formation**: Resolved a critical bug where a single `/` token followed by a generated `/* ... */` comment would form `//`, inadvertently commenting out the rest of the line. The tokenizer (`tokenize_minimal_cpp`) was updated to append a space to any single `/` token, converting it to `/ `. This prevents the issue while preserving the semantics of the division operator.

### Helper Tools
- **Image conversion**: `helper/img2grid.py` converts images to ASCII art grids with configurable width and aspect ratio
- **Grid generation**: Supports various input formats and customizable density mapping

## Limitations and next steps
- Visual similarity uses binary (0/1) density matching; grayscale density mapping could improve fidelity
- Current comment generation uses random characters; smarter content generation could enhance appearance
- No Tree-sitter integration yet; relying on custom minimal tokenizer
- Inter-token spacing is currently minimal; more sophisticated spacing could improve readability

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
