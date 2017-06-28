import sys

f = open(sys.argv[1], 'r')
for line in f.readlines():
    line = line.rstrip()
    if line.endswith('\\'):
        line = line[:-1].rstrip().ljust(77) + '\\'
    print line
