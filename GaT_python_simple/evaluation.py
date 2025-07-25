import copy
import random
from typing import Optional, Dict
from mcts import MCTSNode
from guard_towers import Board, parse_move

from collections import defaultdict

# one Counter per ID iteration; keys = ply (0 = root), values = moves generated
depth_move_counters: dict[int, defaultdict[int, int]] = {}


TTEntry = Dict[str, any]
TranspositionTable = Dict[int, TTEntry]
PVTable = Dict[int, str]

transposition_table: TranspositionTable = {}
pv_table: PVTable = {}

def copy_board(board: Board) -> Board:
    return copy.deepcopy(board)


def evaluate(board: Board, player: str) -> int:
    """
    Evaluate the given board state from the perspective of the specified player.

    The evaluation considers:
    - A large positive score if the opponent's guard is captured (win).
    - A large negative score if the player's guard is captured (loss).
    - The material value of towers on the board, with bonuses for certain stack heights and centralization.
    """
    HEIGHT_BONUS = {
        2: 3,
        4: 5
    }
    opponent = 'r' if player == 'b' else 'b'
    if board.find_guard(opponent) is None:
        return 10000
    if board.find_guard(player) is None:
        return -10000

    score = 0
    for row in board.grid:
        for piece in row:
            if piece:
                value = piece.height if piece.kind == 'tower' else 0
                if piece.color == player:
                    score += value
                    score += HEIGHT_BONUS.get(piece.height, 0)
                else:
                    score -= value
    return score


# Helper: adaptive search depth based on move count
def _adaptive_depth(board: Board, player: str, base_depth: int = 5) -> int:
    """
    Pick a search depth that grows when the position is simpler.

    Heuristic:
    - Many legal moves  ( > 25 )      → keep base depth.
    - Medium legal moves (16–25)      → +1 ply.
    - Few legal moves   ( 8–15)       → +2 plies.
    - Very few moves    ( ≤ 7)        → +3 plies.

    This is cheap (just counts moves) and tracks the real branching factor,
    which is what makes deeper search affordable once material comes off.
    """
    num_moves = len(board.generate_moves(player))
    if num_moves <= 7:
        return base_depth + 3
    if num_moves <= 15:
        return base_depth + 2
    if num_moves <= 25:
        return base_depth + 1
    return base_depth


def alphabeta(board: Board, depth: int, alpha: int, beta: int, maximizing: bool, player: str,ply: int = 0, iter_ID: int = 1) -> int:
    """
    Perform the alpha-beta pruning minimax search to evaluate the best achievable score from the current board state.

    Args:
        board (Board): The current game board state.
        depth (int): The maximum search depth.
        alpha (int): The alpha value for pruning (best already explored option along the path to the maximizer).
        beta (int): The beta value for pruning (best already explored option along the path to the minimizer).
        maximizing (bool): True if the current move is for the maximizing player, False otherwise.
        player (str): The player color ('b' or 'r') for whom the evaluation is done.

    Returns:
        int: The evaluated score of the board state from the perspective of the player.
    """

    opponent = 'r' if player == 'b' else 'b'
    side_to_move = player if maximizing else opponent
    moves = board.generate_moves(side_to_move)
    depth_move_counters[iter_ID][ply] += len(moves)

    # Preserve the original alpha–beta window so we can label the TT entry correctly
    alpha_orig = alpha
    beta_orig  = beta

    def move_sort_key(m):
        if m == pv_move:
            return float('inf')
        frm, to, n = parse_move(m)
        dest = board.piece_at(to)
        if dest:
            if dest.kind == 'guard':
                return 1000
            return dest.height
        return 0

    zobrist = board.zobrist_hash()
    entry = transposition_table.get(zobrist)

    # Use TT if valid
    if entry and entry['depth'] >= depth:
        if entry['type'] == 'exact':
            return entry['score']
        elif entry['type'] == 'alpha' and entry['score'] <= alpha:
            return alpha
        elif entry['type'] == 'beta' and entry['score'] >= beta:
            return beta

    # Terminal: guard captured?
    if board.find_guard(opponent) is None or board.find_guard(player) is None:
        return evaluate(board, player)
    # Depth zero → quiescence
    if depth == 0:
        return quiescence(board, alpha, beta, player)

    # Null‑move pruning (depth reduction R = 2)
    if depth > 2:
        board.apply_null_move()
        score_nm = -alphabeta(board, depth - 1 - 2, -beta, -beta + 1, not maximizing, player, ply + 1, iter_ID)
        board.unapply_null_move()
        if score_nm >= beta:
            return beta

    # Move ordering: PV-first then MVV-LVA
    zob = board.zobrist_hash()
    tt_ent = transposition_table.get(zob, {})
    pv_move = tt_ent.get('best_move')
    best_move = None
    if maximizing:
        max_eval = -float('inf')
        moves = board.generate_moves(player)  # same side as root when maximizing branch
        moves.sort(key=move_sort_key, reverse=True)
        for move in moves:
            try:
                frm, to, n = parse_move(move)
                board.apply_move(player, frm, to, n)
            except Exception:
                continue
            try:
                eval = alphabeta(board, depth - 1, alpha, beta, False, player, ply + 1, iter_ID)
            finally:
                board.unapply_move()
            if eval > max_eval:
                max_eval = eval
                best_move = move
            alpha = max(alpha, eval)
            if beta <= alpha:
                break
        score = max_eval
    else:
        min_eval = float('inf')
        moves.sort(key=move_sort_key, reverse=True)
        for move in moves:
            try:
                frm, to, n = parse_move(move)
                board.apply_move(opponent, frm, to, n)
            except Exception:
                continue
            try:
                eval = alphabeta(board, depth - 1, alpha, beta, True, player, ply + 1, iter_ID)
            finally:
                board.unapply_move()
            if eval < min_eval:
                min_eval = eval
                best_move = move
            beta = min(beta, eval)
            if beta <= alpha:
                break
        score = min_eval

    # Store in TT with replacement policy
    node_type = 'exact'
    if score <= alpha_orig:
        node_type = 'alpha'
    elif score >= beta_orig:
        node_type = 'beta'
    old = transposition_table.get(zobrist)
    if not old or depth >= old['depth']:
        transposition_table[zobrist] = {
            'score': score,
            'depth': depth,
            'type': node_type,
            'best_move': best_move
        }

    # Save PV
    if best_move:
        pv_table[zobrist] = best_move

    return score


def find_best_move(board: Board, player: str, base_depth: int = 5) -> Optional[str]:
    """
    Find the best move for the given player by searching the game tree up to the specified depth using alpha-beta pruning.

    The function evaluates all possible moves for the player and selects the move that leads to the highest evaluated score.

    Args:
        board (Board): The current game board state.
        player (str): The player color ('b' or 'r') to find the best move for.
        base_depth (int, optional): The search depth limit. Defaults to 5.

    Returns:
        str: The best move in string format, or None if no moves are available.
    """

    depth = _adaptive_depth(board, player, base_depth)
    best_move = None
    best_score = -float('inf')
    root_zob = board.zobrist_hash()
    for d in range(1, depth + 1):
        depth_move_counters[d] = defaultdict(int)
        alpha = -float('inf') if d == 1 else best_score - 50
        beta  =  float('inf') if d == 1 else best_score + 50
        best_score = alphabeta(board, d, alpha, beta, True, player, 1, iter_ID=d)
        best_move = pv_table.get(root_zob, best_move)
    return best_move


def quiescence(board: Board, alpha: int, beta: int, player: str) -> int:
    """
    Quiescence search: only explore capture moves to avoid horizon effects.
    """
    stand_pat = evaluate(board, player)
    if stand_pat >= beta:
        return beta
    if alpha < stand_pat:
        alpha = stand_pat
    opponent = 'r' if player == 'b' else 'b'
    for move in board.generate_moves(player):
        frm, to, n = parse_move(move)
        dest = board.piece_at(to)
        if dest is None or dest.color == player:
            continue
        try:
            board.apply_move(player, frm, to, n)
        except:
            continue
        score = -quiescence(board, -beta, -alpha, opponent)
        board.unapply_move()
        if score >= beta:
            return beta
        if score > alpha:
            alpha = score
    return alpha


# --- Minimax search without alpha-beta or TT ---
def minimax(board: Board, depth: int, maximizing: bool, player: str, ply: int = 0, iter_ID: int = 0) -> int:
    opponent = 'r' if player == 'b' else 'b'
    # Generate moves for the current side and log them *before* any early return
    moves = board.generate_moves(player if maximizing else opponent)
    if iter_ID in depth_move_counters:
        depth_move_counters[iter_ID][ply] += len(moves)

    # Terminal or leaf node
    if depth == 0 or board.find_guard(opponent) is None or board.find_guard(player) is None:
        return evaluate(board, player)
    # Count generated moves at this ply for the given iteration ID
    if maximizing:
        best = -float('inf')
        for move in moves:
            frm, to, n = parse_move(move)
            board.apply_move(player, frm, to, n)
            val = minimax(board, depth - 1, False, player, ply + 1, iter_ID)
            board.unapply_move()
            if val > best:
                best = val
        return best
    else:
        best = float('inf')
        for move in moves:
            frm, to, n = parse_move(move)
            board.apply_move(opponent, frm, to, n)
            val = minimax(board, depth - 1, True, player, ply + 1, iter_ID)
            board.unapply_move()
            if val < best:
                best = val
        return best


def find_best_move_MM(board: Board, player: str, base_depth: int = 5) -> Optional[str]:
    """
    Find the best move for the given player by searching the game tree up to the specified depth using alpha-beta pruning.

    The function evaluates all possible moves for the player and selects the move that leads to the highest evaluated score.

    Args:
        board (Board): The current game board state.
        player (str): The player color ('b' or 'r') to find the best move for.
        base_depth (int, optional): The search depth limit. Defaults to 5.

    Returns:
        str: The best move in string format, or None if no moves are available.
    """

    # Simple minimax search at fixed depth
    best_move = None
    best_score = -float('inf')
    # Initialize move counters for this search depth
    depth_move_counters[base_depth] = defaultdict(int)
    root_moves = board.generate_moves(player)
    # Count root moves at ply 1 (root ply is 1)
    depth_move_counters[base_depth][1] = len(root_moves)  # ply 1 = root
    # Evaluate each root move
    for move in root_moves:
        frm, to, n = parse_move(move)
        board.apply_move(player, frm, to, n)
        score = minimax(board, base_depth - 1, False, player, ply=2, iter_ID=base_depth)
        board.unapply_move()
        if score > best_score:
            best_score = score
            best_move = move
    return best_move


def mcts_simulate(node: MCTSNode, eval_player: str) -> float:
    board = copy.deepcopy(node.board)
    player = node.player
    for _ in range(10): # this is the depth of how many moves we simulate, similar to the depth in alphabeta search
        moves = board.generate_moves(player)
        if not moves:
            break
        move = random.choice(moves)
        frm, to, n = parse_move(move)
        try:
            board.apply_move(player, frm, to, n)
        except Exception:
            break
        if board.find_guard('r' if player == 'b' else 'b') is None:
            break
        player = 'r' if player == 'b' else 'b'
    return evaluate(board, eval_player)


# simulations means how many times we simulate the game from the current node, i.e. how many times we play random games from the current board state
def mcts_find_best_move(board: Board, player: str, simulations: int = 1000) -> Optional[str]:
    """
    Find the best move for the given player using Monte Carlo Tree Search.
    Args:
        board (Board): The current game board state.
        player (str): The player color ('b' or 'r') to find the best move for.
        simulations (int, optional): Number of MCTS simulations to run. Defaults to 1000.
    Returns:
        str: The best move in string format, or None if no moves are available.
    """
    root = MCTSNode(copy.deepcopy(board), player)
    if not root.untried_moves:
        return None
    for _ in range(simulations):
        node = root.select()
        if node.untried_moves:
            node = node.expand()
        result = mcts_simulate(node, player)
        node.backpropagate(result)
    best_node = root.best_child()
    return best_node.move