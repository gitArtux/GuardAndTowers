import socket


class Network:
    def __init__(self):
        self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server = "localhost"
        self.port = 5555
        self.addr = (self.server, self.port)
        self.p = self.connect()

    def getP(self):
        return self.p

    def connect(self):
        try:
            self.client.connect(self.addr)
            return self.client.recv(2048).decode('utf-8')
        except:
            pass

    def send(self, data):
        try:
            self.client.send(data.encode('utf-8'))
            return (self.client.recv(4096).decode('utf-8'))
        except socket.error as e:
            print(e)

