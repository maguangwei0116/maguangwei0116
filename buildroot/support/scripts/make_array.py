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
            f.write("static const unsigned char card_buf[] = \n{\n    ")
            for d in all:
                e = "0x%s, " % binascii.b2a_hex(d)
                f.write(e)
                i = i + 1
                h = i % 16
                if h == 0:
                    f.write("\n    ")
            
            if (i % 16) == 0:
                f.seek(-4, 2)
                f.write("};\n")
            else:
                f.seek(-1, 2)
                f.write("\n};\n")
            f.close()
