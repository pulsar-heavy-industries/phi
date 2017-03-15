from __future__ import with_statement
from _Framework.ControlSurface import ControlSurface
from _Framework.InputControlElement import *
from Cenx4DeviceComponent import Cenx4DeviceComponent
from Cenx4Mgr import Cenx4Mgr
from MIDIProtocol import (
    SysExProtocol,
    CENX4_MAIN_MIDI_SYSEX_APP_CMD,
    CENX4_APP_ABLETON_SYSEX_RESYNC,
)

from Cenx4EncoderElement import Cenx4EncoderElement


#from PHITelnet import PHITelnetMixin
#class Cenx4(ControlSurface, PHITelnetMixin):
class Cenx4(ControlSurface):
    def __init__(self, c_instance):
        self.log = self.log_message

        super(Cenx4, self).__init__(c_instance)

        with self.component_guard():
            self.__c_instance = c_instance
            self.do_setup()

        self.log('Cenx4 Control Surface init success')

    def suggest_input_port(self):
        return 'CENX4 MIDI'

    def suggest_output_port(self):
        return 'CENX4 MIDI'

    def do_setup(self):
        Cenx4Mgr.set_log(self.log)
        Cenx4Mgr.send_midi = self.__c_instance.send_midi
        Cenx4Mgr.sysex = SysExProtocol(self.__c_instance.send_midi)

        self._ab_device_component = Cenx4DeviceComponent()
        self.set_device_component(self._ab_device_component)

        # TODO hardcoded 16
        device_controls = [Cenx4EncoderElement(MIDI_CC_TYPE, Cenx4Mgr.cfg.midi_ch, i, True) for i in range(16)]
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
