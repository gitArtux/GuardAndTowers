import socket
from _thread import *
import json
import random
from gat import Game
import pygame
import sys

server = "localhost"
port = 5555

#block about writing the log, delete for public version
fileName = sys.argv[1]
logfile = open(fileName, 'w')
playerNames = fileName.split("-")
logfile.write("Spieler Rot: " + playerNames[1] + "\n")
logfile.write("Spieler Blau: " + playerNames[2].split(".")[0] + "\n")
logfile.write("\n")
#end block about log

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    s.bind((server, port))
except socket.error as e:
    str(e)

s.listen(2)
print("Waiting for a connection, Server Started")

connected = set()
games = {}
idCount = 0
pygame.init()
clock = pygame.time.Clock()


def threaded_client(conn, p, gameId):
    try:
        initialTime = 120000
        timePlayer = initialTime
        #fileLog = open(str(gameId) + str(p) + str(random.randint(1, 999999)) + ".txt", "a")
        global idCount
        conn.send(str.encode(str(p)))
        firstSend = False
        if gameId in games:
            game = games[gameId]
            if p == 0:
                print("P1 connected")
                #fileLog.write("P1 connected \n")
                firstSend = True
                game.connectP1()
                game.turn = "r"
            else:
                print("P2 connected")
                #fileLog.write("P2 connected \n")
                game.connectP2()
        
        reply = ""
        while True:
            try:
                # Receiving data
                data = conn.recv(4096).decode('utf-8')
                if not data:
                    print("No data received. Breaking.")
                    break
                data = json.loads(data)

                if gameId in games:
                    game = games[gameId]

                    # Processing the move (only if it's not "get")
                    if data != "get":
                        timeForMove = clock.tick()
                        timePlayer = timePlayer - timeForMove + 10
                        #fileLog.write("Time remaining active player: " + str(timePlayer) + "\n")

                        if timePlayer < 0:
                            handle_game_end(game, "Timeout", conn, logfile, timePlayer, 1-p)
                            break

                        # Validating the turn and sending to playTurn
                        if (p == 0 and game.turn == "r") or (p == 1 and game.turn == "b"):
                            try:
                                game.playTurn(p, data)
                            except Exception as e:
                                print(f"Error in playTurn: {e}")
                                import traceback
                                traceback.print_exc()
                                break
                        else:
                            print(f"Invalid player turn: Player {p} tried to play on {game.turn}'s turn.")

                        # Checking for invalid move
                        if not game.valid:
                            print("Move was invalid. Ending game for the player.")
                            handle_game_end(game, "Invalid Move", conn, logfile, timePlayer, 1-p)
                            break
                        
                        #next lines are only logfile
                        if(game.turn == "b"):
                            logfile.write("Red move: "+ data + "   ---  time left: " + str(timePlayer) + "\n")
                        else:
                            logfile.write("Blue move: "+ data + "   ---  time left: " + str(timePlayer) + "\n")
                        logfile.write(game.boardString + "\n")
                        #logfile done

                        # Checking for winner
                        game.determinateWinner()
                        if game.win is not None:
                            print(f"Game won by: {game.win}")
                            handle_game_end(game, "win", conn, logfile, timePlayer, p)
                            break

                    # Building response output
                    try:
                        output = dict(
                            board=game.boardToString(),
                            turn=game.turn,
                            bothConnected=game.bothConnected(),
                            time=timePlayer,
                            end=False
                        )
                        output = json.dumps(output).encode('utf-8')
                        conn.sendall(output)
                    except Exception as e:
                        print(f"Error in sending game state: {e}")
                        import traceback
                        traceback.print_exc()
                        break

                    # Handling first send
                    if firstSend and game.bothConnected():
                        clock.tick()
                        firstSend = False
                else:
                    print("Game ID not in games. Breaking.")
                    break

            except json.JSONDecodeError as je:
                print(f"JSON decoding error: {je}")
                import traceback
                traceback.print_exc()
                break
            except ConnectionResetError:
                print("Client disconnected.")
                break
            except Exception as e:
                print(f"Unexpected Error in game loop: {e}")
                import traceback
                traceback.print_exc()
                break

    except Exception as main_e:
        print(f"Critical Error in threaded_client: {main_e}")
        import traceback
        traceback.print_exc()
    
    finally:
        # Clean up safely
        #fileLog.close()
        logfile.close()
        print("Lost connection")
        try:
            del games[gameId]
            print("Closing Game", gameId)
        except KeyError:
            print("Game ID not found in games.")

        idCount -= 1
        conn.close()

def handle_game_end(game, reason, conn, fileLog, timePlayer, player):
    if player == 0:
        colour = "r"
    else: colour = "b"
    # Write reason to log and print
    log_line = f"Game finished {reason}, win: {colour}\n"
    print(log_line.strip())
    fileLog.write(log_line)

    # Build the response
    output = dict(
        board=game.boardToString(),
        turn=game.turn,
        bothConnected=game.bothConnected(),
        time=timePlayer,
        end=True
    )
    output = json.dumps(output).encode('utf-8')
    conn.sendall(output)
    fileLog.close()

while True:
    conn, addr = s.accept()
    print("Connected to:", addr)

    idCount += 1
    p = 0
    gameId = (idCount - 1)//2
    if idCount % 2 == 1:
        games[gameId] = Game(gameId)
        print("Creating a new game...")
    else:
        games[gameId].ready = True
        p = 1


    start_new_thread(threaded_client, (conn, p, gameId))