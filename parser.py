import crc16
import serial
import sys
import time

magic = [0xde, 0xad, 0xbe, 0xef]

port = serial.Serial(sys.argv[1], int(sys.argv[2]))

state = 0
new_data = []
data = []
magic_status = 0
updated = False

debug=0
messagewalk=False;

def read():
    global state
    global new_data
    global data
    global magic_status
    global messagewalk
    global updated
    if state == 0: #DSCOM_STATE_READY
        if debug > 0:
            if not messagewalk:
                print("READY");
        messagewalk = True;
        if (port.in_waiting > 1):
            if debug >= 2:
                print("Peek: {0:02x} ({1:02x})".format(port.peek(), magic[magic_status]))
            r = port.read(size=1)[0]
            if (r == magic[magic_status]):
                magic_status += 1;
            else:
                magic_status = 0;
            if debug > 0:
                print("Magic status: {0}".format(magic_status))
            if (magic_status >= len(magic)):
                magic_status = 0
                state = 1 #DSCOM_STATE_READING
                messagewalk = False
    elif state == 1: #DSCOM_STATE_READING
        if debug > 0:
            if not messagewalk:
                print("READING");
        messagewalk = True;
        if (port.in_waiting > 2):
            dlen = getTwoBytesSerial()
            if debug >= 1:
                print("Length: {0}".format(dlen))
            readData(dlen)
            if debug >= 1:
                print("Data read");
            packetCrc = getTwoBytesSerial()
            calculatedCrc = crc16.crc16xmodem(new_data)
            if debug >= 1:
                sys.stdout.write("Calculated CRC: {0:02x}\nReceived CRC: {1:02x}\n".format(packetCrc, calculatedCrc))
            messagewalk = False
            if (calculatedCrc != packetCrc):
                if debug >= 1:
                    print("CRC doesn't match");
                state = 0 #DSCOM_STATE_READY;
            else:
                state = 2 #DSCOM_STATE_APPLY;
    elif state == 2: #DSCOM_STATE_APPLY
        if debug > 0:
            if not messagewalk:
                print("APPLY");
        messagewalk = False;
        data = new_data;
        if debug >= 2:
            print("Done applying")
        updated = True;
        state = 0 #DSCOM_STATE_READY;
    else:
        state = 0 #DSCOM_STATE_READY;
        messagewalk = False;

def write(out_data):
    #uint8_t len_high, len_low, crc_high, crc_low;
    data_crc = crc16.crc16xmodem(out_data)
    len_high, len_low = splitTwoBytes(len(out_data))
    crc_high, crc_low = splitTwoBytes(data_crc)
    port.write(magic)
    port.write(len_high)
    port.write(len_low)
    port.write(out_data)
    port.write(crc_high)
    port.write(crc_low)

def getTwoBytesSerial():
    # Wait for serial bytes
    while port.in_waiting < 2:
        pass
    high = port.read()[0] << 8
    low = port.read()[0]
    combined = high | low
    return combined

def splitTwoBytes(din):
    out_high = din >> 8
    out_low = din & 0xff
    return out_high, out_low

def readData(dlen):
    global new_data
    new_data = []
    while (port.in_waiting < dlen):
        pass
    # Read in data from serial
    new_data = port.read(size=dlen)

def getData():
    global updated
    updated = False
    return data;

def getDataLen():
    return len(data)

def isUpdated():
    global updated
    return updated

#MAIN
def main():
    while True:
        read();
        if isUpdated():
            for i in getData():
                sys.stdout.write("{0:02x} ".format(i))
            sys.stdout.write("\n".format(i))
    time.sleep(0.001)

if sys.flags.interactive:
    pass
else:
    main()