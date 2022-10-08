#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

enum class fmt_color {
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7,
    NONE = -1
};

enum class fmt_fx {
    NONE = 0,
    BOLD = 1,
    UNDERLINE = 2,
    INVERSE = 4,
};

class format_string {
private:
    bool m_enabled = true;


public:
    std::string format(const std::string& input, fmt_color fg = fmt_color::NONE, fmt_color bg = fmt_color::NONE, fmt_fx effect = fmt_fx::NONE) {
        if (!enabled()) {
            return input;
        }

        size_t has_fg = fg != fmt_color::NONE;
        size_t has_bg = bg != fmt_color::NONE;
        size_t has_bold = (size_t)effect &(size_t)fmt_fx::BOLD;
        size_t has_underline = (size_t)effect & (size_t)fmt_fx::UNDERLINE;
        size_t has_inverted = (size_t)effect & (size_t)fmt_fx::INVERSE;

        std::vector<size_t> codes;
        if (has_fg) {
            codes.push_back((size_t)fg + 30);
        }
        if (has_bg) {
            codes.push_back((size_t)bg + 40);
        }
        if (has_bold) {
            codes.push_back(1);
        }
        if (has_underline) {
            codes.push_back(4);
        }
        if (has_inverted) {
            codes.push_back(7);
        }

        if (codes.empty()) {
            return input;
        }

        std::stringstream s;
        s << "\033[" << codes[0];
        for (size_t i = 1; i < codes.size(); i++) {
            s << ';' << codes[i];
        }
        s << "m" << input << "\033[0m";
        return s.str();
    }

    bool enabled() const {
        return m_enabled;
    }

    void set_enabled(bool enabled) {
        m_enabled = enabled;
    }
} FMT;