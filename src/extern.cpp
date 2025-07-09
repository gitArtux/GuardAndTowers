// engine_api.cpp
#include "extern.h"
#include "alpha_beta.hpp"    // where fancy_transform is declared
#include <cstring>

extern "C" {

DLL_EXPORT char* make_move_c(const char* in, int factor) {
    // 1) convert to std::string
    std::string input{in};
    // 2) call your C++ logic
    std::string out = make_move(input, factor);
    // 3) allocate a C‐string and copy
    char* cstr = (char*)std::malloc(out.size() + 1);
    std::memcpy(cstr, out.c_str(), out.size() + 1);
    return cstr;
}

DLL_EXPORT void free_string(char* s) {
    std::free(s);
}

} // extern "C"