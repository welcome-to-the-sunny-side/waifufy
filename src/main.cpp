#include "waifufy_core.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using namespace waifufy;

struct Args {
    std::string code;
    std::string art;
    std::string out;
    std::optional<int> width;
    std::optional<int> height;
    bool dump_meta = false; // print W,H and token count to stderr
};

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " --code <path> --art <path> --out <path> [--width N] [--height N] [--dump-meta]\n";
}

static bool parse_int(std::string_view s, int& v) {
    if (s.empty()) return false;
    bool neg = false;
    size_t i = 0;
    if (s[0] == '-') { neg = true; i = 1; }
    long long acc = 0;
    for (; i < s.size(); ++i) {
        if (s[i] < '0' || s[i] > '9') return false;
        acc = acc * 10 + (s[i] - '0');
        if (acc > 1'000'000'000) return false;
    }
    v = static_cast<int>(neg ? -acc : acc);
    return true;
}

static bool parse_args(int argc, char** argv, Args& a) {
    for (int i = 1; i < argc; ++i) {
        std::string key = argv[i];
        auto need = [&](int more) {
            if (i + more >= argc) { usage(argv[0]); return false; }
            return true;
        };
        if (key == "--code") {
            if (!need(1)) return false; a.code = argv[++i];
        } else if (key == "--art") {
            if (!need(1)) return false; a.art = argv[++i];
        } else if (key == "--out") {
            if (!need(1)) return false; a.out = argv[++i];
        } else if (key == "--width") {
            if (!need(1)) return false; int v; if (!parse_int(argv[++i], v)) { usage(argv[0]); return false; } a.width = v;
        } else if (key == "--height") {
            if (!need(1)) return false; int v; if (!parse_int(argv[++i], v)) { usage(argv[0]); return false; } a.height = v;
        } else if (key == "--dump-meta") {
            a.dump_meta = true;
        } else if (key == "-h" || key == "--help") {
            usage(argv[0]);
            std::exit(0);
        } else {
            std::cerr << "Unknown option: " << key << "\n";
            usage(argv[0]);
            return false;
        }
    }
    if (a.code.empty() || a.art.empty() || a.out.empty()) { usage(argv[0]); return false; }
    return true;
}

int main(int argc, char** argv) {
    Args args;
    if (!parse_args(argc, argv, args)) return 2;

    // Read inputs
    std::string code_text = read_file(args.code);
    std::string art_text = read_file(args.art);

    // Tokenize: strip comments first, then tokenize
    std::string code_nc = strip_comments_preserve_literals(code_text);
    std::vector<Token> tokens = tokenize_minimal_cpp(code_nc);

    // Art -> density grid
    AsciiDensity map = default_ascii_density_01();
    Art art = parse_art_to_density(art_text, args.width, args.height, &map);

    if (args.dump_meta) {
        std::cerr << "W=" << art.W << " H=" << art.H << ", tokens=" << tokens.size() << "\n";
    }

    // Convert using placeholder (you implement real layout inside convert_layout)
    std::string output_code = convert_layout(tokens, art.W, art.H, art.density, map);

    // Ensure output directory exists
    try {
        std::filesystem::path outp(args.out);
        if (outp.has_parent_path()) {
            std::filesystem::create_directories(outp.parent_path());
        }
    } catch (...) {
        // ignore
    }

    // Write output
    std::ofstream ofs(args.out, std::ios::out | std::ios::binary);
    if (!ofs) {
        std::cerr << "Failed to open --out path: " << args.out << "\n";
        return 3;
    }
    ofs.write(output_code.data(), static_cast<std::streamsize>(output_code.size()));
    ofs.flush();

    return 0;
}
