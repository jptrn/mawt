# Constructing Wavelet Trees with SIMD Instructions

Code repository for Wavelet Tree Construction Master's Thesis. This project is implemented as a C++ Header Library using C++20 and CMake. The algorithm is based on the O(n * (log sigma / sqrt(log n))) time  construction algorithm introduced by Babenko et al. (2014) and Munro et al. (2016) and the work of Kaneta (2018).

## Minimum Requirements

* git
* gcc Version 9.3.0 or higher
* cmake Version 3.16.3 or higher 
* CPU with support for the following instruction sets
  * BMI2
  * SSE instructions including:
    * SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2
  * AVX-512 instructions including the following extensions:
    * AVX-512, AVX-512F, AVX-512BW, AVX-512512BITALG, AVX-512VBMI, AVX-512VBMI2, AVX-512VL, AVX-512VPOPCNTDQ
* OpenMP

If your system does not support these instruction sets, you can emulate the program using the [Intel Software Development Emulator](https://www.intel.com/content/www/us/en/developer/articles/tool/software-development-emulator.html) (Intel SDE). See the webpage for instructions. You can use the SDE to execute the tests (`sde64 -- make test`) or directly run executables (`sde64 -- demo/usage_example/usage_example`).

## Building

```
git submodule update --init
mkdir build && cd build
cmake ..
make
```

First, download all dependencies. Then create a directory for your build and switch to it. Execute cmake with `cmake ..` to create the build files and build the project with `make`.

By default, the project will be built with `-march='native'`. If your system does not support the necessary instruction sets, override the `ARCH_FLAGS` variable (`-DARCH_FLAGS="..."`) with the flags listed in `ArchFlags.txt` to make GCC build the project with the correct architecture flags enabled. You can then run the program with the Intel SDE as described above.

Use the `-DCMAKE_BUILD_TYPE="..."` option to switch between Debug and Release mode. Release mode enables all optimizations, while Debug mode enables GDB support. Note that you must delete `build/CMakeCache.txt` for the change in build type to take effect.

Allowed values:
* `cmake -DCMAKE_BUILD_TYPE="Debug" ..`
* `cmake -DCMAKE_BUILD_TYPE="Release" ..`
* `cmake -DCMAKE_BUILD_TYPE="RelWithDebInfo" ..`

Executables will be placed in the build directory under the same relative path as in the project source tree. For example, after building, run the usage example demo with `demo/usage_example/usage_example`.

## Testing 

Tests are implemented with [GoogleTest](https://github.com/google/googletest). Run all unit tests with `make test`.  Alternatively, execute individual test cases by running the executables under `build/test`.

## Benchmarking

The main benchmark file is `build/benchmark/benchmark_wt`. Execute the standard benchmark suite with `make run_test_benchmark`, which runs the main benchmark with a standard set of parameters and logs the results in `benchmark/log/temp`.

To run the benchmark with your own parameters, run the benchmark executable directly with your parameters or call `bench.sh` with your parameters of choice, which automatically logs the results in `benchmark/log/temp`. See `benchmark/test_benchmark.sh` for an example.

Included third-party implementations we compare against:

* [kurpicz/pwm (Fischer, Kurpicz, Löbel 2018)](https://github.com/kurpicz/pwm)

## Overview

The project is a C++ library of consisting of header files providing classes and functions implementing wavelet trees and wavelet matrices. All implementations are located in `include`.

The main entry point is `include/wt/lwt.hpp` for the (levelwise) wavelet tree and `include/wt/wm.hpp` for the wavelet matrix. Use the static factory methods `lwt::make` and `wm::make` to create a wavelet tree/matrix from a given input sequence. For construction, you need a `wt_params` tag. This object is a token that bundles all meta-parameters for the wavelet tree/matrix construction, such as which algorithm to choose, and hyperparameters for the algorithm. See the definition in `include/wt/wt_params.hpp` for details. This object is passed to the algorithm as a template parameter.

The following type tokens are available:

* `wt_params_basic` - Naive construction
* `wt_params_no_lut<tau, n, type>` - Break the algorithm into two phases, but process each element individually
* `wt_params_lut<tau, n, divisions, type>` - Use a Lookup Table as described by Babenko et al. and Kaneta et al.
* `wt_params_pext<tau, type>` - Use a PEXT-based implementation as shown by Kaneta, for `sizeof(type) <= 8`
* `wt_params_shuffle<type>`- Use a PSHUFB-based implementation, for `sizeof(type) >= 8`, and `tau = 8`

The parameter `type` denotes which data type is used for the temporary buffers. Its size in bits corresponds to the parameter `b` in theory. If all three can be supplied, `n * tau` must be equal to `sizeof(type) * 8`, otherwise `n = sizeof(type) * 8 / tau`.

The type token `wt_params_lut` is not available for Huffman-shaped wavelet trees/matrices.

Usage example:

```c++
    #include <wt/lwt.hpp>
    #include <wt/wt_params.hpp>
    #include <cstdint>
    #include <vector>

    // ...

    // wt_impl_type contains metaparameters for the wavelet tree construction algorithm
    // 'wt_params_lut<2, 4>' creates a LUT-based implementation of the two-phase algorithm of Babenko and Munro, with tau = 2 and N = 4
    // see include/wt/wt_params.hpp for more possible types
    using wt_impl_type = wt_params_lut<2, 4>;
    
    std::vector<uint8_t> values { ... };
    auto t = lwt::make<uint8_t, wt_impl_type>(values, 8);
```

When using domain decomposition, use the functions `lwt_make_dd` and `wm_make_dd` to create wavelet trees and wavelet matrices, respectively. You must provide a separate type token for the domain decomposition paramters.

See `include/wt/parallel/wt_dd_params.hpp` for details.

Domain decomposition usage example:

```c++
    #include <wt/parallel/lwt_dd.hpp>
    #include <wt/parallel/wt_dd_params.hpp>
    #include <wt/lwt.hpp>
    #include <wt/wt_params.hpp>
    #include <cstdint>
    #include <vector>
    #include <omp.h>

    // ...

    // The thread count is set externally to the construction algorithm
    constexpr size_t thread_count = 4;
    omp_set_num_threads(thread_count);

    // wt_impl_type contains metaparameters for the wavelet tree construction algorithm
    // 'wt_params_lut<2, 4>' creates a LUT-based implementation of the two-phase algorithm of Babenko and Munro, with tau = 2 and N = 4
    // see include/wt/wt_params.hpp for more possible types
    using wt_impl_type = wt_params_lut<2, 4>;

    // wt_dd_params_type contains metaparameters for the domain decomposition algorithm
    // see include/wt/parallel/wt_dd_params.hpp for details
    using wt_dd_params_type = wt_dd_params<uint64_t, thread_count>;
    
    std::vector<uint8_t> values { ... };
    auto t = lwt_make_dd<uint8_t, wt_impl_type, wt_dd_params_type>(values, 8);
```

See `demo/usage_example/usage_example.cpp` for a complete usage example.




