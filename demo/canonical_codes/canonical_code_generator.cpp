#include <huffman/huffman_code.hpp>
#include <huffman/huffman_encoder.hpp>
#include <util/debug.hpp>

#include <cstddef>
#include <iostream>
#include <unordered_map>

int main(int, char**) {
    std::cout << "Generating canonical encoding for given codes" << std::endl;

    std::unordered_map<size_t, huffman_code> codes;
    codes[0] = { 0, 2 };
    codes[1] = { 0, 3 };
    codes[2] = { 0, 3 };
    codes[3] = { 0, 4 };
    codes[4] = { 0, 4 };
    codes[5] = { 0, 4 };

    auto canonized = get_canonical_huffman_codes(codes);

    for (auto [ symbol, code ] : canonized) {
        auto [ c, length ] = code;
        std::cout << "[" << symbol << "] = { " << print_bits(c) << ", " << length << " }" << std::endl;
    }


    std::cout << "Done" << std::endl;
}   
