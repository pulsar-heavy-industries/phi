from _Framework.SessionComponent import SessionComponent
from Cenx4ChannelStripComponent import Cenx4ChannelStripComponent


class Cenx4SessionComponent(SessionComponent):
     def __init__(self, num_tracks, num_scenes):
         super(Cenx4SessionComponent, self).__init__(num_tracks, num_scenes)
