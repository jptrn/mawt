#pragma once

#include <util/timer.hpp>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>

/*
 * A simple utility class for more fine-grained time measuring.
 * Time intervals can be measured and associated with specific names to be identified later
 * Note that there is no automatic namespacing, so if multiple phases might have the same name, a
 * prefix should be added to the name accordingly.
 * 
 * Using this class will impose some unknown (if small) runtime overhead. It can therefore
 * be statically and dynamically be disabled to be a no-op class, if desired
 * 
 * Define the preprocessor macro TIME_MEASURE_DISABLE to disable time measurements at compile time
 * 
 * Also note that there is no sanity checking, so the same key may be re-used, one measurement may be 
 * started, but never terminated, or a measurement may be terminated that was never started
 * It is up to the user to ensure this class is used correctly.
 */

class time_measure {
public:
    static time_measure DUMMY_MEASURE;

private:
    std::unordered_map<std::string, timer> m_timers;
    std::unordered_map<std::string, double> m_duration;
    bool m_enabled = true;    

public:
    inline time_measure(bool enable = true) : 
        m_timers(),
        m_duration(),
        m_enabled(enable) {}

    inline void start([[maybe_unused]] const std::string& key) {
        #ifndef TIME_MEASURE_DISABLE
        if (m_enabled) {
            m_timers[key].reset();
        }
        #endif
    }

    inline void end([[maybe_unused]] const std::string& key) {
        #ifndef TIME_MEASURE_DISABLE
        if (m_enabled) {
            m_duration[key] = m_timers[key].elapsed();
        }
        #endif
    }

    inline double get([[maybe_unused]] const std::string& key) {
        #ifndef TIME_MEASURE_DISABLE
        if (m_enabled) {
            return m_duration[key];
        }
        #endif
        return 0;
    }

    inline bool enabled() const {
        #ifndef TIME_MEASURE_DISABLE
        return false;
        #endif
        return m_enabled;
    }

    inline void set_enabled([[maybe_unused]] bool enabled) {
        #ifndef TIME_MEASURE_DISABLE
        m_enabled = enabled;
        #endif
    }
};

time_measure TIME_MEASURE_DUMMY;    // Use if you don't care about measured times