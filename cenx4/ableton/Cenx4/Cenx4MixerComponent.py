from _Framework.MixerComponent import MixerComponent
from Cenx4ChannelStripComponent import Cenx4ChannelStripComponent
from Cenx4Mgr import Cenx4Mgr


class Cenx4MixerComponent(MixerComponent):
        def __init__(self, num_tracks):
            super(Cenx4MixerComponent, self).__init__(num_tracks)

        def _create_strip(self):
            Cenx4Mgr.log_message('_create_strip called')
            return Cenx4ChannelStripComponent(len(self._channel_strips))
