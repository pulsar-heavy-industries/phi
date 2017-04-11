from __future__ import with_statement

import Live
import time
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
    num_pots = 4
    midi_ch = 2
    enable_banks_encoder = False


#from PHITelnet import PHITelnetMixin
#class Cenx4(ControlSurface, PHITelnetMixin):
class Cenx4(ControlSurface):
    def __init__(self, c_instance):
        self.log = self.log_message
        self.resynced = False
        self.last_resync_at = time.clock()

        super(Cenx4, self).__init__(c_instance)

        with self.component_guard():
            self.__c_instance = c_instance
            self._do_setup()

        self.log('Cenx4 Control Surface init success')

    def suggest_input_port(self):
        return 'CENX4 MIDI'

    def suggest_output_port(self):
        return 'CENX4 MIDI'

    def _do_setup(self):
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

    def refresh_state(self):
        super(Cenx4, self).refresh_state()

        self.log('refresh_state!')
        self.sysex.send_resync()

    def receive_midi(self, midi_bytes):
        self.log(('IN MIDI', midi_bytes))

        # CC on our selected channel
        cc = 0xb0 | self.cfg.midi_ch

        # Check for bank encoder
        if ((len(midi_bytes) == 3) and
            (midi_bytes[0] == cc) and
            (midi_bytes[1] == self.cfg.num_pots - 1) and
            self.cfg.enable_banks_encoder):

            if midi_bytes[2] == 63:
                direction = -1
            elif midi_bytes[2] == 65:
                direction = 1
            else:
                direction = 0
            if direction:
                self._ab_device_component._banks_encoder(direction)
            return


        super(Cenx4, self).receive_midi(midi_bytes)

    def build_midi_map(self, midi_map_handle):
        self.log('BUILD MIDI MAP')

        if self.cfg.enable_banks_encoder:
            script_handle = self.__c_instance.handle()
            Live.MidiMap.forward_midi_cc(script_handle, midi_map_handle, self.cfg.midi_ch, self.cfg.num_pots - 1)

        super(Cenx4, self).build_midi_map(midi_map_handle)

    def handle_sysex(self, midi_bytes):
        data = self.sysex.decode(midi_bytes)
        if data is None:
            return super(Cenx4, self).handle_sysex(midi_bytes)

        if len(data) >= 2 and data[0] == CENX4_MAIN_MIDI_SYSEX_APP_CMD:
            cmd, data = data[1], data[2:]
            if cmd == CENX4_APP_ABLETON_SYSEX_RESYNC:
                num_modules, enable_banks_encoder = data

                self.cfg.num_pots = num_modules * 4
                self.cfg.enable_banks_encoder = bool(enable_banks_encoder)

                self.log(('RESYNC', data))
                self.resynced = True

                self.__c_instance.request_rebuild_midi_map()

                pots_map = {
                    4: [0, 1, 2, 3],
                    8: [
                        # The order of the first 8 parameters matters due to the popular instrument rack
                        # macros arrangement. Other pots don't matter as much.
                        0, 2, 4, 6,
                        1, 3, 5, 7,
                    ],
                    12: [
                        0, 2, 4, 6,
                        1, 3, 5, 7,
                        8, 9, 10, 11,
                    ],
                    16 : [
                        0, 2, 4, 6,
                        1, 3, 5, 7,
                        8, 9, 10, 11,
                        12, 13, 14, 15,
                    ],
                }.get(self.cfg.num_pots, list(range(self.cfg.num_pots)))
                self.log(('Pots map', pots_map))
                device_controls = [Cenx4EncoderElement(self, MIDI_CC_TYPE, self.cfg.midi_ch, i, True) for i in range(self.cfg.num_pots)]

                avail_params = len(device_controls)
                if self.cfg.enable_banks_encoder:
                    avail_params -= 1

                self._ab_device_component.set_parameter_controls(tuple(
                    device_controls[pots_map[idx]] for idx in range(avail_params)
                ))
                self._ab_device_component._assign_parameters()
                self._ab_device_component._reset_banks()
                #self._ab_device_component.send_params()

            else:
                self.log(('Incoming app sysex cmd', cmd, data))

    def update_display(self):
        super(Cenx4, self).update_display()

        for pot_num, data in enumerate(self.pots_data):
            if data['needs_update']:
                self.sysex.set_pot_all_scaled(pot_num, data['value'], data['value_min'], data['value_max'], data['name'], data['value_str'])
                data['needs_update'] = False

        if not self.resynced:
            if time.clock() > self.last_resync_at + 0.1:
                self.sysex.send_resync()
                self.last_resync_at = time.clock()

    def update_pot(self, pot_num, param):
        self.pots_data[pot_num] = {
            'name': param.name,
            'value_str': unicode(param),
            'value': param.value,
            'value_min': param.min,
            'value_max': param.max,
            'needs_update': True,
        }
