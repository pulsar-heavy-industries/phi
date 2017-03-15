from _Framework.DeviceComponent import DeviceComponent
from Cenx4Mgr import Cenx4Mgr


class Cenx4ChannelDeviceComponent(DeviceComponent):
    def __init__(self):
        super(Cenx4ChannelDeviceComponent, self).__init__()
        self._device_banks = {}

    def _assign_parameters(self):
        Cenx4Mgr.log_message('def _assign_parameters(self): called')
