from __future__ import with_statement
from _Framework.ControlSurface import ControlSurface
from _Framework.InputControlElement import *
from Cenx4MixerComponent import Cenx4MixerComponent
from Cenx4SessionComponent import Cenx4SessionComponent
from Cenx4DeviceComponent import Cenx4DeviceComponent
from Cenx4Mgr import Cenx4Mgr
from MIDIProtocol import (
    SysExProtocol,
    CENX4_MAIN_MIDI_SYSEX_APP_CMD,
    CENX4_APP_ABLETON_SYSEX_RESYNC,
)

from Cenx4EncoderElement import Cenx4EncoderElement


import sys
sys.path.append('/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7')
import StringIO, socket, code

class Cenx4(ControlSurface):
    def __init__(self, c_instance):
        super(Cenx4, self).__init__(c_instance)
        with self.component_guard():
            self.__c_instance = c_instance
            self.do_setup()
        self.log_message('hi!!')


        # Telnet
        self.originalstdin = sys.stdin
        self.originalstdout = sys.stdout
        self.originalstderr = sys.stderr

        self.stdin = StringIO.StringIO()
        self.stdout = StringIO.StringIO()
        self.stderr = StringIO.StringIO()

        self.telnetSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.telnetSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.telnetSocket.bind( ('', 1337) )
        self.telnetSocket.setblocking(False)
        self.telnetSocket.listen(1)
        self.telnetConnection = None

        self.interpreter = code.InteractiveConsole(globals())

        self.telnetBuffer = ""
        self.lastData = ""
        self.commandBuffer = []

    def suggest_input_port(self):
        return '?'

    def suggest_output_port(self):
        return '?'

    def disconnect(self):
        super(Cenx4, self).disconnect()

        # Telnet
        sys.stdin = self.originalstdin
        sys.stdout = self.originalstdout
        sys.stderr = self.originalstderr
        self.telnetSocket.close()

    def do_setup(self):
        Cenx4Mgr.set_log(self.log_message)
        Cenx4Mgr.send_midi = self.__c_instance.send_midi
        Cenx4Mgr.sysex = SysExProtocol(self.__c_instance.send_midi)

        # mixer_d_controls = [ Cenx4ParameterElement(MIDI_CC_TYPE, 0, i + 64) for i in range(32) ]

        # TODO what is this 8?
        self._session = Cenx4SessionComponent(8, 12)
        self._mixer = Cenx4MixerComponent(8)
        self._session.set_mixer(self._mixer)
        for i in range(8):
            ch = self._mixer.channel_strip(i)
            self.log_message(repr(ch))
            # ch.set_device_controls(tuple(mixer_d_controls[i * 4:i * 4 + 4]))

        self._ab_device_component = Cenx4DeviceComponent()
        self.set_device_component(self._ab_device_component)

        device_controls = [Cenx4EncoderElement(MIDI_CC_TYPE, 2, i, True) for i in range(16)]
        self._ab_device_component.set_parameter_controls(tuple(device_controls))

    def handle_sysex(self, midi_bytes):
        data = Cenx4Mgr.sysex.decode(midi_bytes)
        if data is None:
            return super(Cenx4, self).handle_sysex(midi_bytes)

        if len(data) >= 2 and data[0] == CENX4_MAIN_MIDI_SYSEX_APP_CMD:
            cmd, data = data[1], data[2:]
            Cenx4Mgr.log(('Incoming app sysex cmd', cmd, data))
            if cmd == CENX4_APP_ABLETON_SYSEX_RESYNC:
                self._ab_device_component.send_params()

    def update_display(self):
        super(Cenx4, self).update_display()

        Cenx4Mgr.update_display()

        # Telnet

        #Keep trying to accept a connection until someone actually connects
        if not self.telnetConnection:
            try:
                #Does anyone want to connect?
                self.telnetConnection, self.addr = self.telnetSocket.accept()
            except:
                #No one connected in this iteration
                pass
            else:
                #Yay! Someone connected! Send them the banner and first prompt.
                self.telnetConnection.send("Welcome to the Ableton Live Python Interpreter\r\n")
                self.telnetConnection.send("Brought to by LiveAPI.org\r\n")
                self.telnetConnection.send(">>> ")
        else:
            #Someone's connected, so lets interact with them.
            try:
                #If the client has typed anything, get it
                data = self.telnetConnection.recv(1)
            except:
                #Nope they haven't typed anything yet
                data = "" #

            #If return is pressed, process the command (This if statement is so ugly because ableton python doesn't have universal newlines)
            if (data == "\n" or data == "\r") and (self.lastData != "\n" and self.lastData != "\r"):
                continues = self.interpreter.push(self.telnetBuffer.rstrip()) #should be strip("/r/n") but ableton python throws an error
                self.commandBuffer.append(self.telnetBuffer.rstrip())
                self.telnetBuffer = ""

                #if the user input is multi-line, continue, otherwise return the results

                if continues:
                    self.telnetConnection.send("... ")
                else:
                    #return stdout to the client
                    self.telnetConnection.send(self.stdout.getvalue().replace("\n","\r\n"))
                    #return stderr to the client
                    self.telnetConnection.send(self.stderr.getvalue().replace("\n","\r\n"))
                    self.telnetConnection.send(">>> ")

                #Empty buffers by creating new stringIO objects
                #There's probably a better way to empty these
                self.stdin.close()
                self.stdout.close()
                self.stderr.close()
                self.stdin = StringIO.StringIO()
                self.stdout = StringIO.StringIO()
                self.stderr = StringIO.StringIO()
                #re-redirect the stdio
                sys.stdin = self.stdin
                sys.stdout = self.stdout
                sys.stderr = self.stderr


            elif data == "\b": #deals with backspaces
                if len(self.telnetBuffer):
                    self.telnetBuffer = self.telnetBuffer[:-1]
                    self.telnetConnection.send(" \b") #deletes the character on the console
                else:
                    self.telnetConnection.send(" ")
            elif data != "\n" and data != "\r":
                self.telnetBuffer = self.telnetBuffer + data
            self.lastData = data
