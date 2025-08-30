# img2grid

Convert an image into a **binary text grid** using two characters — by default `'0'` for ON pixels and a space `' '` for OFF pixels.

- Output is an **H × W** grid (W characters per line, H lines), saved to a `.txt` file.
- Emphasis on **high contrast**, **sharp boundaries**, and options to create **large solid regions** (via morphology).
- Fully configurable characters and processing pipeline. A **whitespace-only** mode is included.

> Heads-up: Some editors visually trim trailing spaces; your `.txt` will contain them even if you don’t see them.

---

## Install

```bash
python -m venv .venv
source .venv/bin/activate  # Windows: .venv\Scripts\activate
pip install -r requirements.txt
```

## Usage

```bash
python img2grid.py INPUT_IMAGE [-o OUTPUT_TXT]
                           [--width W | --height H | --scale S]
                           [--y-aspect A]
                           [--clahe --clahe-clip C --clahe-grid G]
                           [--blur K]
                           [--method {otsu,adaptive_mean,adaptive_gaussian,canny}]
                           [--adaptive-block B --adaptive-c C2]
                           [--canny-lo L --canny-hi H2]
                           [--morph {none,open,close,erode,dilate}]
                           [--morph-ksize K --morph-iters I]
                           [--invert]
                           [--on-char CH --off-char CH]
                           [--whitespace-only]
                           [--save-preview PREVIEW.png]
```

### Minimal example

```bash
python img2grid.py photo.png --width 120 -o photo.txt
```

This resizes to 120 columns, compensates height using `--y-aspect` (default `0.5`), does Otsu thresholding, and writes `photo.txt`.

### Whitespace-only output

If you want **only whitespace characters** in the grid:

```bash
python img2grid.py logo.png --width 100 --whitespace-only -o logo_ws.txt
```

In this mode, ON pixels use **tab** (`\t`) and OFF pixels use a **space**. Tabs and spaces are both whitespace, so the text *looks blank*, but the structure is there. Use `--save-preview` to sanity-check the binary image:

```bash
python img2grid.py logo.png --width 100 --whitespace-only \
    --save-preview logo_preview.png -o logo_ws.txt
```

### Custom characters

You can set custom **ON/OFF** characters. Escape sequences are supported.

```bash
# ON = '0', OFF = space (defaults)
python img2grid.py input.jpg --width 120 -o out.txt

# ON = '#', OFF = '.'
python img2grid.py input.jpg --width 120 --on-char "#" --off-char "." -o out.txt

# ON = em-space (U+2003), OFF = regular space (still whitespace-only)
python img2grid.py input.jpg --width 120 --on-char "\u2003" --off-char " " -o out.txt
```

> Note: Each of `--on-char` and `--off-char` must resolve to **one** character after escapes, to guarantee exact column counts.

---

## Tunable Knobs (Quality & Contrast)

These options let you steer **contrast, sharpness, and solidity** (fewer speckles, larger uniform regions).

### Size & Aspect
- `--width W` or `--height H` or `--scale S`: Choose target size. The program maintains aspect ratio.
- `--y-aspect A` (default `0.5`): Compensates for character cell shape (glyphs are typically taller than wide). If your output looks vertically squashed or stretched, tweak this. Increase `A` for **taller** output, decrease for **shorter**.

### Contrast & Denoising
- `--clahe`: Enables **CLAHE** (contrast-limited adaptive histogram equalization). Helps in low-contrast images.
  - `--clahe-clip C` (default `2.0`): Higher increases local contrast.
  - `--clahe-grid G` (default `8`): Tile size (bigger = broader context).
- `--blur K`: Gaussian blur kernel (odd, e.g., 3, 5). Small blur can remove noise before thresholding.

### Binarization (how we make it black/white)
- `--method otsu` (default): Global threshold using **Otsu**. Good first choice.
- `--method adaptive_mean` or `--method adaptive_gaussian`: Adapts threshold locally, useful for uneven lighting or textured inputs.
  - `--adaptive-block B` (odd, default `31`): Local window size. Larger `B` = smoother, larger areas.
  - `--adaptive-c C2` (default `5`): Fine-tune threshold (subtracts from local mean/gaussian).
- `--method canny`: Keeps **edges** (sharp boundaries). Produces line-art style outputs.
  - `--canny-lo L` / `--canny-hi H2`: Lower/upper thresholds for edges.

### Morphology (shape cleanup)
- `--morph open`: Erode then dilate — **removes small speckles**; great for large uniform areas.
- `--morph close`: Dilate then erode — **fills small holes** inside shapes.
- `--morph erode` / `--morph dilate`: Raw controls if you want to thin/thicken strokes.
  - `--morph-ksize K` (odd, default `3`): Kernel size.
  - `--morph-iters I` (default `1`): Strength.

### Polarity
- `--invert`: Swap ON/OFF after binarization. Handy when the foreground/background comes out flipped.

---

## Output Details

- The program writes a **plain text** file with exactly **W characters** per line and **H lines**, using your chosen ON/OFF characters.
- Newlines (`\n`) separate rows.
- The `--save-preview` option writes the **binary image** (post-processing) as a PNG to help verify what the text encodes.

---

## Examples

**High-contrast logo with large solid regions:**
```bash
python img2grid.py logo.png --width 120 --method adaptive_gaussian \
  --adaptive-block 41 --adaptive-c 5 --morph close --morph-ksize 5 --morph-iters 2 \
  -o logo_grid.txt --save-preview logo_grid_preview.png
```

**Edge-focused drawing:**
```bash
python img2grid.py sketch.jpg --width 140 --method canny --canny-lo 80 --canny-hi 200 \
  -o sketch_edges.txt --save-preview sketch_edges.png
```

**Whitespace-only matrix (tabs vs spaces):**
```bash
python img2grid.py qr.png --width 100 --whitespace-only \
  -o qr_ws.txt --save-preview qr_ws_preview.png
```

---

## Notes & Tips

- Trailing spaces are part of the data; some editors hide them. Use a hex viewer or turn on “show invisibles” if needed.
- For terminals that compress tabs, the **whitespace-only** output is best inspected in a monospaced editor where tabs are width=1, or prefer `--on-char "\u2003"` (em-space) with `--off-char " "` for pure whitespace that preserves columns in many editors.
- If the image looks vertically squashed or stretched, tune `--y-aspect`. Typical values: 0.45–0.6.
- If you want *exactly* the “0 / space” binary look the prompt described, keep the defaults (`--on-char "0" --off-char " "`).

---

## License

MIT
