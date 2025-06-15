import sys
from dataclasses import dataclass
from typing import List, Optional, Tuple
import random
import copy
import numpy as np

from colorama import init, Fore, Back, Style
init(autoreset=True)

FILES = 'ABCDEFG'
RANKS = '1234567'
BOARD_SIZE = 7
DIRS = [(1, 0), (-1, 0), (0, 1), (0, -1)]  # E, W, N, S


random.seed(42)  # Optional: seed for reproducibility


ZOBRIST_TABLE = {
    (x, y, color, kind, height): random.getrandbits(64)
    for x in range(7)
    for y in range(7)
    for color in ('b', 'r')
    for kind in ('guard', 'tower')
    for height in range(1, 8)
}

# Zobrist side-to-move bit
ZOBRIST_SIDE = random.getrandbits(64)

# ---------------------------------------------------------------------------#
# Bit-board helpers  ❱❱  NEW
# ---------------------------------------------------------------------------#
NUM_SQ  = BOARD_SIZE * BOARD_SIZE          # 49

def idx(x: int, y: int) -> int:           # 0-based square index
    return y * BOARD_SIZE + x             # 0..48  (A1 = 0, B1 = 1 …)
def bit(x: int, y: int) -> int:
    return 1 << idx(x, y)
def idx_to_xy(i: int) -> tuple[int, int]:
    return i % BOARD_SIZE, i // BOARD_SIZE

# Map each direction to its index for ray-mask lookup
DIR_IDX = {DIRS[i]: i for i in range(len(DIRS))}

# Precompute ray masks: for each square and each direction, mask of intermediate squares for each distance
RAY_MASK = [[[0] * (BOARD_SIZE - 1) for _ in DIRS] for _ in range(NUM_SQ)]
for sq in range(NUM_SQ):
    x0, y0 = idx_to_xy(sq)
    for d, (dx, dy) in enumerate(DIRS):
        for dist in range(1, BOARD_SIZE):
            mask = 0
            for step in range(1, dist):
                x1 = x0 + dx * step
                y1 = y0 + dy * step
                if 0 <= x1 < BOARD_SIZE and 0 <= y1 < BOARD_SIZE:
                    mask |= bit(x1, y1)
            RAY_MASK[sq][d][dist - 1] = mask




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
                # ---- bit-boards ---------------------------------------------------#
        self.occ   = {'b': 0, 'r': 0}          # all blue / red pieces
        self.guard = {'b': 0, 'r': 0}          # the single guards
        # 7 tower-height layers per colour (index 0 ⇒ height 1 … index 6 ⇒ height 7)
        self.tower = {'b': [0]*7, 'r': [0]*7}

        self.grid: list[list[Optional[Piece]]] = [
            [None for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)
        ]
        self._setup_initial()

        # Zobrist -----------------------------------------------------------#
        self.zobrist = 0
        for y in range(BOARD_SIZE):
            for x in range(BOARD_SIZE):
                p = self.grid[y][x]
                if not p: continue
                h = 1 if p.kind == 'guard' else p.height
                self.zobrist ^= ZOBRIST_TABLE[(x, y, p.color, p.kind, h)]
        self.side_to_move = 'b'
        self.zobrist ^= ZOBRIST_SIDE

        self.move_stack = []
        self.last_move  = None


    def copy(self) -> 'Board':
        return copy.deepcopy(self)

    # ---------- setup ------------------------------------------------------#
    def _setup_initial(self):
        def _put(sq: str, piece: Piece):
            x, y = coord_to_xy(sq)
            self.grid[y][x] = piece
            b = bit(x, y)
            self.occ[piece.color] |= b
            if piece.kind == 'guard':
                self.guard[piece.color] = b
            else:
                self.tower[piece.color][piece.height-1] |= b

        # ---- blue ---------------------------------------------------------#
        _put('D1', Piece('b', 'guard'))
        for sq in ('A1','B1','F1','G1','C2','E2','D3'):
            _put(sq, Piece('b', 'tower'))

        # ---- red ----------------------------------------------------------#
        _put('D7', Piece('r', 'guard'))
        for sq in ('A7','B7','F7','G7','C6','E6','D5'):
            _put(sq, Piece('r', 'tower'))


    # ---------- low‑level access ------------------------------------------#
    def piece_at(self, coord: str) -> Optional[Piece]:
        x, y = coord_to_xy(coord)
        return self.grid[y][x]

    def place(self, coord: str, piece: Optional[Piece]):
        x, y   = coord_to_xy(coord)
        b      = bit(x, y)
        prev   = self.grid[y][x]

        # ---- remove previous ---------------------------------------------#
        if prev:
            self.occ[prev.color] &= ~b
            if prev.kind == 'guard':
                self.guard[prev.color] = 0
            else:
                self.tower[prev.color][prev.height-1] &= ~b

        # ---- add new ------------------------------------------------------#
        self.grid[y][x] = piece
        if piece:
            self.occ[piece.color] |= b
            if piece.kind == 'guard':
                self.guard[piece.color] = b
            else:
                self.tower[piece.color][piece.height-1] |= b


    # ---------- move execution --------------------------------------------#
    def apply_move(self, player: str, frm: str, to: str, n: Optional[int]) -> None:
        """Validate and execute a move (mutates board). Raises ValueError on error."""
        # Record original pieces and remove from hash
        x_from, y_from = coord_to_xy(frm)
        x_to, y_to = coord_to_xy(to)
        orig_from = self.piece_at(frm)
        orig_from_copy = Piece(orig_from.color, orig_from.kind, orig_from.height) if orig_from else None
        orig_dest = self.piece_at(to)
        orig_dest_copy = Piece(orig_dest.color, orig_dest.kind, orig_dest.height) if orig_dest else None
        self.move_stack.append((frm, to, orig_from_copy, orig_dest_copy))
        if orig_from_copy:
            self.zobrist ^= ZOBRIST_TABLE[(x_from, y_from, orig_from_copy.color, orig_from_copy.kind, orig_from_copy.height)]
        if orig_dest_copy:
            self.zobrist ^= ZOBRIST_TABLE[(x_to, y_to, orig_dest_copy.color, orig_dest_copy.kind, orig_dest_copy.height)]

        moving_piece = self.piece_at(frm)
        if moving_piece is None or moving_piece.color != player:
            raise ValueError('No friendly piece on the origin square.')

        dx = FILES.index(to[0]) - FILES.index(frm[0])
        dy = RANKS.index(to[1]) - RANKS.index(frm[1])

        # ---- guard --------------------------------------------------------#
        if moving_piece.kind == 'guard':
            if abs(dx) + abs(dy) != 1:
                raise ValueError('Guard moves exactly one orthogonal square.')
            dest_piece = self.piece_at(to)
            if dest_piece and dest_piece.color == player:
                raise ValueError('Guard cannot move onto a friendly piece.')
            self.place(frm, None)
            self.place(to, moving_piece)
            # Add new guard position to hash
            new_dest = self.piece_at(to)
            height_dest = 1  # guard height
            self.zobrist ^= ZOBRIST_TABLE[(x_to, y_to, new_dest.color, new_dest.kind, height_dest)]
            # Flip side-to-move and update hash
            self.side_to_move = 'r' if self.side_to_move == 'b' else 'b'
            self.zobrist ^= ZOBRIST_SIDE
            # Record last move for highlighting
            self.last_move = (frm, to)
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

        # Add new pieces to hash
        new_dest = self.piece_at(to)
        height_dest = new_dest.height
        self.zobrist ^= ZOBRIST_TABLE[(x_to, y_to, new_dest.color, new_dest.kind, height_dest)]
        new_from = self.piece_at(frm)
        if new_from:
            height_from = new_from.height
            self.zobrist ^= ZOBRIST_TABLE[(x_from, y_from, new_from.color, new_from.kind, height_from)]
        # Flip side-to-move and update hash
        self.side_to_move = 'r' if self.side_to_move == 'b' else 'b'
        self.zobrist ^= ZOBRIST_SIDE
        # Record last move for highlighting
        self.last_move = (frm, to)

    def unapply_move(self):
        """
        Undo the last move using the move_stack and update the zobrist hash.
        """
        frm, to, orig_from, orig_dest = self.move_stack.pop()
        x_from, y_from = coord_to_xy(frm)
        x_to, y_to = coord_to_xy(to)
        # Remove current pieces from hash
        curr_from = self.piece_at(frm)
        if curr_from:
            h = curr_from.height if curr_from.kind == 'tower' else 1
            self.zobrist ^= ZOBRIST_TABLE[(x_from, y_from, curr_from.color, curr_from.kind, h)]
        curr_dest = self.piece_at(to)
        if curr_dest:
            h = curr_dest.height if curr_dest.kind == 'tower' else 1
            self.zobrist ^= ZOBRIST_TABLE[(x_to, y_to, curr_dest.color, curr_dest.kind, h)]
        # Restore original pieces
        self.place(frm, orig_from)
        self.place(to, orig_dest)
        # Add original pieces back to hash
        if orig_from:
            h = orig_from.height if orig_from.kind == 'tower' else 1
            self.zobrist ^= ZOBRIST_TABLE[(x_from, y_from, orig_from.color, orig_from.kind, h)]
        if orig_dest:
            h = orig_dest.height if orig_dest.kind == 'tower' else 1
            self.zobrist ^= ZOBRIST_TABLE[(x_to, y_to, orig_dest.color, orig_dest.kind, h)]
        # Flip side-to-move and update hash
        self.side_to_move = 'r' if self.side_to_move == 'b' else 'b'
        self.zobrist ^= ZOBRIST_SIDE

    def apply_null_move(self):
        """Perform a null move: flip side-to-move and update hash."""
        self.side_to_move = 'r' if self.side_to_move == 'b' else 'b'
        self.zobrist ^= ZOBRIST_SIDE

    def unapply_null_move(self):
        """Undo a null move: flip side-to-move and update hash."""
        self.zobrist ^= ZOBRIST_SIDE
        self.side_to_move = 'r' if self.side_to_move == 'b' else 'b'

    # ---------- move generation -------------------------------------------#
    def generate_moves(self, player: str) -> list[str]:
        moves = []
        opp   = 'r' if player == 'b' else 'b'
        occ   = self.occ['b'] | self.occ['r']

        # -------- guards ---------------------------------------------------#
        gbit = self.guard[player]
        if gbit:
            i      = (gbit & -gbit).bit_length() - 1
            gx, gy = idx_to_xy(i)
            for dx, dy in DIRS:
                nx, ny = gx+dx, gy+dy
                if 0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE:
                    if not (self.occ[player] & bit(nx, ny)):
                        moves.append(f"{xy_to_coord(gx,gy)}-{xy_to_coord(nx,ny)}-1")

        # -------- towers ---------------------------------------------------#
        towers_all = 0
        heights_by_sq = {}
        for h in range(7):
            layer = self.tower[player][h]
            towers_all |= layer
            bb = layer
            while bb:
                ls1 = bb & -bb
                sq  = (ls1.bit_length()-1)
                heights_by_sq[sq] = h+1   # actual height
                bb ^= ls1

        while towers_all:
            ls1 = towers_all & -towers_all
            sq  = (ls1.bit_length()-1)
            x, y = idx_to_xy(sq)
            ht    = heights_by_sq[sq]
            from_sq = xy_to_coord(x, y)

            for take in range(1, ht+1):
                dist = take
                for dx, dy in DIRS:
                    nx, ny = x + dx*dist, y + dy*dist
                    if not (0 <= nx < BOARD_SIZE and 0 <= ny < BOARD_SIZE):
                        continue
                    # path clear via precomputed ray mask
                    dir_idx = DIR_IDX[(dx, dy)]
                    if occ & RAY_MASK[sq][dir_idx][take - 1]:
                        continue
                    dest_piece = self.piece_at(xy_to_coord(nx, ny))
                    legal = False
                    if dest_piece is None:
                        legal = True
                    elif dest_piece.color == player:
                        legal = dest_piece.kind == 'tower'
                    else:  # capture?
                        if dest_piece.kind == 'guard':
                            legal = True
                        elif take >= dest_piece.height:
                            legal = True
                    if legal:
                        to_sq = xy_to_coord(nx, ny)
                        moves.append(f"{from_sq}-{to_sq}-{take}")
            towers_all ^= ls1
        return moves


    # ---------- utility ----------------------------------------------------#
    def find_guard(self, color: str) -> Optional[str]:
        b = self.guard[color]
        if not b:
            return None
        i = (b & -b).bit_length() - 1
        x, y = idx_to_xy(i)
        return xy_to_coord(x, y)

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
                coord = xy_to_coord(x, y)
                pc = self.grid[y][x]
                # Determine base cell content (2 visible chars)
                if pc:
                    char = pc.char()
                    base = Fore.BLUE + char + Style.RESET_ALL if pc.color == 'b' else Fore.RED + char + Style.RESET_ALL
                else:
                    base = '  '
                # Highlight from/to squares
                if self.last_move and coord in self.last_move:
                    cell = Back.YELLOW + base + Style.RESET_ALL
                else:
                    cell = base
                # Add cell with one space padding each side
                row_str += f' {cell}|'
            print(row_str)
        print(horizontal)
        print('   ' + '   '.join(FILES))

    def export_fen(self, current_player: str) -> str:
        """Return a FEN-like string of the current position plus whose turn."""
        rows = []
        for y in reversed(range(BOARD_SIZE)):
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
    
    def zobrist_hash(self) -> int:
        return self.zobrist
    

def fen_to_board(fen: str) -> Tuple[Board, str]:
    rows, player = fen.split()
    rows = rows.split('/')
    board = Board()
    # clear any existing pieces
    for y in range(BOARD_SIZE):
        for x in range(BOARD_SIZE):
            board.grid[y][x] = None
    for rank_idx, row in enumerate(rows):
        file_idx = 0
        i = 0
        while i < len(row):
            c = row[i]
            if c.isdigit():
                file_idx += int(c)
                i += 1
            else:
                code = row[i:i+2]
                if code[1] == 'G':
                    color = code[0].lower()
                    piece = Piece(color, 'guard')
                else:
                    height = int(code[1])
                    color = code[0]
                    piece = Piece(color, 'tower', height)
                y = BOARD_SIZE - 1 - rank_idx
                board.grid[y][file_idx] = piece
                file_idx += 1
                i += 2
    return board, player




# ---------------------------------------------------------------------------#
# Game Loop
# ---------------------------------------------------------------------------#
def main():
    from evaluation import find_best_move
    board = Board()
    player = 'r'           # red starts
    opponent = {'b': 'r', 'r': 'b'}

    while True:
        board.draw()
        possible_moves = board.generate_moves(player)
        turn_color = Fore.BLUE if player == 'b' else Fore.RED
        turn_name = 'Blue' if player == 'b' else 'Red'
        print(f"\nTurn – {turn_color}{turn_name}{Style.RESET_ALL}")
        print(f"Board FEN: {board.export_fen(player)}")
        print("\nPossible moves:")
        print(' '.join(possible_moves) if possible_moves else '(none)')

        best_search_move = find_best_move(copy.deepcopy(board), player, base_depth=7)
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
