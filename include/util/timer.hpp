#pragma once

#include <chrono>

class timer {
private:
    using clock_type = std::chrono::high_resolution_clock;
    using second_type = std::chrono::duration<double, std::ratio<1>>;
    std::chrono::time_point<clock_type> m_start_time;

public:
    inline timer() : m_start_time(clock_type::now()) {}

    inline void reset() {
        m_start_time = clock_type::now();
    }

    inline double elapsed() const {
        auto diff = clock_type::now() - m_start_time;
        return std::chrono::duration_cast<second_type>(diff).count();
    }
};
