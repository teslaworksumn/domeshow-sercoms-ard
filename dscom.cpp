#include "dscom.h"

void DSCom::read(HardwareSerial &s) {
    switch (state) {
        case DSCOM_STATE_READY:
            #ifdef DEBUG
                if (!messagewalk) s.println("READY");
            #endif
            messagewalk = true;
            //cts();
            if (s.available() > 1) {
                #ifdef DEBUG
                    s.print("Peek: ");
                    char b = s.peek();
                    s.println(b);
                #endif
                if (s.read() == magic[magic_status]) {
                    magic_status++;
                } else {
                    magic_status=0;
                }
                #ifdef DEBUG
                    s.print("Magic status: ");
                    s.println(magic_status);

                #endif
                if (magic_status >= DSCOM_MAGIC_LENGTH) {
                    state = DSCOM_STATE_READING;
                    messagewalk = false;
                }
            }
            break;
        case DSCOM_STATE_READING:
            #ifdef DEBUG
                if (!messagewalk) {
                    s.println("READING");
                }
            #endif
            messagewalk = true;
            if (s.available() > 2) {
                uint16_t len = getTwoBytesSerial(s);
                #ifdef DEBUG
                    s.print("Length: ");
                    s.println(len);
                #endif
                readData(s, len);
                #ifdef DEBUG
                    s.println("Data read");
                #endif
                // Update len for future use (writing)
                //data_len = len;
                uint16_t packetCrc = getTwoBytesSerial(s);
                uint16_t calculatedCrc = crc.XModemCrc(new_data, 0, len);
                #ifdef DEBUG
                    s.print("Calculated CRC: ");
                    s.println(calculatedCrc, HEX);
                    s.print("Received CRC: ");
                    s.println(packetCrc, HEX);
                #endif
                messagewalk = false;
                if (calculatedCrc != packetCrc)
                {
                    #ifdef DEBUG
                        s.println("CRC doesn't match");
                    #endif
                    state = DSCOM_STATE_READY;
                }
                else
                {
                    state = DSCOM_STATE_APPLY;
                }
            }
            break;
        case DSCOM_STATE_APPLY:
            #ifdef DEBUG
                if (!messagewalk) s.println("APPLY");
            #endif
            messagewalk = false;
            uint8_t* old_data;
            old_data = data;
            data = new_data;
            data_len = new_data_len;
            free(old_data);
            #ifdef DEBUG
                s.println("Done applying");
            #endif
            updated = true;
            state = DSCOM_STATE_READY;
            break;
        default:
            state = DSCOM_STATE_READY;
            messagewalk = false;
            break;
    }
}

uint16_t DSCom::getTwoBytesSerial(HardwareSerial &s) {
    // Wait for serial bytes
    while (s.available() < 2) {}
    uint16_t high = s.read() << 8;
    uint16_t low = s.read();
    uint16_t combined = high | low;
    return combined;
}

void DSCom::readData(HardwareSerial &s, uint16_t len) {
    new_data_len = len;
    new_data = malloc(sizeof(uint8_t)*len);
    while (s.available() < len) {}
    
    // Read in data from serial
    s.readBytes(new_data, len);
    
}

uint8_t* DSCom::getData() {
    return data;
}