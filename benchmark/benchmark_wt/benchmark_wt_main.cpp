// the main benchmark executable

#include <cstddef>
#include <cstdlib>
#include <iostream>

#include <benchmark_wt/benchmark_params.hpp>
#include <benchmark_wt/benchmark_random.hpp>
#include <benchmark_wt/benchmark_read_file.hpp>
#include <benchmark_wt/benchmark_rng.hpp>

#include <format/format_string.hpp>

#include <tlx/cmdline_parser.hpp>

int main(int argc, char** argv) {
    std::cout << std::boolalpha;
    std::cout << "Starting Wavelet Tree Benchmark" << std::endl;

    // Print Timestamp
    std::cout << "Current time: " << get_timestamp() << std::endl;

    // Process command line
    tlx::CmdlineParser cmd;
    cmd.set_description("Wavelet Tree Benchmark");

    benchmark_params params;
    cmd.add_size_t("iterations", params.iterations, "Number of iterations to run the benchmark for");
    cmd.add_flag("random", params.random_seed, "Randomize RNG seed");
    cmd.add_size_t("seed", params.seed, "RNG seed to use. Only required if 'random' not set");
    cmd.add_stringlist("file", params.file_paths, "Path to file. Needs a corresponding --file-alphabet-bits command, otherwise the file is assumed to be byte-encoded");
    cmd.add_stringlist("file-alpha-bits", params.file_alpha_bits, "Size of file given with --file");
    cmd.add_bytes("min", params.text_min, "If path not given, generate random texts with sizes starting at given parameter");
    cmd.add_bytes("max", params.text_max, "If path not given, generate random texts with sizes ending at given parameter");
    cmd.add_size_t("alpha-min", params.alpha_bits_min, "If path not given, generate random text with alphabet starting at n bits, otherwise read text with n bits per character");
    cmd.add_size_t("alpha-max", params.alpha_bits_max, "If path not given, generate random text with alphabet up to n bits");
    cmd.add_size_t("alpha-step", params.alpha_bits_step, "If path not given, go up n bits with each step");
    cmd.add_string("distribution", params.distribution, "If a random text is given, use the following distribution");
    cmd.add_stringlist("implementation", params.implementations, params.known_implementations_help_text());
    cmd.add_stringlist("ext", params.extra_keys, "Extra keys that should be output for each data structure");
    cmd.add_flag("all", params.all_implementations, "Benchmark all data structures");
    cmd.add_flag("no-highlight", params.no_highlight_output, "If enabled, do not highlight output");
    cmd.add_flag("suffix-array", params.create_suffix_array, "If a benchamrk is read from file, creates a suffix array");
    cmd.add_flag("get-words", params.remap_to_words, "Build input for WT based on words of input text");
    cmd.add_size_t("get-words-sep", params.remap_to_words_sep, "If option 'get-words' is set, use the character corresponding to the given ascii code as separator, default: space");
    cmd.add_flag("verify", params.verify, "Verify correct implementation result and fail fast if an error is detected");
    cmd.add_flag("compute-effective-alphabet", params.compute_effective_alphabet, "Compute the effective alphabet for the text");

    if (!cmd.process(argc, argv)) {
        std::exit(EXIT_FAILURE);
    }
    FMT.set_enabled(!params.no_highlight_output);
    params.process();
    benchmark_rng rng(params);
    std::cout << "Real Seed: " << rng.seed() << std::endl;

    std::cout << std::endl;
    std::cout << "Starting Benchmark..." << std::endl;

    benchmark_results results;
    
    if (params.file_paths.empty()) {
        for (size_t iteration = 0; iteration < params.iterations; iteration++) {
            std::cout << "Iteration " << iteration << "..." << std::endl;
            for (size_t size = params.text_min; size <= params.text_max; size *= 2) {
                std::cout << "Size: " << size << "..." << std::endl;
                for (size_t alpha_bits = params.alpha_bits_min; alpha_bits <= params.alpha_bits_max; alpha_bits += params.alpha_bits_step) {
                    std::cout << "Alphabet Bits: " << alpha_bits << "..." << std::endl;
                    // Generate new text in each new iteration
                    results.results.clear();
                    bench_random(params, results, rng, iteration, size, alpha_bits);
                    std::cout << std::endl;
                }
            }
        }
    } else {
        for (size_t p = 0; p < params.file_paths.size(); p++) {
            std::cout << "Text: " << params.file_paths[p] << " (" << params.file_alpha_bits_num[p] << " bits)..." << std::endl;
            results.results.clear();
            // Run multiple iterations here to not generate the (same) text multiple times
            bench_read_file(params, results, params.file_paths[p], params.file_alpha_bits_num[p]);
        }
        std::cout << std::endl;
    }

    if (results.failed) {
        std::cout << FMT.format("!! Warning !!", fmt_color::RED, fmt_color::NONE, fmt_fx::BOLD) 
            << " Mismatch detected" << std::endl 
            << results.failure_msg << std::endl;
        std::cout << "Failed implementations: " << std::endl;
        for (auto c : results.failed_implementations) {
            std::cout << "- " << c << std::endl;
        }
    } 

    std::cout << "Done" << std::endl;
    std::cout << "Finish Time: " << get_timestamp() << std::endl;
    return EXIT_SUCCESS;    
}