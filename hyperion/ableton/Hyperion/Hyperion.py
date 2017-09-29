from __future__ import with_statement #compatibility for Live 9, need to be written at the first line of the script

import sys
#sys.path.append('/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7')
import StringIO, socket, code

import Live #you import Live, in order to be able to use its components
from _Framework.ControlSurface import ControlSurface
from _Framework.ButtonElement import ButtonElement
from _Framework.EncoderElement import EncoderElement
from _Framework.SliderElement import SliderElement
from _Framework.InputControlElement import *
from _Framework.CompoundComponent import CompoundComponent
from _Framework.ChannelStripComponent import ChannelStripComponent, release_control
from VUMeter import VUMeter




IS_MOMENTARY=True

a=  None
b = None

class HyperionChan(CompoundComponent):
    def __init__(self, hyperion, midi_ch, *a, **kw):
        super(HyperionChan, self).__init__(*a, **kw)

        self.hyperion = hyperion
        self.midi_ch = midi_ch

        def make_button(midi_ch, identifier, name, midi_type=MIDI_CC_TYPE):
            return ButtonElement(True, midi_type, midi_ch, identifier, name=name)

        ids = [[0x29, 0x2a], [0x2b, 0x2c]]

        self._left = make_button(1, ids[midi_ch][0], 'Left{}'.format(midi_ch), MIDI_NOTE_TYPE)
        self._left.add_value_listener(self._left_right_button, identify_sender=True)

        self._right = make_button(1, ids[midi_ch][1], 'Right{}'.format(midi_ch), MIDI_NOTE_TYPE) # 2 is hack for launchcontrol midi ch
        self._right.add_value_listener(self._left_right_button, identify_sender=True)

        self._fader = SliderElement(MIDI_CC_TYPE, 1, [0x4d, 0x4f][midi_ch]) # 1st slider, 3rd slider
        self.pots = [
            EncoderElement(MIDI_CC_TYPE, 1, [0x0d, 0x0f][midi_ch], Live.MidiMap.MapMode.absolute, name='Pot1'),
            EncoderElement(MIDI_CC_TYPE, 1, [0x1d, 0x1f][midi_ch], Live.MidiMap.MapMode.absolute, name='Pot2'),
        ]

        #self._cs = ChannelStripComponent()
        #self._cs.set_volume_control(self._fader)
        #self._vu = VUMeter(self)

        self._track = None
        self._bind_to_track(self.hyperion.song().tracks[0])

    def log(self, msg, *args):
        self.hyperion.log_message(('HyperionChan({}): ' + msg).format(self.midi_ch, *args))

    def disconnect(self):
        super(HyperionChan, self).disconnect()

        self._left.remove_value_listener(self._left_right_button)
        self._right.remove_value_listener(self._left_right_button)

    def _get_track_mapper_device(self, track):
        for device in track.devices:
            if device.name == 'MultiMapper16 V2.0':
                return device

    def _get_all_tracks(self, all_tracks):
        got_tracks = []

        for cur_track in all_tracks:
            if isinstance(cur_track, (Live.Track.Track, Live.DrumChain.DrumChain)):
                got_tracks.append(cur_track)

            devices = list(cur_track.devices)
            if len(devices) and isinstance(devices[0], Live.RackDevice.RackDevice):
                got_tracks.extend(self._get_all_tracks(devices[0].chains))

        return got_tracks

    def _left_right_button(self, value, sender):
        if value == 0:
            return

        tracks = self._get_all_tracks(self.hyperion.song().tracks)
        tracks = [track for track in tracks if self._get_track_mapper_device(track)]

        for t in tracks:
            self.log('AAAAA {}', t.name)

        # tracks = list(self.hyperion.song().tracks)
        try:
            cur_track_idx = tracks.index(self._track)
        except ValueError:
            self._bind_to_track(self._track)
        else:
            cur_track_idx += 1
            if cur_track_idx == len(tracks):
                cur_track_idx = 0
            self._bind_to_track(tracks[cur_track_idx])

    def _bind_to_track(self, track):
        if self._track:
            #self._cs.set_track(None)
            self._fader.release_parameter()
            [pot.release_parameter() for pot in self.pots]

            self._track = None

        if not track:
            return

        self.log('binding to {}', track.name)
        self._track = track

        self._fader.connect_to(track.mixer_device.volume)

        mapper_dev = self._get_track_mapper_device(track)
        self.pots[0].connect_to(mapper_dev.parameters[3])
        self.pots[1].connect_to(mapper_dev.parameters[4])

        # self._cs.set_track(track)

        # if self._track.has_audio_output:
        #     self._vu.set_vu_meter(track, self._fader)
        # else:
        #     self._vu.set_vu_meter(None, None)

class Hyperion(ControlSurface):
    def __init__(self, c_instance):
        ControlSurface.__init__(self,c_instance)
        with self.component_guard():
            self.__c_instance = c_instance
            self.hyperion_chans = [HyperionChan(self, ch) for ch in range(2)]

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


        global a, b
        a =self
        b = self.__c_instance




    def disconnect(self):
        ControlSurface.disconnect(self)

        [chan.disconnect() for chan in self.hyperion_chans]

        sys.stdin = self.originalstdin
        sys.stdout = self.originalstdout
        sys.stderr = self.originalstderr
        self.telnetSocket.close()

    def update_display(self):
        ControlSurface.update_display(self)

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
