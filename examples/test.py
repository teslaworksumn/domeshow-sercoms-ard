import argparse
import atexit
import crc16
import serial
import sys
import time
import threading
import traceback

magicpacket = [0xde, 0xad, 0xbe, 0xef]

parser = argparse.ArgumentParser()
parser.add_argument("-p", "--port", help="Serial port name")
parser.add_argument("-b", "--baud", help="Baud rate for serial port", type=int)
args = parser.parse_args()

portname = args.port
baud = args.baud

serial_reader_run = True
def serial_reader():
	flush = True
	while (serial_reader_run):
		try:
			#print(port.inWaiting())
			if port.inWaiting() > 0:
				flush = True
				inbytes = port.read()
				for b in inbytes:
					if b >= 0x20 and b <= 0x255:
						sys.stdout.write(chr(b));
					elif b == 0x00:
						sys.stdout.write("{NUL}")
					elif b == 0x01:
						sys.stdout.write("{SOH}")
					elif b == 0x02:
						sys.stdout.write("{STX}")
					elif b == 0x03:
						sys.stdout.write("{ETX}")
					elif b == 0x04:
						sys.stdout.write("{EOT}")
					elif b == 0x05:
						sys.stdout.write("{ENQ}")
					elif b == 0x06:
						sys.stdout.write("{ACK}")
					elif b == 0x07:
						sys.stdout.write("{BEL}")
					elif b == 0x08:
						sys.stdout.write("{BS}")
					elif b == 0x09:
						sys.stdout.write("\t")
					elif b == 0x0a:
						sys.stdout.write("\n")
					elif b == 0x0b:
						sys.stdout.write("\v")
					elif b == 0x0c:
						sys.stdout.write("\f")
					elif b == 0x0d:
						sys.stdout.write("\r")
					elif b == 0x0e:
						sys.stdout.write("{SO}")
					elif b == 0x0f:
						sys.stdout.write("{SI}")
					elif b == 0x10:
						sys.stdout.write("{DLE}")
					elif b == 0x11:
						sys.stdout.write("{DC1}")
					elif b == 0x12:
						sys.stdout.write("{DC2}")
					elif b == 0x13:
						sys.stdout.write("{DC3}")
					elif b == 0x14:
						sys.stdout.write("{DC4}")
					elif b == 0x15:
						sys.stdout.write("{NAK}")
					elif b == 0x16:
						sys.stdout.write("{SYN}")
					elif b == 0x17:
						sys.stdout.write("{ETB}")
					elif b == 0x18:
						sys.stdout.write("{CAN}")
					elif b == 0x19:
						sys.stdout.write("{EM}")
					elif b == 0x1a:
						sys.stdout.write("{SUB}")
					elif b == 0x1b:
						sys.stdout.write("{ESC}")
					elif b == 0x1c:
						sys.stdout.write("{FS}")
					elif b == 0x1d:
						sys.stdout.write("{GS}")
					elif b == 0x1e:
						sys.stdout.write("{RS}")
					elif b == 0x1f:
						sys.stdout.write("{US}")
					else:
						sys.stdout.write("{WTF}")
			else:
				if flush:
					sys.stdout.flush()
					flush = False
			time.sleep(0.05)
		except SystemExit:
			sys.stdout.write('SERIAL exiting due to SystemExit\n')
			break
		except OSError:
			e = sys.exc_info()
			sys.stderr.write('SERIAL: {0}: {1}\n'.format(e[0],e[1]))
			traceback.print_tb(e[2])
			break
		except:
			e = sys.exc_info()
			sys.stderr.write('SERIAL: {0}: {1}\n'.format(e[0],e[1]))
			traceback.print_tb(e[2])

port = serial.Serial(port=portname, baudrate=baud, rtscts=True, timeout=3)
sys.stdout.write("SERIAL: opened {0} @ {1}\n".format(portname, baud))
serial_thread = threading.Thread(target=serial_reader, args=[])
serial_thread.start()

def allstop():
	global serial_reader_run
	sys.stdout.write("Allstop called\n")
	serial_reader_run = False
	serial_thread.join()
	port.close()
def ehook(type, value, traceback):
	sys.stderr.write("I found an ehook!\n")
	if type is SystemExit:
		allstop()
		sys.__excepthook__(type, value, traceback)
	else:
		sys.__excepthook__(type, value, traceback)
sys.excepthook = ehook

def highlow16(val):
	h = (val>>8)&0xff
	l = val & 0xff
	return (h,l)

def sendmagic():
	if port.is_open:
		port.write(bytearray(magicpacket))
	else:
		sys.stderr.write("SERIAL: Port cannot write as it is closed\n")

def senddata(data):
	dl = len(data)
	bad = bytes(data)
	crc = crc16.crc16xmodem(bad)
	crchl = highlow16(crc)
	if dl > 4096:
		raise IndexError("Data length must be <=4096 ({1} found)".format(dl))
	dlhl = highlow16(dl)
	sendmagic()
	d2s = [dlhl[0],dlhl[1]]+data+[crchl[0],crchl[1]]
	port.write(bytes(d2s))

