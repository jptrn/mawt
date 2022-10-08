#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>


// A `wrapper` is a bridge between an arbitrary WT's implementation and the interface which the
// main benchmark class expects. A wrapper is a class that needs to support the following operations:
//
// <constructor>(const std::vector<uint64_t>& data, size_t depth)
// size_t access(size_t i) const
// size_t height() const
// size_t threads() const
// std::string ext(const std::string& key)
// static const std::string name()

class wt_wrapper_dummy {
public:
    inline static const std::string name() {
        return "dummy";
    }

    inline size_t threads() const {
        return 1;
    }

    inline wt_wrapper_dummy(const std::vector<uint64_t>&, size_t) {}

    inline size_t height() const {
        return 0;
    }

    inline size_t access(size_t) const {
        return 0;
    }

    inline std::string ext(const std::string&) {
        return "-";
    }
};