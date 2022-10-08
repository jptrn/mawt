#pragma once

#include <cstddef>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>

#include <benchmark_util.hpp>

// Holds all parameter sfor the benchmark, along with random number generation
struct benchmark_params {
public:
    std::vector<std::string> KNOWN_IMPLEMENTATIONS {
        "dummy",
        "basic",
        "twophase_no_lut", 
        "twophase_4_4", 
        "twophase_pext_pext",
        "twophase_pext_pshufb",
        "lwt_basic",
        "lwt_twophase_no_lut", 
        "lwt_twophase_4_4", 
        "lwt_twophase_pext_pext",
        "lwt_twophase_pext_pshufb",
        "pwm_pc",
        "pwm_ps"
    };

    std::unordered_set<std::string> KNOWN_DISTRIBUTIONS {
        "uniform"
    };

    static const size_t DEFAULT_SIZE = 65536;
    static const size_t DEFAULT_ALPHA_BITS = 8;

    size_t iterations = 3;
    bool random_seed = false;
    size_t seed = 0;
    bool verify = false;
    bool compute_effective_alphabet = false;
    std::vector<std::string> implementations;   // currently unused
    std::vector<std::string> extra_keys;        // Extra data-structure specific keys that are output with each DS. Mainly used for getting different times for different parts of a construction algorithm
    bool all_implementations;
    bool create_suffix_array = false;
    bool remap_to_words = false;
    size_t remap_to_words_sep = ' ';
    bool no_highlight_output = false;

    // If Input is read from file
    std::vector<std::string> file_paths;
    std::vector<std::string> file_alpha_bits;   // tlx only allows string lists, so we need to do the parsing ourselves
    std::vector<size_t> file_alpha_bits_num;

    // If Input is generated randomly
    size_t text_min = 0;
    size_t text_max = 0;
    size_t alpha_bits_min = 0;
    size_t alpha_bits_max = 0;
    size_t alpha_bits_step = 0;
    std::string distribution = "uniform";

private:
    void calculate_file_alpha_bits() {
        for (const auto& bits : file_alpha_bits) {
            size_t bit_depth = std::stoi(bits);
            if (bit_depth > 64) {
                std::cout << "Invalid bit depth given: " << bits << std::endl;
                std::exit(EXIT_FAILURE);
            }
            file_alpha_bits_num.push_back(bit_depth);
        }
    }

public:
    // -- Utility function: Prints all known implementations --
    std::string known_implementations_help_text() {
        std::stringstream s;
        s << "Benchmark the following implementations. Known implementations are: ";
        s << join_to_string(KNOWN_IMPLEMENTATIONS);
        return s.str();
    }

    // Process, verify and output the benchmark parameters
    void process() {
        if (!KNOWN_DISTRIBUTIONS.contains(distribution)) {
            std::cout << "Unknown distribution '" << distribution << "'!" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        if (!file_paths.empty()) {
            calculate_file_alpha_bits();
        }   
        if (alpha_bits_min > 64 || alpha_bits_max > 64) {
            std::cout << "Invalid alphabet size!" << std::endl;
            std::exit(EXIT_FAILURE);            
        }

        std::cout << std::endl << "Config:" << std::endl << std::endl;
        std::cout << "Iterations: " << iterations << std::endl;
        if (!file_paths.empty()) {
            std::cout << "Reading files from disk:" << std::endl;
            for (size_t i = 0; i < file_paths.size(); i++) {
                std::cout << "File: " << file_paths[i] << " (";
                if (file_alpha_bits_num.size() <= i) {
                    std::cout << "8 bits [Default])" << std::endl;
                    file_alpha_bits_num.push_back(8);
                } else {
                    std::cout << file_alpha_bits_num[i] << " bits)" << std::endl;
                }
            }
            if (text_min != 0) {
                std::cout << "Getting prefix of " << text_min << " elements" << std::endl;
            }
            if (create_suffix_array) {
                std::cout << "Creating suffix arrays..." << std::endl;
            } else if (remap_to_words) {
                std::cout << "Get words from text, separate on '" << remap_to_words_sep <<  '\'' << std::endl;                    
            }
            if (create_suffix_array && remap_to_words) {
                std::cout << "Invalid parameter combination: --suffix_array, --get_words" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        } else {
            std::cout << "Generating random text..." << std::endl;
            if (text_min == 0) {
                text_min = DEFAULT_SIZE;
                text_max = DEFAULT_SIZE;
            }
            if (alpha_bits_min == 0) {
                alpha_bits_min = DEFAULT_ALPHA_BITS;
                alpha_bits_max = DEFAULT_ALPHA_BITS;
            }
            if (alpha_bits_step == 0) {
                alpha_bits_step = 8;
            }
            std::cout << "Size (Min): " << text_min << std::endl;
            std::cout << "Size (Max): " << text_max << std::endl;
            std::cout << "Alphabet Size (Min): " << (1ULL << alpha_bits_min) << " (" << alpha_bits_min << " bits)" << std::endl;
            std::cout << "Alphabet Size (Max): " << (1ULL << alpha_bits_max) << " (" << alpha_bits_max << " bits)" << std::endl;
            std::cout << "Alphabet Bits Increment: " << alpha_bits_step << std::endl;
            std::cout << "Distribution: " << distribution << std::endl;
        }

        if (implementations.empty() || all_implementations) {
            implementations.insert(implementations.end(), KNOWN_IMPLEMENTATIONS.begin(), KNOWN_IMPLEMENTATIONS.end());
        }
        std::cout << "Implementations: " << join_to_string(implementations) << std::endl;
        std::cout << "Distribution: " << distribution << std::endl;
        std::cout << "Is random seed: " << random_seed << std::endl;
        std::cout << "Seed: " << seed << std::endl;

        if (!extra_keys.empty()) {
            std::cout << "Extra keys: " << join_to_string(extra_keys) << std::endl;
        }

        std::cout << "Verifying Results: " <<  verify << std::endl;
        std::cout << "Computing Effective Alphabet: " << compute_effective_alphabet << std::endl;
    }
};