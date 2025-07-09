#pragma once
#include <cstdint>
#include <cstdlib>

#ifdef _WIN32
  #define DLL_EXPORT __declspec(dllexport)
#else
  #define DLL_EXPORT
#endif

extern "C" {
  // Wraps std::string fancy_transform(const std::string&, int)
  // Returns a malloc’d C‐string; caller must free().
  DLL_EXPORT char* make_move_c(const char* in, int factor);

  // Frees strings returned above
  DLL_EXPORT void free_string(char* s);
}