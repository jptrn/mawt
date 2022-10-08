#pragma once

#include <unistd.h>

#include <malloc_count/malloc_count.h>
#include <malloc_count/stack_count.h>

class spacer {
private:
    double m_used;

public:
    inline spacer() {
        reset();
    }

    inline void reset() {
        m_used = malloc_count_current();
        malloc_count_reset_peak();
    }

    inline ssize_t space_used() {
        return malloc_count_current() - m_used;
    }

    inline ssize_t space_peak() {
        return malloc_count_peak() - m_used;
    }
};
