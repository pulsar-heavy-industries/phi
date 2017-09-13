# This sucky script creates the LCD "VU Meter" lookup table
chars_len = 8
steps = chars_len * 5
tbl = []
for i in range(steps):
    #print i / 5, i % 5
    b = chr(5) * (i / 5)
    b += chr(1 + (i % 5))
    b += ' ' * (chars_len - len(b))
    print '{},'.format(repr(b).replace('\'', '"'))
