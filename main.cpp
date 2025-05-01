#include <iostream>
#include <intrin.h>

void print_simd_support() {
    int cpuInfo[4] = {0};

    // Get highest function parameter
    __cpuid(cpuInfo, 0);
    int maxFunctionId = cpuInfo[0];

    // Basic feature flags
    __cpuid(cpuInfo, 1);
    bool sse     = (cpuInfo[3] & (1 << 25)) != 0;
    bool sse2    = (cpuInfo[3] & (1 << 26)) != 0;
    bool sse3    = (cpuInfo[2] & (1 << 0))  != 0;
    bool ssse3   = (cpuInfo[2] & (1 << 9))  != 0;
    bool sse41   = (cpuInfo[2] & (1 << 19)) != 0;
    bool sse42   = (cpuInfo[2] & (1 << 20)) != 0;
    bool avx     = (cpuInfo[2] & (1 << 28)) != 0;

    // Extended features (AVX2, AVX512)
    __cpuid(cpuInfo, 7);
    bool avx2    = (cpuInfo[1] & (1 << 5)) != 0;
    bool avx512f = (cpuInfo[1] & (1 << 16)) != 0;

    std::cout << "SIMD support:\n";
    std::cout << "  SSE   : " << sse << "\n";
    std::cout << "  SSE2  : " << sse2 << "\n";
    std::cout << "  SSE3  : " << sse3 << "\n";
    std::cout << "  SSSE3 : " << ssse3 << "\n";
    std::cout << "  SSE4.1: " << sse41 << "\n";
    std::cout << "  SSE4.2: " << sse42 << "\n";
    std::cout << "  AVX   : " << avx << "\n";
    std::cout << "  AVX2  : " << avx2 << "\n";
    std::cout << "  AVX-512F: " << avx512f << "\n";
}

int main() {
    print_simd_support();
    return 0;
}
