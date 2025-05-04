import copy

from guard_towers import Board, parse_move

def copy_board(board: Board) -> Board:
    return copy.deepcopy(board)


def evaluate(board: Board, player: str) -> int:
    opponent = 'r' if player == 'b' else 'b'

    # Win/loss check
    if board.find_guard(opponent) is None:
        return 10000  # win
    if board.find_guard(player) is None:
        return -10000  # loss

    score = 0
    # Bonus for tower material
    for row in board.grid:
        for piece in row:
            if piece:
                value = piece.height if piece.kind == 'tower' else 0
                if piece.color == player:
                    score += value
                else:
                    score -= value

    return score


def alphabeta(board: Board, depth: int, alpha: int, beta: int, maximizing: bool, player: str) -> int:
    opponent = 'r' if player == 'b' else 'b'

    if depth == 0 or board.find_guard(player) is None or board.find_guard(opponent) is None:
        return evaluate(board, player)

    moves = board.generate_moves(player if maximizing else opponent)
    if not moves:
        return evaluate(board, player)

    if maximizing:
        max_eval = -float('inf')
        for move in moves:
            new_board = copy_board(board)
            try:
                frm, to, n = parse_move(move)
                new_board.apply_move(player, frm, to, n)
                eval = alphabeta(new_board, depth - 1, alpha, beta, False, player)
                max_eval = max(max_eval, eval)
                alpha = max(alpha, eval)
                if beta <= alpha:
                    break
            except:
                continue
        return max_eval
    else:
        min_eval = float('inf')
        for move in moves:
            new_board = copy_board(board)
            try:
                frm, to, n = parse_move(move)
                new_board.apply_move(opponent, frm, to, n)
                eval = alphabeta(new_board, depth - 1, alpha, beta, True, player)
                min_eval = min(min_eval, eval)
                beta = min(beta, eval)
                if beta <= alpha:
                    break
            except:
                continue
        return min_eval


def find_best_move(board: Board, player: str, depth: int = 3) -> str:
    best_value = -float('inf')
    best_move = None
    for move in board.generate_moves(player):
        new_board = copy_board(board)
        try:
            frm, to, n = parse_move(move)
            new_board.apply_move(player, frm, to, n)
            move_value = alphabeta(new_board, depth - 1, -float('inf'), float('inf'), False, player)
            if move_value > best_value:
                best_value = move_value
                best_move = move
        except:
            continue
    return best_move
