from _Framework.ChannelStripComponent import ChannelStripComponent
from Cenx4ChannelDeviceComponent import Cenx4ChannelDeviceComponent
from Cenx4Mgr import Cenx4Mgr


class Cenx4ChannelStripComponent(ChannelStripComponent):
    def __init__(self, id):
        super(Cenx4ChannelStripComponent, self).__init__()
        self._track_id = id
        self._device = Cenx4ChannelDeviceComponent()
        Cenx4Mgr.log_message('Cenx4ChannelStripComponent init %r' % (id, ))

    def _on_selected_device_changed(self):
        Cenx4Mgr.log_message('def _on_selected_device_changed(self):')
