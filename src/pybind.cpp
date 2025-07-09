#include <pybind11/pybind11.h>
#include <string>

std::string compute_best_move(const std::string& fen_pos, int depth);

namespace py = pybind11;

PYBIND11_MODULE(libboard, m) {
    m.def("compute_best_move", &compute_best_move, "Compute the best move from a FEN string and depth");
}