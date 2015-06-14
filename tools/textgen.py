#!/usr/bin/env python

lines = [
    ' # # #  # # #  # # #    ### #  # ###   ##   ###  ### #### ###     ###  ##   ###  ',
    ' # # #  # # #  # # #   #    #  # #  # #  # #    #    #    #  #   #    #  # # # # ',
    ' # # #  # # #  # # #    ##  ###  ###  ####  ##   ##  ###  ###    #    #  # # # # ',
    ' # # #  # # #  # # #      # #  # #  # #  #    #    # #    #  #   #    #  # # # # ',
    '  # #    # #    # #  # ###  #  # #  # #  # ###  ###  #### #  # #  ###  ##  # # # ',
]

# Registers with right values for colors
black = 'r1'
white = 'r5'

# Output port
port = 'PORTD'

i = 0
print '''
#include "pins.i"
'''

for line in lines:
    print '\n\t; %d cycles including ret' % (len(line) + 4)
    print '\t.global txt_line%d' % i
    print 'txt_line%d:' % i
    for ch in line:
        if ch == ' ':
            color = black
        else:
            color = white
        print '\tout %s, %s' % (port, color)
    print '\tret'
    i += 1
