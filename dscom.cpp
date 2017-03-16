#include "dscom.h"

void DSCom::read() {
    switch (state) {
        case DSCOM_STATE_READY:
            #ifdef DSCOM_DEBUG
                if (!messagewalk) s->println("READY");
            #endif
            messagewalk = true;
            //cts();
            if (s->available() > 1) {
                #ifdef DSCOM_DEBUG_2
                    s->print("Peek: ");
                    s->print(s->peek(), HEX);
                    s->print(" (");
                    s->print(magic[magic_status],HEX);
                    s->println(")");
                #endif
                if (s->read() == magic[magic_status]) {
                    magic_status++;
                } else {
                    magic_status=0;
                }
                #ifdef DSCOM_DEBUG_1
                    s->print("Magic status: ");
                    s->println(magic_status);
                #endif
                if (magic_status >= DSCOM_MAGIC_LENGTH) {
                    magic_status=0;
                    state = DSCOM_STATE_READING;
                    messagewalk = false;
                }
            }
            break;
        case DSCOM_STATE_READING:
            #ifdef DSCOM_DEBUG
                if (!messagewalk) {
                    s->println("READING");
                }
            #endif
            messagewalk = true;
            if (s->available() > 2) {
                uint16_t len = getTwoBytesSerial();
                #ifdef DSCOM_DEBUG_1
                    s->print("Length: ");
                    s->println(len);
                #endif
                readData(len);
                #ifdef DSCOM_DEBUG_1
                    s->println("Data read");
                #endif
                // Update len for future use (writing)
                //data_len = len;
                uint16_t packetCrc = getTwoBytesSerial();
                uint16_t calculatedCrc = crc.XModemCrc(new_data, 0, len);
                #ifdef DSCOM_DEBUG_1
                    s->print("Calculated CRC: ");
                    s->println(calculatedCrc, HEX);
                    s->print("Received CRC: ");
                    s->println(packetCrc, HEX);
                #endif
                messagewalk = false;
                if (calculatedCrc != packetCrc)
                {
                    #ifdef DSCOM_DEBUG_1
                        s->println("CRC doesn't match");
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
            #ifdef DSCOM_DEBUG
                if (!messagewalk) s->println("APPLY");
            #endif
            messagewalk = false;
            uint8_t* old_data;
            old_data = data;
            data = new_data;
            data_len = new_data_len;
            free(old_data);
            #ifdef DSCOM_DEBUG_2
                s->println("Done applying");
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

void DSCom::write(uint8_t* out_data, uint16_t len) {
    uint8_t len_high, len_low, crc_high, crc_low;
    uint16_t crc = crc.XModemCrc(out_data, 0, len)
    DSCom::splitTwoBytes(len, len_high, len_low);
    DSCom::splitTwoBytes(crc, crc_high, crc_low);
    s->write(magic, DSCOM_MAGIC_LENGTH);
    s->write(len_high);
    s->write(len_low);
    s->write(out_data, len);
    s->write(crc_high);
    s->write(crc_low);
}

uint16_t DSCom::getTwoBytesSerial() {
    // Wait for serial bytes
    while (s->available() < 2) {}
    uint16_t high = s->read() << 8;
    uint16_t low = s->read();
    uint16_t combined = high | low;
    return combined;
}

void DSCom::splitTwoBytes(uint16_t in, uint8_t &out_high, uint8_t &out_low) {
    out_high = in >> 8;
    out_low = in & 0xff;
}

void DSCom::readData(uint16_t len) {
    new_data_len = len;
    new_data = (uint8_t*)malloc(sizeof(uint8_t)*len);
    while ((uint16_t)s->available() < len) {}
    
    // Read in data from serial
    s->readBytes(new_data, len);
}

uint8_t* DSCom::getData() {
    updated = false;
    return data;
}