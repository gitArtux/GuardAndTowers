import os, ctypes
from ctypes import c_char_p, c_int

dll_path = r"C:\Users\Arthur\Documents\Studium\PSKI\GuardAndTowers\engine.dll"

# 1) Let Windows also look in the MSVC redist folder:
os.add_dll_directory(
    r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
    r"\VC\Redist\MSVC\14.44.35112\x64"
)

# 2) (Still) ensure your own DLL’s folder is on the search path:
os.add_dll_directory(os.path.dirname(dll_path))

# 3) Now load it:
lib = ctypes.CDLL(dll_path)
print("Loaded OK!")

# Set the return type of make_move_c to c_char_p
lib.make_move_c.restype = c_char_p

fen = c_char_p(b"r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r")
result_ptr = lib.make_move_c(fen, c_int(3))
result_str = result_ptr.decode('utf-8')
print("Result:", result_str)
lib.free_string(result_ptr)
print("Fen string freed.")