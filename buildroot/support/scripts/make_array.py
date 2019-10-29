#! /usr/bin/python
# coding=utf-8

import numpy
import binascii
import sys

print sys.argv[0]
print sys.argv[1]
print sys.argv[2]

if __name__=='__main__':
    i = 0
    with open(sys.argv[1], 'rb') as f:
        all = f.read()

        with open(sys.argv[2], 'w') as f:
            f.write("static char card_buf[]={")
            f.write('\n')
            for d in all:
                e = "0x%s," % binascii.b2a_hex(d)
                f.write(e)
                i = i + 1
                h = i % 50
                if h == 0:
                    f.write('\n')
            f.seek(-1, 2)
            f.write("};")
            f.write('\n')
            f.close()
