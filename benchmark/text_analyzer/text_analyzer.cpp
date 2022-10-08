// Text Analyzer
// usage:
//
// text_anayler [--file ... [--file-alpha-bits ...]?]+
//
// Used for spitting out some useful information about all of the input texts in a SQLplottools friendly format

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <benchmark_util.hpp>
#include <huffman/huffman_tree.hpp>
#include <huffman/huffman_encoder.hpp>

#include <format/format_string.hpp>

#include <tlx/cmdline_parser.hpp>

std::vector<std::string> texts;
std::vector<std::string> text_sizes;
std::vector<size_t> text_sizes_integral;
bool no_highlight;
size_t prefix_text = 0;



template <typename value_type>
void process_file(const std::string& path, size_t bits, size_t prefix) {
    std::vector<value_type> text;
    if (read_file(path, bits, text, prefix)) {
        std::cout << "Error reading file " << path << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Get parameters
    std::cout << "Getting alphabet size..." << std::endl;
    size_t real_alpha_size = count_alphabet<value_type>(text);
    std::cout << "Getting H0..." << std::endl;
    double h0 = get_h0<value_type>(text);

    // Get size of Huffman-Encoded Text
    // Since we're not concerned with canonical codes, we just directly call the tree functions
    std::cout << "Getting Huffman Size" << std::endl;
    auto huff_tree = std::unique_ptr<huffman_tree>(get_huffman_tree(text));
    size_t max_length = 0;
    auto huffman_codes = get_huffman_codes<value_type>(huff_tree.get(), max_length);
    size_t huff_bit_size = 0;
    for (auto s : text) {
        huff_bit_size += huffman_codes[s].length;
    }

    // Output results
    std::cout << FMT.format("RESULT", fmt_color::BLUE)
        << " file=" << path
        << " size=" << text.size()
        << " alpha_size=" << real_alpha_size
        << " zero_order_entropy=" << h0
        << " huff_bit_size=" << huff_bit_size
        << " longest_huffman_code=" << max_length
        << std::endl;
}

int main(int argc, char** argv) {
    std::cout << std::boolalpha;
    std::cout << "Text Anaylzer" << std::endl;
    std::cout << "Current Time: " << get_timestamp() << std::endl << std::endl;

    tlx::CmdlineParser cmd;
    cmd.set_description("Text Analyzer");

    cmd.add_stringlist("file", texts, "Input file to read");
    cmd.add_stringlist("file-bits", text_sizes, "Size for input file to read, assumed 8 if missing");
    cmd.add_bytes("prefix", prefix_text, "If set, get prefix of text of 'n' characters");
    cmd.add_flag("no-highlight", no_highlight, "If enabled, do not highlight output");

    if (!cmd.process(argc, argv)) {
        std::exit(EXIT_FAILURE);
    }

    FMT.set_enabled(!no_highlight);

    if (texts.empty()) {
        std::cout << "Must provide at least one file!" << std::endl;
        cmd.print_usage();
        std::exit(EXIT_FAILURE);
    }

    // Calculate text widths in bits
    for (const auto& bits : text_sizes) {
        size_t bit_depth = std::stoi(bits);
        if (bit_depth > 64) {
            std::cout << "Invalid bit depth given: " << bits << std::endl;
            cmd.print_usage();
            std::exit(EXIT_FAILURE);
        }
        text_sizes_integral.push_back(bit_depth);
    }

    for (size_t i = 0; i < texts.size(); i++) {
        std::cout << "File: " << texts[i] << " (";
        if (text_sizes_integral.size() <= i) {
            std::cout << "8 bits [Default])" << std::endl;
            text_sizes_integral.push_back(8);
        } else {
            std::cout << text_sizes_integral[i] << " bits)" << std::endl;
        }
    }

    if (prefix_text != 0) {
        std::cout << "Getting prefix of " << prefix_text << " characters" << std::endl;
    }
    std::cout << std::endl << "Start..." << std::endl;

    for (size_t i = 0; i < texts.size(); i++) {
        std::cout << std::endl;
        std::cout << "Reading File: " << texts[i] << " (" << text_sizes_integral[i] << " bits)..." << std::endl;
        if (text_sizes_integral[i] <= 8) {
            process_file<uint8_t>(texts[i], text_sizes_integral[i], prefix_text);
        } else if (text_sizes_integral[i] <= 16) {
            process_file<uint16_t>(texts[i], text_sizes_integral[i], prefix_text);
        } else if (text_sizes_integral[i] <= 32) {
            process_file<uint32_t>(texts[i], text_sizes_integral[i], prefix_text);
        } else {
            process_file<uint64_t>(texts[i], text_sizes_integral[i], prefix_text);
        }
        std::cout << std::endl;
    }

    std::cout << std::endl << "Done" << std::endl;
}