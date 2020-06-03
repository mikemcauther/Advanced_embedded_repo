#!/usr/bin/env python3

import socket
import sys
import os
import time
import datetime
import struct
import threading
import queue
import re
import copy
import select
import logging

class SocketConnections():

    class ClientGroup:
        '''
        Unique group of clients identified by connection socket
        Used to distribute messages to all connections against the connection socket
        '''

        def __init__(self, connection_port,connection_host):
            self._connection_host = connection_host
            self._connection_port = connection_port
            self._connection_socket = self._new_socket(connection_port,connection_host)
            self._clients = {}
            self._clients_lock = threading.Lock()

        def _new_socket(self, port,host):
            new_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            new_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            #new_socket.bind(('', port))
            new_socket.bind((host, port))
            new_socket.listen()
            return new_socket

        def accept_connection(self, connection_socket):
            if connection_socket == self._connection_socket:
                (client_socket, (addr, port)) = self._connection_socket.accept()
                client_socket.setblocking(0)
                self._clients_lock.acquire()
                self._clients[client_socket] = {"addr": addr, "local_port": self._connection_port, "remote_port": port}
                self._clients_lock.release()
                print("Client: {:s}:{:d} connected".format(addr, port))
                return client_socket
            return None

        def distribute(self, byte_buffer):
            broken_clients = []
            self._clients_lock.acquire()
            for client in self._clients:
                try:
                    client.send(byte_buffer)
                except (socket.timeout, socket.error):
                    broken_clients.append(socket)
            for client in broken_clients:
                self.close_socket(client)
            self._clients_lock.release()
            return

        def close(self):
            print("Sockets: Closing port {:d}".format(self._connection_port))
            self._connection_socket.close()
            # Try for up to a second to acquire the lock
            # If this fails, it is likely the thread crashed while it held the lock
            acquired = self._clients_lock.acquire(True, 1.0)
            client_list = [x for x in self._clients]
            for client in client_list:
                self.close_socket(client)
            if acquired:
                self._clients_lock.release()

        def close_socket(self, sock):
             if sock in self._clients:
                 try:
                     sock.shutdown(sock.SHUT_RDWR)
                 except:
                     pass
                 try:
                     sock.close()
                 except:
                     pass
                 print("Client: {:s}:{:d} disconnected".format(self._clients[sock]["addr"], self._clients[sock]["remote_port"]))
                 del self._clients[sock]
                 return True
             return False

        def socket_info(self, sock):
            return (self._clients[sock]["addr"], self._clients[sock]["remote_port"])

        @property
        def num_clients(self):
            return len(self._clients)

        @property
        def sockets(self):
            self._clients_lock.acquire()
            sockets = [self._connection_socket, *self._clients]
            self._clients_lock.release()
            return sockets

    def __init__(self, packet_port, debug_port,packet_host):
        self.packet_group = self.ClientGroup(packet_port,packet_host)
        self.debug_group = self.ClientGroup(debug_port,packet_host)
        self.max_clients = 0
        self._receive_len=128

    def socket_recv(self):
        #packets = []
        data = bytearray()
        # Listen for events on all sockets we are interested in
        all_sockets = self.packet_group.sockets + self.debug_group.sockets
        readable, writable, err = select.select(all_sockets, [], [], 1.0)
        for sock in readable:
            # A new client connecting to one of our groups
            if self.packet_group.accept_connection(sock) or self.debug_group.accept_connection(sock):
                self.client_summary()
            # An existing client has had some event
            else:
                # Try and read data from sock
                try:
                    #packets.append(PacpTransportSocket.receive_from(sock))
                    packet = sock.recv(self._receive_len)
                    if packet == b"":
                        raise ConnectionResetError
                    #msg = byte.decode('utf-8')
                    #print(msg)
                    data.extend(packet)
                # Check for remote connections closing
                except (ConnectionResetError, sock.timeout, sock.error):
                    if self.packet_group.close_socket(sock) or self.debug_group.close_socket(sock):
                        self.client_summary()
                    continue
                # Extract packets from the sock stream
        for sock in writable:
            print("Socket appeared in writeable list {:}".format(sock))
            if self.packet_group.close_socket(sock) or self.debug_group.close_socket(sock):
                self.client_summary()
        for sock in err:
            print("Socket appeared in error list {:}".format(sock))
            if self.packet_group.close_socket(sock) or self.debug_group.close_socket(sock):
                self.client_summary()
        #return packets
        return data

    def release(self):
        try:
            self.packet_group._clients_lock.release()
        except RuntimeError:
            pass
        try:
            self.debug_group._clients_lock.release()
        except RuntimeError:
            pass

    def close(self):
        self.packet_group.close()
        self.debug_group.close()

    def client_summary(self):
        current_clients = self.packet_group.num_clients + self.debug_group.num_clients
        self.max_clients = max(self.max_clients, current_clients)
        print("Clients: now={:d}, max={:d}".format(current_clients, self.max_clients))

if __name__ == '__main__':
    listener = ListenClient(port=9001)
    import time
    listener.connect()
    listener.read()
    listener.close()
