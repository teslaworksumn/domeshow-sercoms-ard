void read(HardwareSerial s) {
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
                if (!messagewalk) s.println("READING");
            #endif
            messagewalk = true;
            if (s.available() > 2) {
                uint16_t len = getTwoBytesSerial();
                #ifdef DEBUG
                    s.print("Length: ");
                    s.println(len);
                #endif
                readData(len);
                #ifdef DEBUG
                    s.println("Data read");
                #endif
                // Update len for future use (writing)
                data_len = len;
                uint16_t packetCrc = getTwoBytesSerial();
                uint16_t calculatedCrc = crc.XModemCrc(data, 0, data_len);
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
            uint8_t* old_data = data;
            data = new_data;
            data_len = new_data_len;
            free(old_data)
            #ifdef DEBUG
                s.println("Done applying");
            #endif
            state = DSCOM_STATE_READY;
            break;
        default:
            state = DSCOM_STATE_READY;
            messagewalk = false;
            break;
    }
}

uint16_t getTwoBytesSerial(HardwareSerial s) {
    // Wait for serial bytes
    while (s.available() < 2) {}
    uint16_t high = s.read() << 8;
    uint16_t low = s.read();
    uint16_t combined = high | low;
    return combined;
}

void readData(uint16_t len) {
    // Resize data array if smaller than len
    // Should never happen
    new_data_len = len
    
    while (s.available() < len) {}
    
    // Read in data from serial
    s.readBytes(new_data, len);
    
}

void cts(HardwareSerial s) {
    for (int i=DSCOM_MAGIC_LENGTH-1; i>=0; i--) {
        s.write(magic[i]);
    }
}