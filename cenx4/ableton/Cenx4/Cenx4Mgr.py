class Config(object):
    num_pots = 8

class Cenx4Mgr(object):
    cfg = Config()

    @classmethod
    def set_log(cls, log_message):
        cls.log = cls.log_message = log_message

