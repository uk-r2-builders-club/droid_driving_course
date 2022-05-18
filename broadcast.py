import socket
import ipaddress
import time
import json
import threading

class BroadCaster(object):

    def __init__(self):
        super(BroadCaster, self).__init__()
        self.IPADDRESS = socket.gethostbyname(socket.gethostname() + ".local")
        self.UDP_IP = socket.inet_ntoa(socket.inet_aton(self.IPADDRESS)[:3] + b'\xff' )
        self.UDP_PORT = 8888
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        bound = None
        while bound is None:
            try:
                self.sock.bind((self.IPADDRESS,0))
                bound = True
            except:
                print("Fail")
                pass
            time.sleep(5)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)


    def broadcast_message(self, message):
        self.sock.sendto(message, (self.UDP_IP, self.UDP_PORT))
        

