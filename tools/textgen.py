#!/usr/bin/env python

lines = [
    #               |    |    |    :    |    I    |    :    |    |    |
    ' # # #  # # #  # # #    ### # .#.###:::##:::###..### #### ###     ###  ##   ###  ',
    ' # # #  # # #  #.#.#...#....# :#:#:;#;#;;#;#;;::#::..#....#..#...#    #  # # # # ',
    ' # # #  # # #  #.#.#....##..###::###;;####;;##:::##..###..###....#    #  # # # # ',
    ' # # #  # # #  #.#.#......#.#.:#:#:;#;#;;#;;;;#::::#.#....#..#...#    #  # # # # ',
    '  # #    # #    # #  # ###  # .#.#.:#:#::#:###..###  #### #  # #  ###  ##  # # # ',
    #                                        ^--- middle
]

diamond = [
    '     .     ',
    '   .:;:.   ',
    ' ..:;#;:.. ',
    '   .:;:.   ',
    '     .     ',
]

# Registers with right values for colors
colmap = {
    ' ': 'r1',	# black
    '#': 'r5',	# white
    '.': 'r6',	# dark green
    ':': 'r7',	# medium/dark green
    ';': 'r8',	# medium green
}

# Output port
port = 'PORTD'

i = 0
print '''
#include "pins.i"
'''

for line in lines:
    line = diamond[i] + line + diamond[i]
    print '\n\t; %d cycles including ret' % (len(line) + 4)
    print '\t.global txt_line%d' % i
    print 'txt_line%d:' % i
    for ch in line:
        color = colmap[ch]
        print '\tout %s, %s' % (port, color)
    print '\tret'
    i += 1
