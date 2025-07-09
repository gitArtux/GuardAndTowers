// dynload_test.cpp
#include <windows.h>
#include <iostream>
#include <cassert>

// function‐pointer types must match your extern "C" signatures
using PFN_FANCY = char* (__cdecl*)(const char*, int);
using PFN_FREE  = void  (__cdecl*)(char*);

int main() {
    // 1) Load the DLL
    HMODULE h = LoadLibraryA("engine.dll");
    if (!h) {
        std::cerr << "LoadLibrary failed: " << GetLastError() << "\n";
        return 1;
    }

    // 2) Lookup the symbols
    auto fancy = reinterpret_cast<PFN_FANCY>(GetProcAddress(h, "make_move_c"));
    auto freer = reinterpret_cast<PFN_FREE >(GetProcAddress(h, "free_string"));
    assert(fancy && freer && "Could not find exported functions");

    // 3) Call and verify
    char* out = fancy("r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r", 3);
    std::cout << "make_move_c → " << out << "\n";
    freer(out);

    // 4) Unload
    FreeLibrary(h);
    return 0;
}
