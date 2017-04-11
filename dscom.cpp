#include "dscom.h"

void DSCom::read() {
    switch (state) {
        case DSCOM_STATE_READY:
            #ifdef DSCOM_DEBUG
                if (!messagewalk) Serial.println("READY");
            #endif
            messagewalk = true;
            if (s->available() > 1) {
                #ifdef DSCOM_DEBUG_2
                    Serial.print("Peek: ");
                    Serial.print(Serial.peek(), HEX);
                    Serial.print(" (");
                    Serial.print(magic[magic_status],HEX);
                    Serial.println(")");
                #endif
                if (s->read() == magic[magic_status]) {
                    magic_status++;
                } else {
                    magic_status=0;
                }
                #ifdef DSCOM_DEBUG_1
                    Serial.print("Magic status: ");
                    Serial.println(magic_status);
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
                    Serial.println("READING");
                }
            #endif
            messagewalk = true;
            if (s->available() > 2) {
                uint16_t len = getTwoBytesSerial();
                #ifdef DSCOM_DEBUG_1
                    Serial.print("Length: ");
                    Serial.println(len);
                #endif
                uint16_t rd = 0;
                if (len == 0) {
                    new_data_len = 0;
                    new_data = NULL;
                } else if (len > DSCOM_MAX_LENGTH) {
                    #ifdef DSCOM_DEBUG_1
                        Serial.println("Data NOT read (exceeds max len of DSCOM_MAX_LENGTH)");
                    #endif
                    state = DSCOM_STATE_READY;
                } else {
                    rd = readData(len);
                }
                if (!rd) {
                    #ifdef DSCOM_DEBUG_1
                        Serial.println("Data NOT read (bad packet or out of memory)");
                    #endif
                    state = DSCOM_STATE_READY;
                } else {
                    #ifdef DSCOM_DEBUG_1
                        Serial.println("Data read");
                    #endif
                    uint16_t packetCrc = getTwoBytesSerial();
                    uint16_t calculatedCrc = crc.XModemCrc(new_data, 0, len);
                    #ifdef DSCOM_DEBUG_1
                        Serial.print("Calculated CRC: ");
                        Serial.println(calculatedCrc, HEX);
                        Serial.print("Received CRC: ");
                        Serial.println(packetCrc, HEX);
                    #endif
                    messagewalk = false;
                    if (calculatedCrc != packetCrc)
                    {
                        #ifdef DSCOM_DEBUG_1
                            Serial.println("CRC doesn't match");
                        #endif
                        free(new_data);
                        state = DSCOM_STATE_READY;
                    }
                    else
                    {
                        state = DSCOM_STATE_APPLY;
                    }
                }
            }
            break;
        case DSCOM_STATE_APPLY:
            #ifdef DSCOM_DEBUG
                if (!messagewalk) Serial.println("APPLY");
            #endif
            messagewalk = false;
            free(data);
            data = new_data;
            data_len = new_data_len;
            #ifdef DSCOM_DEBUG_2
                Serial.println("Done applying");
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
    uint16_t data_crc = crc.XModemCrc(out_data, 0, len);
    splitTwoBytes(len, len_high, len_low);
    splitTwoBytes(data_crc, crc_high, crc_low);
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

uint16_t DSCom::readData(uint16_t len) {
    new_data_len = len;
    new_data = (uint8_t*)malloc(sizeof(uint8_t)*len);
    if (new_data) {
        while ((uint16_t)s->available() < len) {}
        // Read in data from serial
        return s->readBytes(new_data, len);
    } else {
        return 0;
    }
}

uint8_t* DSCom::getData() {
    updated = false;
    return data;
}