import sys
from dataclasses import dataclass
from typing import List, Optional, Tuple

import numpy as np

FILES = 'ABCDEFG'
RANKS = '1234567'
BOARD_SIZE = 7
DIRS = [(1, 0), (-1, 0), (0, 1), (0, -1)]  # E, W, N, S


# ---------------------------------------------------------------------------#
# Pieces
# ---------------------------------------------------------------------------#
@dataclass
class Piece:
    color: str                # 'b' (blue) or 'r' (red)
    kind: str                 # 'guard' or 'tower'
    height: int = 1           # only used for towers

    # ---------- behaviour --------------------------------------------------#
    def can_capture(self, other: 'Piece', moving_height: int) -> bool:
        """Return True if `self` (or the top part of a tower) may capture `other`."""
        if other is None:
            return False
        if self.kind == 'guard':
            return True
        # self is tower
        if other.kind == 'guard':
            return True
        return moving_height >= other.height  # tower vs tower

    def char(self) -> str:
        """ASCII representation (2–3 chars)."""
        if self.kind == 'guard':
            return self.color.upper() + 'G'
        return self.color + str(self.height)


# ---------------------------------------------------------------------------#
# Helper functions
# ---------------------------------------------------------------------------#
def coord_to_xy(coord: str) -> Tuple[int, int]:
    if len(coord) != 2 or coord[0] not in FILES or coord[1] not in RANKS:
        raise ValueError(f'Invalid square: {coord}')
    x = FILES.index(coord[0])
    y = RANKS.index(coord[1])
    return x, y


def xy_to_coord(x: int, y: int) -> str:
    return FILES[x] + RANKS[y]


def parse_move(move: str) -> Tuple[str, str, Optional[int]]:
    """Return (from_square, to_square, n) where n is None = whole stack."""
    parts = move.upper().split('-')
    if len(parts) not in (2, 3):
        raise ValueError('Move must be FROM-TO or FROM-TO-N')
    frm, to = parts[0], parts[1]
    n = int(parts[2]) if len(parts) == 3 else None
    return frm, to, n


# ---------------------------------------------------------------------------#
# Board
# ---------------------------------------------------------------------------#
class Board:
    def __init__(self):
        self.grid: List[List[Optional[Piece]]] = [
            [None for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)
        ]
        self._setup_initial()

    # ---------- setup ------------------------------------------------------#
    def _setup_initial(self):
        # blue side (bottom)
        self.place('D1', Piece('b', 'guard'))
        for sq in ('A1', 'B1', 'F1', 'G1', 'C2', 'E2', 'D3'):
            self.place(sq, Piece('b', 'tower'))
        # red side (top)
        self.place('D7', Piece('r', 'guard'))
        for sq in ('A7', 'B7', 'F7', 'G7', 'C6', 'E6', 'D5'):
            self.place(sq, Piece('r', 'tower'))

    # ---------- low‑level access ------------------------------------------#
    def piece_at(self, coord: str) -> Optional[Piece]:
        x, y = coord_to_xy(coord)
        return self.grid[y][x]

    def place(self, coord: str, piece: Optional[Piece]):
        x, y = coord_to_xy(coord)
        self.grid[y][x] = piece

    # ---------- move execution --------------------------------------------#
    def apply_move(self, player: str, frm: str, to: str, n: Optional[int]) -> None:
        """Validate and execute a move (mutates board). Raises ValueError on error."""
        moving_piece = self.piece_at(frm)
        if moving_piece is None or moving_piece.color != player:
            raise ValueError('No friendly piece on the origin square.')

        dx = FILES.index(to[0]) - FILES.index(frm[0])
        dy = RANKS.index(to[1]) - RANKS.index(frm[1])

        # ---- guard --------------------------------------------------------#
        if moving_piece.kind == 'guard':
            if abs(dx) + abs(dy) != 1:
                raise ValueError('Guard moves exactly one orthogonal square.')
            if n is not None:
                raise ValueError('Number suffix is invalid when moving a guard.')
            dest_piece = self.piece_at(to)
            if dest_piece and dest_piece.color == player:
                raise ValueError('Guard cannot move onto a friendly piece.')
            self.place(frm, None)
            self.place(to, moving_piece)
            return

        # ---- tower --------------------------------------------------------#
        height = moving_piece.height
        move_n = height if n is None else n
        if move_n < 1 or move_n > height:
            raise ValueError('Invalid number of stones to move.')

        if dx != 0 and dy != 0:
            raise ValueError('Towers move orthogonally – no diagonals.')
        distance = abs(dx or dy)
        # direction unit vector
        step_x = 0 if dx == 0 else (1 if dx > 0 else -1)
        step_y = 0 if dy == 0 else (1 if dy > 0 else -1)
        if distance != move_n:
            raise ValueError('Distance must equal number of moving stones.')

        # path clear?
        x0, y0 = coord_to_xy(frm)
        for step in range(1, distance):
            xx = x0 + step_x * step
            yy = y0 + step_y * step
            if self.grid[yy][xx] is not None:
                raise ValueError('Path is blocked.')

        dest_piece = self.piece_at(to)
        moving_top = Piece(player, 'tower', move_n)

        # capture / merge
        if dest_piece:
            if dest_piece.color == player:
                if dest_piece.kind != 'tower':
                    raise ValueError('Cannot merge with your own guard!')
                dest_piece.height += move_n  # merge
            else:
                if not moving_top.can_capture(dest_piece, move_n):
                    raise ValueError('Cannot capture a larger enemy tower.')
                self.place(to, moving_top)  # capture
        else:
            self.place(to, moving_top)  # simple move

        # unstack: leave remainder
        if move_n == height:
            self.place(frm, None)
        else:
            remaining = Piece(player, 'tower', height - move_n)
            self.place(frm, remaining)

    # ---------- move generation -------------------------------------------#
    def generate_moves(self, player: str) -> List[str]:
        """Return a list of all legal moves for `player` in move‑notation."""
        moves: List[str] = []
        for y in range(BOARD_SIZE):
            for x in range(BOARD_SIZE):
                pc = self.grid[y][x]
                if not pc or pc.color != player:
                    continue
                from_sq = xy_to_coord(x, y)

                # ----- guard moves
                if pc.kind == 'guard':
                    for dx, dy in DIRS:
                        xx, yy = x + dx, y + dy
                        if 0 <= xx < BOARD_SIZE and 0 <= yy < BOARD_SIZE:
                            dest_pc = self.grid[yy][xx]
                            if dest_pc is None or dest_pc.color != player:
                                moves.append(f"{from_sq}-{xy_to_coord(xx, yy)}")
                    continue  # guard done

                # ----- tower moves
                height = pc.height
                for n in range(1, height + 1):         # number of stones moved
                    for dx, dy in DIRS:
                        xx, yy = x + dx * n, y + dy * n
                        if not (0 <= xx < BOARD_SIZE and 0 <= yy < BOARD_SIZE):
                            continue

                        # clear path?
                        blocked = False
                        for step in range(1, n):
                            if self.grid[y + dy * step][x + dx * step] is not None:
                                blocked = True
                                break
                        if blocked:
                            continue

                        dest_pc = self.grid[yy][xx]
                        legal = False
                        if dest_pc is None:
                            legal = True
                        elif dest_pc.color == player:
                            legal = dest_pc.kind == 'tower'  # merge with friendly tower
                        else:
                            # capture?
                            temp_top = Piece(player, 'tower', n)
                            legal = temp_top.can_capture(dest_pc, n)

                        if legal:
                            to_sq = xy_to_coord(xx, yy)
                            suffix = '' if n == height else f'-{n}'
                            moves.append(f"{from_sq}-{to_sq}{suffix}")
        return sorted(moves)

    # ---------- utility ----------------------------------------------------#
    def find_guard(self, color: str) -> Optional[str]:
        for y in range(BOARD_SIZE):
            for x in range(BOARD_SIZE):
                pc = self.grid[y][x]
                if pc and pc.color == color and pc.kind == 'guard':
                    return xy_to_coord(x, y)
        return None

    def is_castle(self, coord: str, color: str) -> bool:
        # blue's castle D1, red's D7
        return (coord == 'D7' and color == 'b') or (coord == 'D1' and color == 'r')

    # ---------- printing ---------------------------------------------------#
    def draw(self):
        horizontal = '  +' + '---+' * BOARD_SIZE
        for y in reversed(range(BOARD_SIZE)):
            print(horizontal)
            row_str = f'{RANKS[y]} |'
            for x in range(BOARD_SIZE):
                pc = self.grid[y][x]
                cell = pc.char() if pc else '  '
                row_str += f'{cell:3}|'
            print(row_str)
        print(horizontal)
        print('    ' + '   '.join(FILES))

    def export_fen(self, current_player: str) -> str:
        """Return a FEN-like string of the current position plus whose turn."""
        rows = []
        for y in range(BOARD_SIZE):
            empties = 0
            row_str = ''
            for x in range(BOARD_SIZE):
                pc = self.grid[y][x]
                if pc is None:
                    empties += 1
                else:
                    if empties > 0:
                        row_str += str(empties)
                        empties = 0
                    row_str += pc.char()
            if empties > 0:
                row_str += str(empties)
            rows.append(row_str)
        return '/'.join(rows) + ' ' + current_player



# ---------------------------------------------------------------------------#
# Game Loop
# ---------------------------------------------------------------------------#
def main():
    from evaluation import find_best_move
    board = Board()
    player = 'b'           # blue starts
    opponent = {'b': 'r', 'r': 'b'}

    while True:
        board.draw()
        possible_moves = board.generate_moves(player)
        print(f"\nTurn – {'Blue' if player == 'b' else 'Red'}")
        print("\nPossible moves:")
        print(' '.join(possible_moves) if possible_moves else '(none)')

        best_search_move = find_best_move(board, player)
        print(f'\nBest AI move: {best_search_move}')

        # prompt & parse
        try:
            move = input('\nEnter move or q(iut), r(andom) b(est AI move): ').strip()
        except (EOFError, KeyboardInterrupt):
            print('\nGame aborted.')
            return
        if move.lower() == 'q':
            print('\nGame aborted.')
            return
        if move.lower() == 'r':
            move = np.random.choice(possible_moves)
            print('\nChose random move: {}'.format(move))
        if move.lower() == 'b':
            move = best_search_move
            print('\nChose best search move: {}'.format(move))
        if not move:
            continue

        try:
            frm, to, n = parse_move(move)
            board.apply_move(player, frm, to, n)
        except Exception as exc:
            print(f'Illegal move: {exc}\n')
            continue

        # ----- win check
        opp_guard_pos = board.find_guard(opponent[player])
        if opp_guard_pos is None:
            board.draw()
            print(f'\n{("Blue" if player=="b" else "Red")} wins by capturing the guard!')
            return
        my_guard_pos = board.find_guard(player)
        if my_guard_pos and board.is_castle(my_guard_pos, player):
            board.draw()
            print(f'\n{("Blue" if player=="b" else "Red")} wins by reaching the castle!')
            return

        player = opponent[player]   # next turn
        print()


if __name__ == '__main__':
    main()
