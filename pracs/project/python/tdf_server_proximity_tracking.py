#!/usr/bin/env python3
"""
TDF event handler server for proximity and tracking
"""

import argparse
import datetime

import tdf3
import ListenClient

class AbstractSTATE():
     def __init__(self,state_name,context):
         self._state_name = state_name
         self._context = context

     def get_cur_time(self):
         return self._context._cur_time

     def enter(self,previous_state):
         self._previous_state = previous_state
         self._context.WriteMessage("Enter:" + self._state_name + "(previous: " + self._previous_state + ")")
         return

     def process(self,event):

class STATE_INIT(AbstractSTATE):
    def __init__(self,state_name,context):
        super().__init__(state_name,context)

    def enter(self,previous_state):
        super().enter(previous_state)
        return

    def process(self,event):
        argData = event.split(',')
        if argData[0] == "EVENT_START" :
            self.transfer_to_state("STATE_TDF_EVENT_HANDLER")

    def exit(self):
        super().exit()
        return

class STATE_TDF_EVENT_HANDLER(AbstractSTATE):
    def __init__(self,state_name,context):
        super().__init__(state_name,context)

    def enter(self,previous_state):
        super().enter(previous_state)
        # Baselisten connection
        listener = ListenClient.ListenClient(base_host, base_port)
        listener.connect()
        return

    def process(self,event):
        argData = event.split(',')
        if argData[0] == "EVENT_START" :
            self.transfer_to_state("STATE_LOGIN_INIT")

            try:
                packet = listener.read(timeout=None)
            except ConnectionResetError:
                print("Connection to baselisten lost...")
                return
            except NotImplementedError:
                continue
            
            # Loop over every payload in the serial packet
            for payload_type, route, payload in packet.iter_payloads():

                first_hop = route[-1]
                pkt_addr = first_hop.address_fmt
                pkt_rssi = "{:d}dBm".format( first_hop.rssi)
                print("From {:s}, {:d} bytes, RSSI {:s}".format(pkt_addr, len(payload), pkt_rssi))

                # This demo is only concerned with TDF payloads
                if payload_type != PayloadType.PAYLOAD_TDF3:
                    print("\tPacket was not a TDF ({:})".format(payload_type))
                    continue
                
                # Payload is a TDF
                for point in parser(payload, datetime.datetime.utcnow(), debug=False, combine=True):
                    print(point)

    def exit(self):
        super().exit()
        return

class EventMonitor():
    def __init__(self):
        self._state_map = {}

        self._next_state = Queue()

        # Init STATE_INIT
        self._state_map['STATE_INIT'] = STATE_INIT("STATE_INIT",self)

        # Init STATE_WIFI_HOT_SPOT_AP_READY
        self._state_map['STATE_WIFI_HOT_SPOT_AP_READY'] = STATE_REPLY_INIT("STATE_WIFI_HOT_SPOT_AP_READY",self)

        # Init STATE_WIFI_IP_READY
        self._state_map['STATE_WIFI_IP_READY'] = STATE_LOGIN_INIT("STATE_WIFI_IP_READY",self)

        # Init STATE_WIFI_IP_READY
        self._state_map['STATE_WIFI_IP_READY'] = STATE_LOGIN_INIT("STATE_WIFI_IP_READY",self)

        # Init STATE_TDF_EVENT_HANDLER
        self._state_map['STATE_TDF_EVENT_HANDLER'] = STATE_LOGIN_INIT("STATE_TDF_EVENT_HANDLER",self)

        # Init thread and queue
        self._outer_message_queue = Queue()
        self._thread = Thread(target=self.process, args=([self._outer_message_queue]))
        self._thread.daemon = False
        self._thread.start()
        self.SendMessage("EVENT_START","")

    def process(self,queue):
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

    def SendMessage(self, eventName, strMessage):
        self._outer_message_queue.put(eventName + "," + strMessage)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='TDF listener')
    parser.add_argument('--host', dest='base_host', type=str, default="localhost", help='Hostname for baselisten')
    parser.add_argument('--port', dest='base_port', type=int, default=9001, help='Port for baselisten')
    parser.add_argument('--tdf', dest='tdf_server', type=str, default=None, help='Hostname for TDF server')

    args = parser.parse_args()

    demo_receiver(**vars(args))
