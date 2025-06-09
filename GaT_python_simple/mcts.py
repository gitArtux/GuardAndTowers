
import copy
import math
from typing import Optional

from guard_towers import Board, parse_move

class MCTSNode:
    def __init__(self, board: Board, player: str, move: Optional[str] = None, parent: Optional['MCTSNode'] = None):
        self.board = board
        self.player = player
        self.move = move
        self.parent = parent
        self.children: list[MCTSNode] = []
        self.visits = 0
        self.value = 0.0
        self.untried_moves = board.generate_moves(player)

    def ucb1(self, c=1.4) -> float:
        if self.visits == 0:
            return float('inf')
        return self.value / self.visits + c * math.sqrt(math.log(self.parent.visits) / self.visits)

    def best_child(self) -> 'MCTSNode':
        return max(self.children, key=lambda child: child.visits)

    def select(self) -> 'MCTSNode':
        node = self
        while node.children and not node.untried_moves:
            node = max(node.children, key=lambda child: child.ucb1())
        return node

    def expand(self) -> 'MCTSNode':
        move = self.untried_moves.pop()
        frm, to, n = parse_move(move)
        new_board = copy.deepcopy(self.board)
        new_board.apply_move(self.player, frm, to, n)
        next_player = 'r' if self.player == 'b' else 'b'
        child_node = MCTSNode(new_board, next_player, move=move, parent=self)
        self.children.append(child_node)
        return child_node

    def backpropagate(self, result: float):
        node = self
        while node:
            node.visits += 1
            node.value += result
            node = node.parent

