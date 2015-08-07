#!/usr/bin/env python

import math
import re
import sys


if __name__ == '__main__':
	
	#check in put arguments
	if len(sys.argv) <= 1:
		print("Usage: bin2h <input-file> [output-file]")
		exit(0);
			
	#set input and output filename
	in_name = sys.argv[1]
	if len(sys.argv) > 2:
		out_name = sys.argv[2].rsplit('.h',1)[0]
	else:
		out_name = sys.argv[1]
	
	#read data from input file
	fp = open(in_name, 'rb');
	data  = ['%02X' % ord(d) for d in fp.read()] #convert to hex-text
	data += ['00'] * (int(4*math.ceil(len(data)/4.0)) - len(data)) #padding
	data  = ['0x' + ''.join(data[i:i+4]) for i in range(0, len(data), 4)]
	data_lines = [', '.join(data[i:i+4]) for i in range(0, len(data), 4)]
	fp.close()
	
	#write data to output header file
	varname = re.sub('[^a-zA-Z_0-9]', '_', out_name).lower()
	fp = open(out_name + '.h', 'w')
	fp.write("#ifndef __%s_H\n"   % (varname.upper()))
	fp.write("#define __%s_H\n\n" % (varname.upper()))
	fp.write("unsigned int %s[%d] = {\n" % (varname, len(data)))
	fp.write("%s" % ',\n'.join(data_lines))
	fp.write("\n};\n\n#endif\n\n")
	fp.close()
	