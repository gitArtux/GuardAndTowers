
"""
Guard & Towers – terminal edition
Author: ChatGPT
The program implements the complete rule‑set described in the PDF (© 1997 Christoph Endres & Robert Wirth).

How to play
-----------
Run     python guard_towers.py

The board is printed on every turn.  
Blue (b) moves first by convention, but you can easily flip that in `main()`.

Enter moves in the form

    FROM-TO            (for a guard or for moving a *whole* tower)
    FROM-TO-N          (to move N stones off the *top* of a taller tower)

Examples:
    D1-D2              – move blue guard from D1 to D2
    A7-A6-1            – move 1 soldier from the red tower at A7 to A6
    F7-D7              – red guard captures blue guard at D7 (winning)

Coordinates use   A‑G  for files (columns) from left to right,  
and              1‑7  for ranks (rows) from bottom to top.

The game ends immediately when a guard is captured or when a guard occupies
the opposing castle square (D1 for red, D7 for blue).

Enjoy!
"""

import sys
from dataclasses import dataclass
from typing import List, Optional, Tuple

FILES = 'ABCDEFG'
RANKS = '1234567'
BOARD_SIZE = 7

# --- pieces -----------------------------------------------------------------
@dataclass
class Piece:
    color: str                # 'b' (blue) or 'r' (red)
    kind: str                 # 'guard' or 'tower'
    height: int = 1           # only used for towers

    def can_capture(self, other: 'Piece', moving_height: int) -> bool:
        if other is None:
            return False
        if self.kind == 'guard':
            return True                   # guard captures anything
        # self is tower
        if other.kind == 'guard':
            return True                   # any tower captures a guard
        return moving_height >= other.height  # tower vs tower

    def char(self) -> str:
        if self.kind == 'guard':
            return self.color.upper() + 'G'
        return self.color + str(self.height)


# --- helpers ----------------------------------------------------------------
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


# --- board ------------------------------------------------------------------
class Board:
    def __init__(self):
        self.grid: List[List[Optional[Piece]]] = [
            [None for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)
        ]
        self._setup_initial()

    # initial array – symmetrical:
    #  towers : A,B,F,G on baseline + C,E on 2nd rank + D on 3rd rank
    def _setup_initial(self):
        # blue side (bottom)
        self.place('D1', Piece('b', 'guard'))
        for sq in ('A1', 'B1', 'F1', 'G1', 'C2', 'E2', 'D3'):
            self.place(sq, Piece('b', 'tower'))
        # red side (top)
        self.place('D7', Piece('r', 'guard'))
        for sq in ('A7', 'B7', 'F7', 'G7', 'C6', 'E6', 'D5'):
            self.place(sq, Piece('r', 'tower'))

    # -- low‑level -----------------------------------------------------------
    def piece_at(self, coord: str) -> Optional[Piece]:
        x, y = coord_to_xy(coord)
        return self.grid[y][x]

    def place(self, coord: str, piece: Optional[Piece]):
        x, y = coord_to_xy(coord)
        self.grid[y][x] = piece

    # -- logic ---------------------------------------------------------------
    def apply_move(self, player: str, frm: str, to: str, n: Optional[int]) -> str:
        """Validate and execute a move. Return '' or a win/lose message."""
        moving_piece = self.piece_at(frm)
        if moving_piece is None or moving_piece.color != player:
            raise ValueError('No friendly piece on the origin square.')

        dx = FILES.index(to[0]) - FILES.index(frm[0])
        dy = RANKS.index(to[1]) - RANKS.index(frm[1])

        # --- guard ----------------------------------------------------------
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
            # capture
            if dest_piece:
                # dest_piece is captured
                pass
            return ''

        # --- tower ----------------------------------------------------------
        height = moving_piece.height
        move_n = height if n is None else n
        if move_n < 1 or move_n > height:
            raise ValueError('Invalid number of stones to move.')

        # direction must be orthogonal & distance == move_n
        if dx != 0 and dy != 0:
            raise ValueError('Towers move orthogonally – no diagonals.')
        distance = abs(dx or dy)
        direction = (dx and (1 if dx > 0 else -1), dy and (1 if dy > 0 else -1))
        if distance != move_n:
            raise ValueError('Distance must equal number of moving stones.')

        # path clear?
        step_x, step_y = direction
        x0, y0 = coord_to_xy(frm)
        for step in range(1, distance):
            xx = x0 + step_x * step
            yy = y0 + step_y * step
            if self.grid[yy][xx] is not None:
                raise ValueError('Path is blocked.')

        dest_piece = self.piece_at(to)
        moving_top = Piece(player, 'tower', move_n)

        # capture / merge rules
        if dest_piece:
            if dest_piece.color == player:
                if dest_piece.kind != 'tower':
                    raise ValueError('Cannot merge with your own guard!')
                # merge
                dest_piece.height += move_n
            else:
                if not moving_top.can_capture(dest_piece, move_n):
                    raise ValueError('Cannot capture a larger enemy tower.')
                # capture
                self.place(to, moving_top if dest_piece.kind != 'guard' else moving_top)
        else:
            # simple move
            self.place(to, moving_top)

        # leave behind (unstacking)
        if move_n == height:
            self.place(frm, None)
        else:
            remaining = Piece(player, 'tower', height - move_n)
            self.place(frm, remaining)

        return ''

    # -- utility -------------------------------------------------------------
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

    # --- printing -----------------------------------------------------------
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


# --- game loop --------------------------------------------------------------
def main():
    board = Board()
    player = 'b'           # blue starts
    opponent = {'b': 'r', 'r': 'b'}

    while True:
        board.draw()
        print(f"\nTurn – {'Blue' if player=='b' else 'Red'}")

        # prompt & parse
        try:
            move = input('Enter move: ').strip()
        except (EOFError, KeyboardInterrupt):
            print('\nGame aborted.')
            return
        if not move:
            continue
        if move.lower() == 'q':
            print('\nGame aborted.')
            return
        try:
            frm, to, n = parse_move(move)
            msg = board.apply_move(player, frm, to, n)
        except Exception as exc:
            print(f'Illegal move: {exc}\n')
            continue

        # win check
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

        player = opponent[player]   # switch turn
        print()

if __name__ == '__main__':
    main()
