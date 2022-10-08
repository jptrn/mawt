#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

// As we cannot test Kaneta's implementation directly, we instead 
// extrapolate his results from the numbers we got
// We directly parse the files into a SQLPlotTools result line,
// which can be nicely read by sqlplottools

struct result_line {
private:
    std::unordered_map<std::string, float> m_data;

public:

    float get(const std::string& key) const {
        return m_data.find(key)->second;
    }

    result_line() { }

    result_line(float naive, float k_pc, float k_ps, float k_rd, float k_rs, float k_pshufb, float k_pext, float pwm_pc, float pwm_ps) { 
        m_data["naive"] = naive;
        m_data["pc"] = k_pc;
        m_data["ps"] = k_ps;
        m_data["route-d"] = k_rd;
        m_data["route-s"] = k_rs;
        m_data["pshufb"] = k_pshufb;
        m_data["pext"] = k_pext;
        m_data["pwm_pc"] = pwm_pc;
        m_data["pwm_ps"] = pwm_ps;

        // Extrapolate numbers for kaneta
        // To obtain the results that Kaneta's implementations would have gotten on kryptonit, we extrapolate them
        // To do this, we calculate the factor between pc in Kaneta's paper and pc in our paper
        // We could also use ps, or average both, all choices are equally arbitrary.

        float pc_extrapolation_factor = pwm_pc / k_pc;
        m_data["pshufb_extra"] = k_pshufb * pc_extrapolation_factor;
        m_data["pext_extra"] = k_pext * pc_extrapolation_factor;
    }
};

struct text_info {
    size_t size;
    size_t alpha_size;
    size_t depth;
    std::string name;

    text_info() { }

    text_info(size_t v_size, size_t v_alpha_size, size_t v_depth) :
        size(v_size),
        alpha_size(v_alpha_size),
        depth(v_depth) { }
};

// Maps each text to its results
using result_collection = std::map<std::string, result_line>;

void print_result_line(const std::string& key, const std::string& wx_type, const text_info& ti, const result_line& rc, size_t ds_order) {
            std::cout << "RESULT"
                << " iteration=" << 0
                << " type=" << wx_type << "_kaneta_" << key
                << " operation=" << "ctor"
                << " file=" << ti.name
                << " size=" << ti.size
                << " height=" << ti.depth
                << " alpha_bits=" << ti.depth
                << " alphabet_size=" << ti.alpha_size
                << " time_in_s=" << rc.get(key)
                << " threads=" << 1 
                << " space_in_bytes=" << 0
                << " working_space=" << 0
                << " mode=" << "fixed"
                << " seed=" << 0
                << " ds_order=" << ds_order
                // Extra keys
                << " ext_get_huffman_codes=-"
                << " ext_make_tree=-"
                << " ext_ctor=-"
                << " ext_init=-"
                << " ext_split=-"
                << " ext_get_borders=-"
                << " ext_merge=-"
                << std::endl;
}

void output_results(const std::string& key, std::map<std::string, result_collection>& results, std::unordered_map<std::string, text_info>& texts, size_t ds_order) {
    // key is either pext, pshufb, naive, pc, ps, route-d or route-s
    for (auto [ wx_type, text_results ] : results) {
        for (auto [ filename, filename_results ] : text_results) {
            print_result_line(key, wx_type, texts[filename], filename_results, ds_order);
        }
        ds_order++;
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


int main(int, char**) {
    std::cout << "Extrapolating results from Kaneta 18" << std::endl;

    std::map<std::string, result_collection> results;

    // Data entry, WT results
    result_collection wt_results;
    wt_results["dblp.xml"] = result_line(5.57, 2.99, 3.03, 4.33, 3.65, 3.09, 3.04, 3.2492, 3.38225);
    wt_results["dna"] =      result_line(4.43, 2.42, 2.72, 2.47, 2.29, 2.07, 2.05, 2.59398, 2.72807);
    wt_results["english"] =  result_line(53.0, 27.2, 28.8, 32.1, 27.9, 23.7, 23.5, 27.0657, 29.3429);
    wt_results["pitches"] =  result_line(1.25, 0.685, 0.812, 0.741, 0.659, 0.576, 0.570, 0.736077, 0.782538);
    wt_results["proteins"] = result_line(16.1, 8.29, 8.67, 12.8, 10.9, 9.27, 9.12, 8.91129, 9.30639);
    wt_results["sources"] =  result_line(4.94, 2.54, 2.61, 3.21, 3.09, 2.37, 2.55, 2.70738, 2.76978);

    wt_results["chr22.dna"] =       result_line(0.233, 0.143, 0.188, 0.190, 0.173, 0.157, 0.156, 0.160094, 0.180847);
    wt_results["etext99"] =         result_line(2.32, 1.31, 1.62, 1.57, 1.35, 1.16, 1.14, 1.30212, 1.40402);
    wt_results["gcc-3.0.tar"] =     result_line(1.91, 1.32, 1.12, 1.467, 1.105, 0.949, 0.935, 1.12612, 1.15296);
    wt_results["howto"] =           result_line(0.832, 0.478, 0.496, 0.592, 0.512, 0.438, 0.432, 0.501835, 0.547473);
    wt_results["jdk13c"] =          result_line(1.30, 0.708, 0.789, 1.03, 0.877, 0.829, 0.755, 0.764915, 0.801313);
    wt_results["linux-2.4.5.tar"] = result_line(2.76, 1.41, 1.45, 2.24, 1.95, 1.30, 1.51, 1.51293, 1.57052);
    wt_results["rctail96"] =        result_line(2.25, 1.18, 1.20, 1.66, 1.40, 1.19, 1.17, 1.26148, 1.33957);
    wt_results["rfc"] =             result_line(2.28, 1.25, 1.27, 1.62, 1.42, 1.20, 1.19, 1.34428, 1.41356);
    wt_results["sprot34.dat"] =     result_line(2.12, 1.14, 1.36, 1.54, 1.70, 1.13, 1.13, 1.2145, 1.21848);
    wt_results["w3c2"] =            result_line(2.30, 1.30, 1.28, 1.66, 1.41, 1.30, 1.18, 1.32002, 1.37317);
    results["wt"] = wt_results;

    // WM Results
    result_collection wm_results;
    wm_results["dblp.xml"] = result_line(6.20, 3.00, 3.01, 3.65, 2.74, 2.02, 2.05, 3.26632, 3.41937);
    wm_results["dna"] =      result_line(5.88, 2.43, 2.70, 1.81, 1.53, 1.29, 1.30, 2.62318, 2.74689);
    wm_results["english"] =  result_line(57.0, 27.2, 28.5, 26.7, 20.6, 15.8, 15.9, 27.1954, 29.5603);
    wm_results["pitches"] =  result_line(1.37, 0.684, 0.709, 0.595, 0.495, 0.429, 0.547, 0.738091, 0.786106);
    wm_results["proteins"] = result_line(22.4, 8.29, 8.41, 11.1, 8.44, 6.36, 6.43, 8.98639, 9.3888);
    wm_results["sources"] =  result_line(5.81, 2.54, 2.95, 2.69, 2.28, 1.57, 1.59, 2.71768, 2.79738);

    wm_results["chr22.dna"] =       result_line(0.385, 0.143, 0.164, 0.132, 0.110, 0.130, 0.092, 0.162687, 0.178777);
    wm_results["etext99"] =         result_line(3.01, 1.27, 1.34, 1.30, 1.00, 0.803, 0.771, 1.30896, 1.41665);
    wm_results["gcc-3.0.tar"] =     result_line(2.18, 1.05, 1.29, 1.13, 0.828, 0.633, 0.639, 1.13043, 1.16494);
    wm_results["howto"] =           result_line(1.011, 0.478, 0.494, 0.495, 0.384, 0.295, 0.298, 0.50366, 0.538821);
    wm_results["jdk13c"] =          result_line(1.57, 0.708, 0.705, 1.17, 0.665, 0.500, 0.506, 0.768796, 0.809449);
    wm_results["linux-2.4.5.tar"] = result_line(3.14, 1.41, 1.64, 1.46, 1.14, 0.872, 1.11, 1.52008, 1.5859);
    wm_results["rctail96"] =        result_line(2.37, 1.22, 1.19, 1.39, 1.27, 0.779, 0.792, 1.26825, 1.35159);
    wm_results["rfc"] =             result_line(2.65, 1.30, 1.28, 1.34, 1.05, 0.791, 0.811, 1.34833, 1.42676);
    wm_results["sprot34.dat"] =     result_line(2.81, 1.14, 1.38, 1.27, 0.982, 0.905, 0.855, 1.22009, 1.23254);
    wm_results["w3c2"] =            result_line(2.63, 1.54, 1.27, 1.73, 1.07, 0.80, 0.81, 1.32552, 1.3869);
    results["wm"] = wm_results;

    std::unordered_map<std::string, text_info> text_infos;
    // These are also entered manually, I could automate it, but it's probably simpler to just do it like this
    text_infos["dblp.xml"] = text_info(296135874ULL, 97, 7); 
    text_infos["dna"]      = text_info(403927746ULL, 16, 4);
    text_infos["english"]  = text_info(2210395553ULL, 239, 8);
    text_infos["pitches"]  = text_info(55832855ULL, 133, 8);
    text_infos["proteins"] = text_info(1184051855ULL, 27, 5);
    text_infos["sources"]  = text_info(210866607ULL, 230, 8);

    text_infos["chr22.dna"] =       text_info(34553758ULL, 5, 3);
    text_infos["etext99"] =         text_info(105277340ULL, 146, 8);
    text_infos["gcc-3.0.tar"] =     text_info(86630400ULL, 150, 8);
    text_infos["howto"] =           text_info(39422105ULL, 197, 8);
    text_infos["jdk13c"] =          text_info(69728899ULL, 113, 7);
    text_infos["linux-2.4.5.tar"] = text_info(116254720ULL, 256, 8);
    text_infos["rctail96"] =        text_info(114711151ULL, 93, 7);
    text_infos["rfc"] =             text_info(116421901ULL, 120, 7);
    text_infos["sprot34.dat"] =     text_info(109617186ULL, 66, 7);
    text_infos["w3c2"] =            text_info(104201579ULL, 256, 8);

    for (auto& [ text_name, text_info ] : text_infos) {
        text_info.name = text_name;
    }

    output_results("pext_extra", results, text_infos, 300);
    output_results("pshufb_extra", results, text_infos, 400);

  
    std::cout << "Done" << std::endl;
}