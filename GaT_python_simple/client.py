import pygame
import json
from guard_towers import fen_to_board
from evaluation import find_best_move
from network import Network
pygame.font.init()

def main():
    run = True
    clock = pygame.time.Clock()
    n = Network()
    player_val = n.getP()
    if player_val is None:
        print("Could not connect to server or server did not respond with player number.")
        return
    player = int(player_val)
    my_color = 'r' if player == 0 else 'b'
    print("You are player", my_color)

    while run:
        clock.tick(60)
        try:
            #try to send get as a json to server over network, rest is error handling
            game = n.send(json.dumps("get"))
            if game is None:
                raise ValueError("Game data is None")
        except Exception as e:
            run = False
            print("Couldn't get game")
            break

        #response is also a json, json.loads transforms into a python dictionary
        #dictionary consists of board string, a variable turn which is r or b depending whos turn it is, 
        #bothConnected which is True if both players are connected, time which is the left time and end which is False as long as the game isn't finished
        game = json.loads(game)

        if game["bothConnected"] and game["turn"] == my_color:
            board_fen = game["board"]
            print("New Board: " + board_fen)
            print("New Time: " + str(game["time"]))
            print("Current Player's Turn:", game["turn"])

            board, _ = fen_to_board(board_fen)
            ai_move = find_best_move(board, my_color)

            print(f"AI ({my_color}) plays:", ai_move)

            n.send(json.dumps(ai_move))  # send the move to the server



while True:
    main()