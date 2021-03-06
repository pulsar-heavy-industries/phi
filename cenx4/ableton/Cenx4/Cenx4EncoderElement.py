import Live
from _Framework.EncoderElement import EncoderElement
from _Framework.InputControlElement import *
import time


class Cenx4EncoderElement(EncoderElement):
    def __init__(self, cenx4, type, ch, msg, report=False):
        super(Cenx4EncoderElement, self).__init__(type, ch, msg, Live.MidiMap.MapMode.relative_smooth_binary_offset)
        self.cenx4 = cenx4
        self._report_name_val = report
        self._last_time = 0

    def connect_to(self, param):
        self.cenx4.log('Cenx4EncoderElement connect to %r -> %r' % (param.name, self._msg_identifier))
        if self._parameter_to_map_to is not None:
            try:
                self._parameter_to_map_to.remove_value_listener(self._on_value_changed)
            except:
                pass
        super(Cenx4EncoderElement, self).connect_to(param)

        if param is not None:
            if not self._parameter_to_map_to.value_has_listener(self._on_value_changed):
                self._parameter_to_map_to.add_value_listener(self._on_value_changed)

        self._send_param_val(True)

    def release_parameter(self):
        try:
            self._parameter_to_map_to.remove_value_listener(self._on_value_changed)
        except:
            pass

        super(Cenx4EncoderElement, self).release_parameter()

        self._send_param_val(True)

    def disconnect(self):
        if self._parameter_to_map_to is not None:
            self._parameter_to_map_to.remove_value_listener(self._on_value_changed)
        super(Cenx4EncoderElement, self).disconnect()

    def _on_value_changed(self):
        if self._report_name_val:
            self._send_param_val()

    def _send_param_val(self, force = False):
        if self._parameter_to_map_to is not None:
            if hasattr(self, 'select_parameter'):
                if self._report_name_val:
                    self.select_parameter(self._parameter_to_map_to)

        if hasattr(self._parameter_to_map_to, 'name'):
            # Remember, value gets sent as CC
            self.cenx4.log('Cenx4EncoderElement _send_param_val %r :: %s :: %s :: %r' % (self._msg_identifier, unicode(self._parameter_to_map_to), self._parameter_to_map_to.name, self._parameter_to_map_to.value))

            param = self._parameter_to_map_to
            pot_num = self._msg_identifier # this is the cc number
            if pot_num < self.cenx4.cfg.num_pots:
                # Real-time throttled update
                if time.clock() > self._last_time + 0.01 or force:
                    self.cenx4.sysex.set_pot_all_scaled(pot_num, param.value, param.min, param.max, param.name, unicode(param))
                    self._last_time = time.clock()

                # Periodic update, to catch the last value that sometimes gets skipped from the realtime
                # update
                self.cenx4.update_pot(pot_num, param)

    def settings(self):
        self.cenx4.log('Cenx4EncoderElement.settings called!')

        if self._parameter_to_map_to is not None:
            parent = self._parameter_to_map_to.canonical_parent
            if not hasattr(parent, 'name'):
                parent = parent.canonical_parent
            return [unicode(parent.name), unicode(self._parameter_to_map_to.name)]
        else:
            return ['', '']
