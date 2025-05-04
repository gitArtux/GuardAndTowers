import tkinter as tk
from tkinter import messagebox
from guard_towers import Board, coord_to_xy, xy_to_coord, BOARD_SIZE

SQUARE = 80
LIGHT = "#F0D9B5"
DARK  = "#B58863"
BLUE  = "#4A90E2"
RED   = "#D0021B"
HIGHLIGHT = "#FFFF66"

class GameGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Guard & Towers")

        self.board = Board()
        self.current_player = 'b'
        self.opponent = {'b': 'r', 'r': 'b'}

        self.canvas = tk.Canvas(root,
                                width=SQUARE*BOARD_SIZE,
                                height=SQUARE*BOARD_SIZE,
                                highlightthickness=0)
        self.canvas.grid(row=0, column=0, padx=10, pady=10)

        self.status = tk.StringVar(value="Turn: Blue")
        tk.Label(root, textvariable=self.status,
                 font=("Arial", 14)).grid(row=0, column=1, sticky="n")

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

    # ------------------------------------------------------------------ #
    # Drawing
    # ------------------------------------------------------------------ #
    def draw_board(self):
        for y in range(BOARD_SIZE):
            for x in range(BOARD_SIZE):
                px, py = x*SQUARE, (BOARD_SIZE-1-y)*SQUARE
                color = LIGHT if (x+y) % 2 == 0 else DARK
                self.canvas.create_rectangle(px, py, px+SQUARE, py+SQUARE,
                                             fill=color, outline="black")

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
                px, py = x*SQUARE+SQUARE/2, (BOARD_SIZE-1-y)*SQUARE+SQUARE/2
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
        if piece.color != self.current_player:
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
            px, py = dx*SQUARE, (BOARD_SIZE-1-dy)*SQUARE
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


if __name__ == "__main__":
    tk.Tk().destroy()  # ensure no default root lingers
    root = tk.Tk()
    GameGUI(root)
    root.mainloop()
