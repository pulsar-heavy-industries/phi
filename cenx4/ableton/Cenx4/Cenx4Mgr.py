class Config(object):
    num_pots = 8

pots_data = []
for x in range(255):
    pots_data.append({
        'name': 'NAME?',
        'value_str': '?!',
        'value': 0,
        'value_min': 0,
        'value_max': 0,
        'needs_update': x < Config.num_pots,
    })

class Cenx4Mgr(object):
    cfg = Config()

    @classmethod
    def set_log(cls, log_message):
        cls.log = cls.log_message = log_message

    @classmethod
    def update_pot(cls, pot_num, param):
        pots_data[pot_num] = {
            'name': param.name,
            'value_str': unicode(param),
            'value': param.value,
            'value_min': param.min,
            'value_max': param.max,
            'needs_update': True,
        }

    @classmethod
    def update_display(cls):
        for pot_num, data in enumerate(pots_data):
            if data['needs_update']:
                cls.sysex.set_pot_text(pot_num, data['name'], data['value_str'])
                cls.sysex.set_pot_val_scaled(pot_num, data['value'], data['value_min'], data['value_max'])
                data['needs_update'] = False

