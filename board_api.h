#pragma once

#ifdef _WIN32
  #define BOARD_API __declspec(dllexport)
#else
  #define BOARD_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// BOARD_API int multiply(int a, int b);


// INIT/RESET BOARD () -> ()
// RESET GUI in GUI logic and call init_board() in C++ logic

// MOVE (to, from/FEN) -> (bool allowed, bool_game over)
// FEN or Bitboard in args
// Adjust GUI if allowed == true
// if bool_game_over do something

/*
OPTIONAL--------------------

UNDO

AI_MOVE

RAND_MOVE
*/

#ifdef __cplusplus
}
#endif