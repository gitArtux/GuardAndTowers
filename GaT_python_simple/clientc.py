import pygame
import json
from network import Network

import libboard

import time
pygame.font.init()

def main():
    run = True
    clock = pygame.time.Clock()
    n = Network()
    player = int(n.getP())
    print("You are player", player)

    move_count = 0  # Initialize move count
    search_depth = 4  # Set a default search depth

    while run:
        clock.tick(60)
        try:
            #try to send get as a json to server over network, rest is error handling
            game = n.send(json.dumps("get"))
            if game is None:
                raise ValueError("Game data is None")
        except:
            run = False
            print("Couldn't get game")
            break

        #response is also a json, json.loads transforms into a python dictionary
        #dictionary consists of board string, a variable turn which is r or b depending whos turn it is, 
        #bothConnected which is True if both players are connected, time which is the left time and end which is False as long as the game isn't finished
        game = json.loads(game)

        #allow input just when both players are in
        if game["bothConnected"]:
            time.sleep(0.1)  # Adding a small delay to avoid flooding the server with requests
            #allow to only give input, when it is your turn
            if player == 0 and game["turn"] == "r":
                #printing not necessary, game["board"] is the way to get the board string
                # print("New Board: " + game["board"])
                print("New Time: " + str(game["time"]))
                print("Depth:", search_depth)

                #change to any input you like. This one is just console input. Change it here to respond with your Ai's answer. 
                #Answer must have format: start-end-height like E7-F7-1
                # i = input()

                board_fen = game["board"]
                board_fen = board_fen[:-2] + " " + board_fen[-1]  # Ensure the FEN is correctly formatted
                print("Board FEN:", board_fen)

                try:
                    ai_move = libboard.compute_best_move(board_fen, search_depth)
                except Exception as e:
                    print("Error computing best move:", e)
                    ai_move = None

                move_count += 1
                if move_count % 5 == 0:  # Increase search depth every 5 moves
                    if search_depth < 8:
                        search_depth += 1

                if game["time"] < 20000:  # If time is running low, reduce search depth
                    search_depth = 4

                print("AI Move:", ai_move)
                #json.dumps(i) transforms the input into a json. You can print it, if you want to see the difference
                data = json.dumps(ai_move)

                #send data via network
                n.send(data)
            elif player == 1 and game["turn"] == "b":
                print("New Board: " + game["board"])
                print("New Time: " + str(game["time"]))
                print("Depth:", search_depth)

                #Please also change your Input here to respond with your Ai's answer
                # i = input()

                board_fen = game["board"]
                board_fen = board_fen[:-2] + " " + board_fen[-1]  # Ensure the FEN is correctly formatted
                
                try:
                    ai_move = libboard.compute_best_move(board_fen, search_depth)
                except Exception as e:
                    print("Error computing best move:", e)
                    ai_move = None

                move_count += 1
                if move_count % 5 == 0:
                    if search_depth < 8:
                        search_depth += 1

                if game["time"] < 20000:  # If time is running low, reduce search depth
                    search_depth = 4

                print("AI Move:", ai_move)
                data = json.dumps(ai_move)
                n.send(data)

while True:
    main()