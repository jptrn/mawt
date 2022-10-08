#pragma once

#include <cstddef>
#include <string>
#include <unordered_set>
#include <vector>

// Used for holding persistent "result" data between benchmark iterations
// Used for verifying the WT implementations are correct

struct benchmark_results {
public:
    std::vector<size_t> results;
    std::unordered_set<std::string> failed_implementations;
    std::string failure_msg = "";
    bool failed = false;
};