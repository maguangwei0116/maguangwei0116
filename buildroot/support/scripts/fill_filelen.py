#! /usr/bin/python

import struct
import sys
import argparse
import os


def generate_image(fname, imaged):
	f = open(fname, "wb+")
	f.write(imaged)
	f.close()
	print "generate_image to " + fname 

if __name__ == "__main__":
	
	# - optional
	parser = argparse.ArgumentParser()
	#parser.add_argument("-xip", help="use sxip or not", action="store_true", default=False)
	parser.add_argument("-dump", help="dump scd C header", action="store_true", default=False)
	parser.add_argument("fname", type=str, help="filename")
	parser.parse_args()
	args = parser.parse_args()

	if os.path.isfile(args.fname):
		f = open(args.fname, "rb")
		fdata = f.read()
		f.close()
	else:
		print "!!file: " + args.fname + " not exist"
		sys.exit(1)
	
	
	file_len = len(fdata);
	#file_len = hex(file_len);	
	#file_len = bytes(file_len);
	bytes = struct.pack('i',file_len)
	fdata = fdata[:0x108] + bytes + fdata[0x10c:];
	
	#fdata[0x108] = (file_len>>0) & 0xff;
	#fdata[0x109] = (file_len>>8) & 0xff;
	#fdata[0x10a] = (file_len>>16) & 0xff;
	#fdata[0x10b] = (file_len>>24) & 0xff;
	
	generate_image(args.fname+".tmp", fdata)
	sys.exit(0)
