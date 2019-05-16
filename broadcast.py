import socket
import time
import json
import threading

class BroadCaster(object):

    def __init__(self):
        super(BroadCaster, self).__init__()
        self.UDP_IP = '192.168.43.255'
        self.UDP_PORT = 8888
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('192.168.43.1',0))
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)


    def broadcast_message(self, message):
        self.sock.sendto(message, (self.UDP_IP, self.UDP_PORT))
        

