import unittest

# Import the implementation under test -------------------------------
# Adjust the import below if the file is in a package.
import guard_towers as gt

from typing import Tuple, Dict, List, Union

# Test cases for generate_moves: map FEN positions to expected move lists
# FEN strings go from the top of the board to the bottom, and from left to right. As per server
TEST_CASES_GENERATE_MOVES: Dict[str, Union[str, List[str]]] = {
    '3RG3/7/7/3b13/7/7/3BG3 b': 'D1-C1-1 D1-D2-1 D1-E1-1 D4-C4-1 D4-D3-1 D4-D5-1 D4-E4-1',
    '3RG3/7/7/7/7/7/3BG3 r': 'D7-C7-1 D7-D6-1 D7-E7-1',
    'r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r': 'A7-A6-1 A7-B7-1 B7-A7-1 B7-B6-1 B7-C7-1 C6-B6-1 C6-C5-1 C6-C7-1 C6-D6-1 D5-C5-1 D5-D4-1 D5-D6-1 D5-E5-1 D7-C7-1 D7-D6-1 D7-E7-1 E6-D6-1 E6-E5-1 E6-E7-1 E6-F6-1 F7-E7-1 F7-F6-1 F7-G7-1 G7-F7-1 G7-G6-1',
}

class TestGenerateMoves(unittest.TestCase):
    def test_generate_moves(self):
        for fen, expected in TEST_CASES_GENERATE_MOVES.items():
            board, player = gt.fen_to_board(fen)
            moves = board.generate_moves(player)
            # If expected is a string, split into list; otherwise assume list
            expected_list = expected.split() if isinstance(expected, str) else expected
            self.assertEqual(
                sorted(moves), sorted(expected_list),
                f"Failed for FEN {fen}, got {moves}"
            )


# ----------- helpers -------------------------------------------------
def empty_board() -> gt.Board:
    """Return a Board with no pieces on it."""
    b = gt.Board()
    for y in range(gt.BOARD_SIZE):
        for x in range(gt.BOARD_SIZE):
            b.grid[y][x] = None
    return b


def tower(color: str, height: int) -> gt.Piece:
    return gt.Piece(color, "tower", height)


def guard(color: str) -> gt.Piece:
    return gt.Piece(color, "guard")


# ----------- test‑cases ---------------------------------------------
class TestGuardsAndTowers(unittest.TestCase):
    # --- guard moves --------------------------------------------------
    def test_guard_move_valid(self):
        board = gt.Board()
        board.apply_move("b", "D1", "D2", None)  # north 1
        self.assertIsNone(board.piece_at("D1"))
        pc = board.piece_at("D2")
        self.assertIsNotNone(pc)
        self.assertEqual(pc.kind, "guard")
        self.assertEqual(pc.color, "b")

    def test_guard_move_invalid_diagonal(self):
        board = gt.Board()
        with self.assertRaises(ValueError):
            board.apply_move("b", "D1", "E2", None)

    # --- tower movement ----------------------------------------------
    def test_tower_move_valid_distance_equals_height(self):
        board = gt.Board()
        board.apply_move("b", "C2", "C3", None)  # height‑1 tower, 1 step
        self.assertIsNone(board.piece_at("C2"))
        self.assertIsNotNone(board.piece_at("C3"))

    def test_tower_move_invalid_distance_not_equal_height(self):
        board = gt.Board()
        with self.assertRaises(ValueError):
            board.apply_move("b", "C2", "C4", None)  # wants to go 2 squares with height‑1

    # --- tower stacking ----------------------------------------------
    def test_merge_friendly_towers(self):
        board = gt.Board()
        board.apply_move("b", "A1", "B1", None)           # merge A1→B1
        pc = board.piece_at("B1")
        self.assertEqual(pc.kind, "tower")
        self.assertEqual(pc.height, 2)
        self.assertIsNone(board.piece_at("A1"))

    # --- captures -----------------------------------------------------
    def test_capture_enemy_tower(self):
        board = empty_board()
        board.place("A1", tower("b", 3))
        board.place("A4", tower("r", 2))
        board.apply_move("b", "A1", "A4", None)           # capture
        pc = board.piece_at("A4")
        self.assertEqual(pc.color, "b")
        self.assertEqual(pc.height, 3)

    def test_illegal_capture_larger_enemy_tower(self):
        board = empty_board()
        board.place("A1", tower("b", 2))
        board.place("A3", tower("r", 3))
        with self.assertRaises(ValueError):
            board.apply_move("b", "A1", "A3", None)

    def test_capture_guard_win(self):
        board = empty_board()
        board.place("A1", tower("b", 2))
        board.place("A3", guard("r"))
        board.apply_move("b", "A1", "A3", None)
        self.assertIsNone(board.find_guard("r"))

    # --- reaching the castle -----------------------------------------
    def test_reach_castle_win(self):
        board = empty_board()
        board.place("D6", guard("b"))
        board.apply_move("b", "D6", "D7", None)
        self.assertTrue(board.is_castle("D7", "b"))


# --------------------------------------------------------------------
if __name__ == "__main__":
    unittest.main(verbosity=2)
