import tkinter as tk
from tkinter import messagebox
from guard_towers import Board, coord_to_xy, xy_to_coord, BOARD_SIZE, parse_move

import numpy as np
from evaluation import find_best_move
import threading

import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../CPP_Version")))

import libboard

cpp_result = libboard.say_hello("Ben")
print(cpp_result)

SQUARE = 80
LIGHT = "#F0D9B5"
DARK  = "#B58863"
BLUE  = "#4A90E2"
RED   = "#D0021B"
HIGHLIGHT = "#FFFF66"

# Margins for labels
LEFT_MARGIN = 30   # space for rank labels
BOTTOM_MARGIN = 30 # space for file labels

class GameGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Guard & Towers")

        self.board = Board()
        self.current_player = 'b'
        self.opponent = {'b': 'r', 'r': 'b'}

        self.canvas = tk.Canvas(root,
                                width=SQUARE*BOARD_SIZE + LEFT_MARGIN,
                                height=SQUARE*BOARD_SIZE + BOTTOM_MARGIN,
                                highlightthickness=0)
        self.canvas.grid(row=0, column=0, rowspan=BOARD_SIZE, padx=10, pady=10)

        self.status = tk.StringVar(value="Turn: Blue")
        tk.Label(root, textvariable=self.status,
                 font=("Arial", 14)).grid(row=0, column=1, sticky="n")

        self.random_button = tk.Button(root, text="Random Move", command=self.random_move)
        self.random_button.grid(row=1, column=1, sticky="n", pady=5)

        self.best_button = tk.Button(root, text="Best AI Move", command=self.best_ai_move)
        self.best_button.grid(row=2, column=1, sticky="n", pady=5)

        self.export_button = tk.Button(root, text="Export FEN", command=self.export_position)
        self.export_button.grid(row=3, column=1, sticky="n", pady=5)

        # Free play mode checkbox
        self.free_play = tk.BooleanVar(value=False)
        self.free_play_cb = tk.Checkbutton(root, text="Free Play", variable=self.free_play)
        self.free_play_cb.grid(row=5, column=1, sticky="n", pady=5)

        self.best_move_label = tk.Label(root, text="AI Move: None", font=("Arial", 14))
        self.best_move_label.grid(row=4, column=1, sticky="n")

        # Maps tag -> square
        self.tag_to_square = {}

        # Drag status
        self.drag = {"tag": None, "from_sq": None,
                     "legal": set(), "highlight": []}

        self.draw_board()
        self.draw_pieces()

        # Mouse bindings (generic tag "piece")
        for ev in ("<ButtonPress-1>", "<B1-Motion>", "<ButtonRelease-1>"):
            self.canvas.tag_bind("piece", ev, getattr(self, ev.strip("<>").replace('-', '_')))

        self.update_best_move_label()

    def export_position(self) -> str:
        fen = self.board.export_fen(self.current_player)
        print('Exporting position:', fen)
        return fen

    # ------------------------------------------------------------------ #
    # Drawing
    # ------------------------------------------------------------------ #
    def draw_board(self):
        for y in range(BOARD_SIZE):
            for x in range(BOARD_SIZE):
                px = LEFT_MARGIN + x * SQUARE
                py = (BOARD_SIZE - 1 - y) * SQUARE
                color = LIGHT if (x + y) % 2 == 0 else DARK
                self.canvas.create_rectangle(px, py, px + SQUARE, py + SQUARE,
                                             fill=color, outline="black")

                # Draw file labels (A-G) below the board
                if y == 0:
                    letter = chr(ord('A') + x)
                    self.canvas.create_text(
                        px + SQUARE/2,
                        SQUARE * BOARD_SIZE + BOTTOM_MARGIN / 2,
                        text=letter,
                        font=("Arial", 12, "bold")
                    )

                # Draw rank labels (1-7) to the left of the board
                if x == 0:
                    number = str(y + 1)
                    self.canvas.create_text(
                        LEFT_MARGIN / 2,
                        py + SQUARE/2,
                        text=number,
                        font=("Arial", 12, "bold")
                    )

    def draw_pieces(self):
        # remove previous pieces (shape & text)
        self.canvas.delete("piece")
        self.tag_to_square.clear()

        for y in range(BOARD_SIZE):
            for x in range(BOARD_SIZE):
                pc = self.board.grid[y][x]
                if not pc:
                    continue
                sq = xy_to_coord(x, y)
                uniq = f"piece_{sq}"
                fill = BLUE if pc.color == 'b' else RED
                px = LEFT_MARGIN + x * SQUARE + SQUARE/2
                py = (BOARD_SIZE - 1 - y) * SQUARE + SQUARE/2
                r = SQUARE*0.35
                if pc.kind == 'guard':
                    shape = self.canvas.create_rectangle(px-r, py-r, px+r, py+r,
                                                         fill=fill, outline="black",
                                                         width=2,
                                                         tags=("piece", uniq))
                    txt = "G"
                else:
                    shape = self.canvas.create_oval(px-r, py-r, px+r, py+r,
                                                    fill=fill, outline="black",
                                                    width=2,
                                                    tags=("piece", uniq))
                    txt = str(pc.height)
                self.canvas.create_text(px, py, text=txt, fill="white",
                                        font=("Arial", 16, "bold"),
                                        tags=("piece", uniq))
                self.tag_to_square[uniq] = sq

    # ------------------------------------------------------------------ #
    # Mouse events
    # ------------------------------------------------------------------ #
    def ButtonPress_1(self, event):
        # any item on tag group "piece" can produce current item
        item = self.canvas.find_withtag("current")
        if not item:
            return
        tags = self.canvas.gettags(item[0])
        uniq_tag = next(t for t in tags if t.startswith("piece_"))
        sq = self.tag_to_square[uniq_tag]
        piece = self.board.piece_at(sq)
        if not self.free_play.get() and piece.color != self.current_player:
            return

        # store drag info
        self.drag["tag"] = uniq_tag
        self.drag["from_sq"] = sq
        # compute legal moves for highlight
        legal = set()
        for mv in self.board.generate_moves(self.current_player):
            if mv.startswith(sq + "-"):
                dest = mv.split('-')[1][:2]    # strip optional suffix
                legal.add(dest)
        self.drag["legal"] = legal
        # highlight
        for dest in legal:
            dx, dy = coord_to_xy(dest)
            px = LEFT_MARGIN + dx * SQUARE
            py = (BOARD_SIZE - 1 - dy) * SQUARE
            hl = self.canvas.create_rectangle(px, py, px+SQUARE, py+SQUARE,
                                              outline=HIGHLIGHT, width=3)
            self.drag["highlight"].append(hl)

        # record offset inside square
        self.drag["offset_x"] = event.x
        self.drag["offset_y"] = event.y

    def B1_Motion(self, event):
        if not self.drag["tag"]:
            return
        dx = event.x - self.drag["offset_x"]
        dy = event.y - self.drag["offset_y"]
        self.canvas.move(self.drag["tag"], dx, dy)
        self.drag["offset_x"], self.drag["offset_y"] = event.x, event.y

    def ButtonRelease_1(self, event):
        if not self.drag["tag"]:
            return
        # target square
        bx = int(event.x // SQUARE)
        by = BOARD_SIZE - 1 - int(event.y // SQUARE)
        dest_sq = xy_to_coord(bx, by) if 0 <= bx < BOARD_SIZE and 0 <= by < BOARD_SIZE else None

        # Free-play: allow any move/capture without validation
        if self.free_play.get():
            if dest_sq is None:
                self._cancel_drag()
                return
            from_sq = self.drag["from_sq"]
            piece = self.board.piece_at(from_sq)
            # Move piece freely on board
            self.board.place(from_sq, None)
            self.board.place(dest_sq, piece)
            self._cancel_drag()
            return

        if dest_sq not in self.drag["legal"]:
            self._cancel_drag()
            return

        from_sq = self.drag["from_sq"]
        pc = self.board.piece_at(from_sq)
        # compute n parameter for tower partial moves
        n_param = None
        if pc.kind == 'tower':
            fx, fy = coord_to_xy(from_sq)
            n_param = abs(bx-fx)+abs(by-fy)
            if n_param == pc.height:
                n_param = None
        try:
            self.board.apply_move(self.current_player, from_sq, dest_sq, n_param)
        except Exception as exc:
            messagebox.showerror("Illegal move", str(exc))
            self._cancel_drag()
            return

        self._clear_highlights()
        self.draw_pieces()
        self._check_win_or_switch()

    # ------------------------------------------------------------------ #
    def _cancel_drag(self):
        # snap back
        self._clear_highlights()
        self.draw_pieces()
        self.drag = {"tag": None, "from_sq": None, "legal": set(), "highlight": []}

    def _clear_highlights(self):
        for hl in self.drag["highlight"]:
            self.canvas.delete(hl)
        self.drag["highlight"].clear()
        self.drag["tag"] = None

    def _check_win_or_switch(self):
        opp_guard = self.board.find_guard(self.opponent[self.current_player])
        if opp_guard is None:
            self.status.set(("Blue" if self.current_player=='b' else "Red") + " wins by capture!")
            self._game_over()
            return
        my_guard = self.board.find_guard(self.current_player)
        if my_guard and self.board.is_castle(my_guard, self.current_player):
            self.status.set(("Blue" if self.current_player=='b' else "Red") + " wins by reaching the castle!")
            self._game_over()
            return
        # next player
        self.current_player = self.opponent[self.current_player]
        self.status.set("Turn: " + ("Blue" if self.current_player=='b' else "Red"))
        self.drag = {"tag": None, "from_sq": None, "legal": set(), "highlight": []}

    def _game_over(self):
        self.canvas.unbind("<ButtonPress-1>")
        self.canvas.unbind("<B1-Motion>")
        self.canvas.unbind("<ButtonRelease-1>")
        messagebox.showinfo("Game Over", self.status.get())

    def update_best_move_label(self):
        # run AI search off the main thread to keep UI responsive
        def search():
            best = find_best_move(self.board, self.current_player)
            text = f"AI Move: {best if best else 'None'}"
            self.root.after(0, lambda: self.best_move_label.config(text=text))
        threading.Thread(target=search, daemon=True).start()

    def random_move(self):
        possible_moves = self.board.generate_moves(self.current_player)
        if not possible_moves:
            messagebox.showinfo("No moves", "No legal moves available.")
            return
        move = np.random.choice(possible_moves)
        frm, to, n = parse_move(move)
        def do_random():
            try:
                self.board.apply_move(self.current_player, frm, to, n)
            except Exception as exc:
                messagebox.showerror("Illegal move", str(exc))
                return
            self.draw_pieces()
            self._check_win_or_switch()
            self.update_best_move_label()
        self.animate_move(frm, to, n, do_random)

    def best_ai_move(self):
        best = find_best_move(self.board, self.current_player)
        if not best:
            messagebox.showinfo("No moves", "No AI move available.")
            return
        frm, to, n = parse_move(best)
        def do_best():
            try:
                self.board.apply_move(self.current_player, frm, to, n)
            except Exception as exc:
                messagebox.showerror("Illegal move", str(exc))
                return
            self.draw_pieces()
            self._check_win_or_switch()
            self.update_best_move_label()
        self.animate_move(frm, to, n, do_best)

    def animate_move(self, from_sq, to_sq, n, callback):
        # find all canvas items for the moving piece
        items = self.canvas.find_withtag(f"piece_{from_sq}")
        if not items:
            callback()
            return
        # compute pixel delta
        x0, y0 = coord_to_xy(from_sq)
        x1, y1 = coord_to_xy(to_sq)
        dx = (x1 - x0) * SQUARE
        dy = (y0 - y1) * SQUARE
        steps = 10
        dx_step = dx / steps
        dy_step = dy / steps
        def step(i):
            if i < steps:
                for item in items:
                    self.canvas.move(item, dx_step, dy_step)
                self.root.after(30, lambda: step(i+1))
            else:
                callback()
        step(0)


if __name__ == "__main__":
    tk.Tk().destroy()  # ensure no default root lingers
    root = tk.Tk()
    GameGUI(root)
    root.mainloop()