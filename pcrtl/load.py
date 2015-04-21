#!C:/cygwin/Python27/python
import sys, os, serial, time

if (os.name == 'nt') :
	port = int(sys.stdin.readline())
else :
	port = ("/dev/ttyUSB" + sys.stdin.readline())[:-1]

try :
	ser = serial.Serial(port, 9600, timeout=1)
except serial.serialutil.SerialException:
	print('-')
	exit(0)

print "connected..."
time.sleep(2.5)
line = ser.readline()
while(len(line) > 0) :
	time.sleep(0.3)
	line = ser.readline()

while(1) :
	u = sys.stdin.readline()
        if (u[0] == 'x') :
		print('Finished')
		ser.close()
		exit()
	if (u[0] == 'd') :
		print('Printing stored PCR schedule:')
	ser.write(u[:-1])
	ser.write("\n")
	time.sleep(0.2)
	line = ser.readline()
	while(len(line) > 0) :
		print(line[:-1])
		time.sleep(0.2)
		line = ser.readline()
	if (u[0] != 'd') :
		print('load [' + u[:-1] + ']')
        
    
