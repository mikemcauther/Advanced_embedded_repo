import os,time
import math

import sys
from threading  import Thread

try:
    from queue import Queue, Empty
except ImportError:
    from Queue import Queue, Empty

import re



import ListenClient

#----------------------------------------------------------------------------------------------------------------------------------------------------
class STATE_INIT():
    def __init__(self,state_name,context,current_node):
        self._state_name = state_name
        self._context = context
        self.current_node = current_node
        self.numerical_pattern = re.compile('^(\-?[0-9]*)([^0-9]*)?$')
        return

    def get_name(self):
        return self._state_name

    def enter(self,previous_state):
        #super().enter(previous_state)
        return

    def process(self,event):
        argData = event.split(',')
        if argData[0] == "EVENT_NEW_PACKET" :
            match_result = self.numerical_pattern.match(argData[2])
            if match_result.group(1) != None:
                self.current_node.callback_on_socket_update(argData[1],match_result.group(1))
        return

    def exit(self):
        #super().exit()
        return

class SocketMonitor():
    def __init__(self,current_node,message_board=None):
        self._cur_time = 0

        self._message_board = message_board
        self.current_node = current_node

        self._state_map = {}
        self._next_state = Queue()

        # Init STATE_INIT
        self._state_map['STATE_INIT'] = STATE_INIT("STATE_INIT",self,current_node)

        self._current_state = self._state_map['STATE_INIT']
        self.add_next_state('STATE_INIT')

        base_socket_port = 333
        base_socket_host = "192.168.43.69"
        self.sockets = ListenClient.SocketConnections(base_socket_port, base_socket_port + 1,base_socket_host)

        # Init thread and queue
        self._outer_message_queue = Queue()
        self._thread = Thread(target=self.process, args=([self._outer_message_queue]))
        self._thread2 = Thread(target=self.socket_process, args=([self.sockets]))
        self._thread.daemon = False
        self._thread2.daemon = False
        self._thread.start()
        self._thread2.start()
        self.SendMessage("EVENT_START","")



    def process(self,queue):
        packet = ""
        while True:
            try:
                event = queue.get()
            except Empty:
                #print('Nothing in queue')
                continue

            self._current_state.process(event)

            while True:
                try:
                    next_state_name = self._next_state.get_nowait()
                except Empty:
                    #print('Nothing in next_state_name queue')
                    break

                if self._current_state.get_name() != next_state_name:
                    self._current_state.exit()

                    next_state_obj = self._state_map[next_state_name]

                    next_state_obj.enter(self._current_state.get_name())
                    self._current_state = next_state_obj

    def socket_process(self,sockets):
        packet = ""
        while True:
            try:
                packet = sockets.socket_recv()
                event = packet.decode('utf-8')
                print(""+event)
                if len(event) < 3:
                    continue
                self.SendMessage("EVENT_NEW_PACKET",event)
            except Empty:
                #print('Nothing in queue')
                continue

        self.sockets.release()
        self.sockets.close()

    def add_next_state(self,next_state_name):
        self._next_state.put(next_state_name)

    def transfer_to_state(self,next_state_name):
        self.add_next_state(next_state_name)
        
    def SendMessage(self, eventName, strMessage):
        self._outer_message_queue.put(eventName + "," + strMessage)

    def WriteMessage(self, strMsg):
        print(strMsg)
        #self._message_board.insert('end', strMsg)
        #self._message_board.see('end')
