#!/usr/bin/env python3
"""
img2grid.py — Convert an image to a binary text grid using two characters (default: "0" for ON, space for OFF).

Output is H*W characters per line (W columns), with newline-separated rows (H rows).
Designed for high-contrast, sharp boundaries, and tunable morphological smoothing.

Dependencies: numpy, opencv-python
"""

import argparse
import sys
from pathlib import Path
import numpy as np

try:
    import cv2
except ImportError as e:
    print("Missing dependency: opencv-python. Install via: pip install opencv-python", file=sys.stderr)
    raise

# -----------------------------
# Helpers
# -----------------------------
def parse_char(s: str) -> str:
    """
    Parse a CLI-supplied character, supporting common escape sequences like '\\t', '\\n', '\\u2003'.
    Must resolve to exactly ONE Unicode codepoint; otherwise we reject (to preserve W exactly).
    """
    if s is None:
        return ' '
    # Allow user-friendly aliases
    aliases = {
        'space': ' ',
        'tab': '\t',
        'nbspace': '\xa0',  # non-breaking space
        'emspace': '\u2003',
        'enspace': '\u2002',
        'thinspace': '\u2009',
        'figspace': '\u2007',
        'mspace': '\u2003',
        'zwnbsp': '\ufeff',  # zero width no-break space (be careful)
    }
    if s.lower() in aliases:
        ch = aliases[s.lower()]
    else:
        # Interpret Python-style escapes
        ch = bytes(s, 'utf-8').decode('unicode_escape')
    # Enforce length 1 Unicode scalar; many fonts treat some spaces visually identical.
    if len(ch) != 1:
        raise argparse.ArgumentTypeError(
            f"--on-char/--off-char must be ONE character after escapes. Got {repr(ch)} from {repr(s)}."
        )
    return ch

def oddize(n: int, minimum: int = 3) -> int:
    """Ensure n is odd and >= minimum."""
    n = int(round(n))
    if n < minimum:
        n = minimum
    if n % 2 == 0:
        n += 1
    return n

def compute_target_size(h, w, width=None, height=None, scale=None, y_aspect=0.5):
    """
    Compute (target_h, target_w) considering character cell aspect ratio.
    y_aspect < 1 compresses height (typical for monospace displays where glyphs are taller).
    """
    if width is None and height is None and scale is None and abs(y_aspect - 1.0) < 1e-9:
        return h, w  # no resize

    if width is not None and height is not None:
        # Trust user; still apply y_aspect by post-scaling height so shapes look right
        th = int(round(height * y_aspect))
        tw = int(round(width))
        th = max(1, th)
        tw = max(1, tw)
        return th, tw

    if width is not None:
        ratio = width / w
        th = int(round(h * ratio * y_aspect))
        tw = int(round(width))
        th = max(1, th)
        tw = max(1, tw)
        return th, tw

    if height is not None:
        # Invert the earlier formula: width ≈ height * (w/h) / y_aspect
        tw = int(round(height * (w / h) / max(y_aspect, 1e-9)))
        th = int(round(height * y_aspect))
        th = max(1, th)
        tw = max(1, tw)
        return th, tw

    if scale is not None:
        th = int(round(h * scale * y_aspect))
        tw = int(round(w * scale))
        th = max(1, th)
        tw = max(1, tw)
        return th, tw

    return h, w

def apply_contrast_enhancement(gray, clahe_clip=2.0, clahe_grid=8):
    """Contrast enhancement via CLAHE."""
    clahe = cv2.createCLAHE(clipLimit=float(clahe_clip), tileGridSize=(int(clahe_grid), int(clahe_grid)))
    return clahe.apply(gray)

def binarize(gray, method='otsu', adaptive_block=31, adaptive_c=5, canny_lo=100, canny_hi=200):
    method = method.lower()
    if method == 'otsu':
        _t, bw = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
        return bw
    if method == 'adaptive_mean':
        bs = oddize(adaptive_block)
        return cv2.adaptiveThreshold(gray, 255, cv2.ADAPTIVE_THRESH_MEAN_C,
                                     cv2.THRESH_BINARY, bs, int(adaptive_c))
    if method == 'adaptive_gaussian':
        bs = oddize(adaptive_block)
        return cv2.adaptiveThreshold(gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
                                     cv2.THRESH_BINARY, bs, int(adaptive_c))
    if method == 'canny':
        edges = cv2.Canny(gray, canny_lo, canny_hi)
        # Convert edges (0/255) already; ensure 0/255 binary like others
        return edges
    raise ValueError(f"Unknown method: {method}")

def apply_morph(bw, morph='none', ksize=3, iterations=1):
    morph = morph.lower()
    if morph == 'none' or iterations <= 0:
        return bw
    k = oddize(ksize, minimum=1)
    kernel = np.ones((k, k), np.uint8)
    if morph == 'open':
        return cv2.morphologyEx(bw, cv2.MORPH_OPEN, kernel, iterations=iterations)
    if morph == 'close':
        return cv2.morphologyEx(bw, cv2.MORPH_CLOSE, kernel, iterations=iterations)
    if morph == 'erode':
        return cv2.erode(bw, kernel, iterations=iterations)
    if morph == 'dilate':
        return cv2.dilate(bw, kernel, iterations=iterations)
    return bw

def grid_to_text(bw, on_char='0', off_char=' '):
    """
    Map 255 -> on_char, 0 -> off_char. Ensure values are exactly 0/255 (for robustness, threshold at 128).
    """
    H, W = bw.shape
    # Ensure binary
    binarized = (bw >= 128).astype(np.uint8)
    # Build lines efficiently
    on_byte = on_char.encode('utf-8')
    off_byte = off_char.encode('utf-8')
    # We'll create a bytes buffer for speed, then decode once at the end
    out = bytearray()
    for r in range(H):
        row = binarized[r]
        # Build row bytes
        out.extend(on_byte if row[0] else off_byte)
        for c in row[1:]:
            out.extend(on_byte if c else off_byte)
        out.append(0x0A)  # newline
    return out.decode('utf-8', errors='strict')

def main():
    ap = argparse.ArgumentParser(
        description="Convert an image to a binary text grid (two characters; default: '0' and space)."
    )
    ap.add_argument("input", help="Path to input image (any format supported by OpenCV).")
    ap.add_argument("-o", "--output", help="Path to output .txt file. Defaults to <input_basename>.txt")
    # Size and aspect
    ap.add_argument("--width", type=int, help="Target character width (columns).")
    ap.add_argument("--height", type=int, help="Target character height (rows) BEFORE y-aspect compression.")
    ap.add_argument("--scale", type=float, help="Uniform scale factor if width/height not provided.")
    ap.add_argument("--y-aspect", type=float, default=0.5,
                    help="Vertical compression factor to compensate glyph aspect ratio (default: 0.5).")
    # Preprocess
    ap.add_argument("--clahe", action="store_true", help="Enable CLAHE contrast enhancement.")
    ap.add_argument("--clahe-clip", type=float, default=2.0, help="CLAHE clip limit (default 2.0).")
    ap.add_argument("--clahe-grid", type=int, default=8, help="CLAHE tile grid size (default 8).")
    ap.add_argument("--blur", type=int, default=0,
                    help="Gaussian blur kernel (odd). 0 disables. Helps denoise before thresholding.")
    # Binarization
    ap.add_argument("--method", choices=["otsu", "adaptive_mean", "adaptive_gaussian", "canny"],
                    default="otsu", help="Binarization method (default: otsu).")
    ap.add_argument("--adaptive-block", type=int, default=31, help="Adaptive threshold block size (odd).")
    ap.add_argument("--adaptive-c", type=int, default=5, help="Adaptive threshold fine-tuning constant.")
    ap.add_argument("--canny-lo", type=int, default=100, help="Canny lower threshold.")
    ap.add_argument("--canny-hi", type=int, default=200, help="Canny upper threshold.")
    # Morphology & tuning
    ap.add_argument("--morph", choices=["none", "open", "close", "erode", "dilate"], default="none",
                    help="Morphological op to smooth shapes and create larger solid areas (default: none).")
    ap.add_argument("--morph-ksize", type=int, default=3, help="Kernel size for morphology (odd).")
    ap.add_argument("--morph-iters", type=int, default=1, help="Iterations for morphology (default 1).")
    ap.add_argument("--invert", action="store_true",
                    help="Invert after binarization (swap ON/OFF).")
    # Characters
    ap.add_argument("--on-char", type=parse_char, default='0',
                    help=r"Character for ON pixels (default '0'). Supports escapes like '\t' or '\u2003'.")
    ap.add_argument("--off-char", type=parse_char, default=' ',
                    help=r"Character for OFF pixels (default space). Supports escapes like '\t' or '\u2003'.")
    ap.add_argument("--whitespace-only", action="store_true",
                    help="Use only whitespace characters: ON=tab, OFF=space. Warning: visualization may be tricky.")
    # Optional preview
    ap.add_argument("--save-preview", metavar="PNG_PATH",
                    help="Save the processed binary image as a PNG for sanity-checking.")
    args = ap.parse_args()

    in_path = Path(args.input)
    if not in_path.exists():
        print(f"Input not found: {in_path}", file=sys.stderr)
        sys.exit(1)

    out_path = Path(args.output) if args.output else in_path.with_suffix(".txt")

    # Read and grayscale
    img = cv2.imread(str(in_path), cv2.IMREAD_COLOR)
    if img is None:
        print(f"Failed to read image: {in_path}", file=sys.stderr)
        sys.exit(2)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Size
    h, w = gray.shape
    th, tw = compute_target_size(h, w, args.width, args.height, args.scale, args.y_aspect)
    if th != h or tw != w:
        gray = cv2.resize(gray, (tw, th), interpolation=cv2.INTER_AREA)

    # Contrast / denoise
    if args.clahe:
        gray = apply_contrast_enhancement(gray, args.clahe_clip, args.clahe_grid)
    if args.blur and args.blur > 0:
        k = oddize(args.blur, 1)
        gray = cv2.GaussianBlur(gray, (k, k), 0)

    # Binarize
    bw = binarize(gray, method=args.method, adaptive_block=args.adaptive_block,
                  adaptive_c=args.adaptive_c, canny_lo=args.canny_lo, canny_hi=args.canny_hi)

    # Post-process
    bw = apply_morph(bw, morph=args.morph, ksize=args.morph_ksize, iterations=args.morph_iters)
    if args.invert:
        bw = 255 - bw

    # Characters
    on_char = '\t' if args.whitespace_only else args.on_char
    off_char = ' ' if args.whitespace_only else args.off_char
    if len(on_char) != 1 or len(off_char) != 1:
        print("Error: on-char and off-char must be single characters.", file=sys.stderr)
        sys.exit(3)

    # Build text and save
    text = grid_to_text(bw, on_char=on_char, off_char=off_char)
    out_path.write_text(text, encoding='utf-8')
    print(f"Wrote text grid: {out_path}  (shape: {bw.shape[0]}x{bw.shape[1]})")

    # Optional preview image
    if args.save_preview:
        # If ON char is space-like and visualization is hard, preview helps
        # Ensure binary 0/255 PNG
        if bw.dtype != np.uint8:
            bw = (bw > 0).astype(np.uint8) * 255
        ok = cv2.imwrite(args.save_preview, bw)
        if ok:
            print(f"Saved preview PNG: {args.save_preview}")
        else:
            print(f"Failed to save preview to: {args.save_preview}", file=sys.stderr)

if __name__ == "__main__":
    main()
