from __future__ import with_statement
from _Framework.ControlSurface import ControlSurface
from _Framework.InputControlElement import *
from Cenx4DeviceComponent import Cenx4DeviceComponent
from MIDIProtocol import (
    SysExProtocol,
    CENX4_MAIN_MIDI_SYSEX_APP_CMD,
    CENX4_APP_ABLETON_SYSEX_RESYNC,
)

from Cenx4EncoderElement import Cenx4EncoderElement


class Config(object):
    num_pots = 8
    midi_ch = 2


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
        self.send_midi = self.__c_instance.send_midi
        self.sysex = SysExProtocol(self)
        self.cfg = Config()

        self.pots_data = []
        for x in range(255):
            self.pots_data.append({
                'name': 'NAME?',
                'value_str': '?!',
                'value': 0,
                'value_min': 0,
                'value_max': 0,
                'needs_update': x < self.cfg.num_pots,
            })

        self._ab_device_component = Cenx4DeviceComponent(self)
        self.set_device_component(self._ab_device_component)

        # TODO hardcoded 16
        device_controls = [Cenx4EncoderElement(self, MIDI_CC_TYPE, self.cfg.midi_ch, i, True) for i in range(16)]
        self._ab_device_component.set_parameter_controls(tuple(device_controls))

    def handle_sysex(self, midi_bytes):
        data = self.sysex.decode(midi_bytes)
        if data is None:
            return super(Cenx4, self).handle_sysex(midi_bytes)

        if len(data) >= 2 and data[0] == CENX4_MAIN_MIDI_SYSEX_APP_CMD:
            cmd, data = data[1], data[2:]
            self.log(('Incoming app sysex cmd', cmd, data))
            if cmd == CENX4_APP_ABLETON_SYSEX_RESYNC:
                self._ab_device_component.send_params()

    def update_display(self):
        super(Cenx4, self).update_display()

        for pot_num, data in enumerate(self.pots_data):
            if data['needs_update']:
                self.sysex.set_pot_all_scaled(pot_num, data['value'], data['value_min'], data['value_max'], data['name'], data['value_str'])
                data['needs_update'] = False



    def update_pot(self, pot_num, param):
        self.pots_data[pot_num] = {
            'name': param.name,
            'value_str': unicode(param),
            'value': param.value,
            'value_min': param.min,
            'value_max': param.max,
            'needs_update': True,
        }
