import numpy as np
import re

class Game:
    #Board is send like a FenString. Start Board is: r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1
    #Move is send like A1-B1-1 where the first coordinate corresponds to the start, the second to the end and the last is number of soldiers. 
    #A1 is in the bottom left corner
    def __init__(self, 
                 turn= None, 
                 oldBoard=None, 
                 boardString=None,
                 move=None, 
                 moveCap=1, 
                 statusEndSquare=None, 
                 valid=True, 
                 valueError=False, 
                 win=None,
                 p1Connected=False,
                 p2Connected=False,
                 id = id,
                 ready = False):
        
        #Use provided turn or default turn
        self.turn = turn if turn is not None else "r"
        
        # Use provided board or default to the standard board setup
        self.oldBoard = oldBoard if oldBoard is not None else np.array([
            ["r1", "r1", 0, "RG", 0, "r1", "r1"],
            [0, 0, "r1", 0, "r1", 0, 0],
            [0, 0, 0, "r1", 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, "b1", 0, 0, 0],
            [0, 0, "b1", 0, "b1", 0, 0],
            ["b1", "b1", 0, "BG", 0, "b1", "b1"]
        ])
        
        # Use provided boardString or default to the standard board setup
        self.boardString = boardString if boardString is not None else "r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r"
        
        # Use provided move or default move
        self.move = move if move is not None else np.array([[2, 3], [3, 3]])
        
        self.moveCap = moveCap
        self.statusEndSquare = statusEndSquare if statusEndSquare is not None else [0, 0, 0]
        self.valid = valid
        self.valueError = valueError
        self.win = win
        self.p1Connected = False
        self.p2Connected = False
        self.id = id
        self.ready = False

    def printBoard(self):
        size = self.oldBoard.shape[0]

        # Print the top border
        print("     0    1    2    3    4    5    6")
        print("  " + " ----" * size)
    
        for i, row in enumerate(self.oldBoard):
            # Print each row with vertical dividers
            print(f"{i} | " + ' | '.join('  ' if cell == '0' else str(cell) for cell in row) + " |")
        
            # Print the horizontal divider between rows
            print("  " + " ----" * size)

    def bothConnected(self):
        return self.p1Connected and self.p2Connected
    
    def connectP1(self):
        self.p1Connected = True

    def connectP2(self):
        self.p2Connected = True

    def playTurn(self, player, move):
        try:
            print(f"Playing turn. Player: {player}, Move: {move}")
            if player == 0:
                if self.turn == "b":
                    self.winner = "b"
                    print("Player 0 tried to play on 'b' turn.")
                    return
            if player == 1:
                if self.turn == "r":
                    self.winner = "r"
                    print("Player 1 tried to play on 'r' turn.")
                    return
            
            self.parseMoveString(move)
            self.validMove()
            if self.valid:
                self.doMove()
                self.boardToString()
            else:
                print("Move is invalid.")

        except Exception as e:
            print(f"Error in playTurn: {e}")
            import traceback
            traceback.print_exc()
            raise
    
    def validMove(self):
        #Test if a non Integer is commited
        if self.valueError:
            self.valid = False
            return False, "Invalid string"
        
        #Test if Move is in the bounds of field
        if np.any((self.move < 0) | (self.move > 6)):
            self.valid = False
            return False, "Out of bounds"
        
        start_row, start_col = self.move[0]
        end_row, end_col = self.move[1]

        #Test if move is orthogonal
        if start_row != end_row and start_col != end_col:
            self.valid = False
            return False, "Not orthoganal"
        elif start_row == end_row and start_col == end_col:
            self.valid = False
            return False, "Same square"
        
        startSpace = self.oldBoard[start_row, start_col]
        endSpace = self.oldBoard[end_row, end_col]
        # Calculate the row and column differences
        row_diff = abs(end_row - start_row)
        col_diff = abs(end_col - start_col)
        
        #Test if own piece is moved
        if not self.ownPieceIsMoved(startSpace):
            self.valid = False
            return False, "Moved enemy piece or from a not occupied square."

        #Test if Guard is moved
        if startSpace[-1] == "G":
            if (row_diff == 1 and col_diff == 0) or (row_diff == 0 and col_diff == 1):
                #Test if square is mine
                if not self.squareIsMine(endSpace):
                    self.valid = True
                    return True
                else: 
                    self.valid = False
                    return False, "Guard tries to bully own soldier"
            else:
                self.valid = False
                return False, "Guard is moved more than one space"
        
        #Test if player is trying to move more pieces, than they have
        if startSpace == 0:
            self.valid = False
            return False, "Start square doesn't contain a soldier"
        
        if self.moveCap > int(startSpace[-1]):
            self.valid = False
            return False, "Start square doesn't contain enough soldiers"
        
        #Test if player is moving exactly as many squares, as he is moving soldiers
        if not(row_diff == self.moveCap or col_diff == self.moveCap):
            self.valid = False
            return False, "Trying to move more or less squares than moving soldiers"
        
        #Test if player tries to jump
        if not self.pathIsClear(start_row, start_col, end_row, end_col):
            self.valid = False
            return False, "Path is not clear"

        #Test status on the end square
        if not (self.squareIsEmpty(endSpace) or self.squareIsMine(endSpace) or self.squareIsEnemy(endSpace)):
            self.valid = False
            return False, self.statusEndSquare, "End square isn't empty, doesn't contain my soldier or contains more enemy soldiers than moved soldiers"
        
        self.valid = True
        return True
    
    def doMove(self):
        start_row, start_col = self.move[0]
        end_row, end_col = self.move[1]
        startSquare = self.oldBoard[start_row, start_col]
        endSquare = self.oldBoard[end_row, end_col]

        #Check if Guard is moved
        if startSquare[-1] == "G":
            self.oldBoard[start_row, start_col] = 0
            self.oldBoard[end_row, end_col] = startSquare
        #Check if Guard is captured
        elif endSquare[-1] == "G": 
            startStack = int(startSquare[-1])
            newStart = str(self.turn + str(startStack - self.moveCap))
            if int(newStart[-1]) == 0:
                self.oldBoard[start_row, start_col] = 0
            self.oldBoard[start_row, start_col] = newStart
            self.oldBoard[end_row, end_col] = self.turn + str(self.moveCap)
        #Normal Movement
        else:
            startStack = int(startSquare[-1])
            endStack = int(endSquare[-1])

            newStart = str(self.turn + str(startStack - self.moveCap))

            #Correction if start square is now empty
            if int(newStart[-1]) == 0:
                self.oldBoard[start_row, start_col] = 0
            else: self.oldBoard[start_row, start_col] = newStart

            #Other kind of normal movements
            if self.squareIsEmpty(endSquare) or self.squareIsEnemy(endSquare):
                self.oldBoard[end_row, end_col] = self.turn + str(self.moveCap)
            #Movements on itself
            else:
                self.oldBoard[end_row, end_col] = self.turn + str(self.moveCap + endStack)
            
        #Change turns
        self.changeTurns()

    def changeTurns(self):
        if self.turn == "r":
                self.turn = "b"
        else: self.turn = "r"
    
    def determinateWinner(self):
        if not np.any(self.oldBoard == "RG"):
            self.win = "b"
        if not np.any(self.oldBoard == "BG"):
            self.win = "r"
        if "B" in self.oldBoard[0,3]:
            self.win = "b"
        if "R" in self.oldBoard[6,3]:
            self.win = "r"
        #TODO: Draw, include Threefold repetition, Check different draw variations
        
    #Functions specific for this game to minimize code waste
    def ownPieceIsMoved(self, startSpace):
        if startSpace[-1] == "G":
            if self.turn == "r":
                if startSpace[0] == "R":
                    return True
                else: return False
            if self.turn == "b":
                if startSpace[0] == "B":
                    return True
                else: return False
        if startSpace[0] == self.turn:
            return True
        else: return False
    
    def pathIsClear(self, start_row, start_col, end_row, end_col):
        if start_row == end_row:  # Horizontal move
            step = 1 if end_col > start_col else -1  # Determine direction
            for col in range(start_col + step, end_col, step):  # Exclude start_col and end_col
                if not self.squareIsEmpty(self.oldBoard[start_row, col]):
                    return False
        elif start_col == end_col:  # Vertical move
            step = 1 if end_row > start_row else -1  # Determine direction
            for row in range(start_row + step, end_row, step):  # Exclude start_row and end_row
                if not self.squareIsEmpty(self.oldBoard[row, start_col]):
                    return False
        else:
            # Not a valid orthogonal move
            return False
        
        # If no squares were occupied, the path is clear
        return True

    def squareIsEmpty(self, endSpace):
        if endSpace == "0":
            return True
        else: 
            self.statusEndSquare[0] = 1
            return False
    
    def squareIsMine(self, endSpace):
        if self.turn in endSpace:
            return True
        else: 
            self.statusEndSquare[1] = 1
            return False
    
    def squareIsEnemy(self, endSpace):
        if self.turn == "r":
            enemy = "b"
        else: enemy = "r"

        if enemy in endSpace:
            if self.moveCap >= int(endSpace[-1]):
                return True
        #TODO: Ugly, try to write this segment decent
        elif "G" in endSpace:
            if enemy == "r":
                if "B" in endSpace:
                    return True
                else: return False, "Trying to capture own Guard"
            else: 
                if "R" in endSpace:
                    return True
                else: return False, "Trying to capture own Guard"
        else: 
            self.statusEndSquare[2] = 1
            return False
    
    def setMove(self, start, end, height):
        try:
            start_row, start_col = int(start[0]), int(start[-1])
            end_row, end_col = int(end[0]), int(end[-1])
            self.move = np.array([[start_row, start_col], [end_row, end_col]])
            self.moveCap = int(height)
            self.valueError = False
            return self.move
        except ValueError:
            self.valueError = True
            return False, "Please enter valid integer" 
        
    #Functions for parsing
    def boardToString(self):
        try:
            rows = []
            for i, row in enumerate(self.oldBoard):
                row_str = ''
                empty_count = 0
                for j, cell in enumerate(row):
                    if str(cell) == "0":
                        empty_count += 1
                    else:
                        if empty_count > 0:
                            row_str += str(empty_count)
                            empty_count = 0
                        row_str += str(cell)
                
                if empty_count > 0:
                    row_str += str(empty_count)
                rows.append(row_str)
            board_string = "/".join(rows) + " " + self.turn
            self.boardString = board_string
            return board_string

        except Exception as e:
            print("Error in boardToString:", e)
            raise
    
    def parseMoveString(self, move_str):
        grid_size = 7
        try:
            part1, part2, height_str = move_str.strip().split('-')
            self.moveCap = int(height_str)

            def algebraic_to_coords(pos):
                col = ord(pos[0].upper()) - ord('A')  # 'A' → 0, 'B' → 1, etc.
                row = grid_size - int(pos[1])         # '1' → bottom, so subtract from grid size
                return [row, col]

            start = algebraic_to_coords(part1)
            end = algebraic_to_coords(part2)
            self.move = np.array([start, end])

        except ValueError:
            print("Invalid move string format:", move_str)
            if self.turn == "r":
                self.win = "b"
            else: 
                self.win = "r"
            raise ValueError(f"Invalid move string format: '{move_str}'")

        except Exception as e:
            print("Unexpected error in parseMoveString:", e)
            raise

