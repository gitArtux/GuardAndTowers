#include <pybind11/pybind11.h>
#include <string>

std::string say_hello(const std::string& name) {
    return "Hello from C++! My name is " + name;
}

namespace py = pybind11;

PYBIND11_MODULE(libboard, m) {
    m.def("say_hello", &say_hello, "A function that greets the user");
}
// Compilation command (for python3.11):
// c++ -O3 -Wall -shared -std=c++17 -fPIC \
// $(python3.11 -m pybind11 --includes) pybind.cpp \
// -undefined dynamic_lookup \
// -o libboard$(python3.11-config --extension-suffix)