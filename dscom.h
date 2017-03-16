#ifndef DSCom_H
#define DSCom_H
#define LIBRARY_VERSION_DSCOM_H   "0.1"

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
#include <crc16.h>

#define DSCOM_STATE_READY     0
#define DSCOM_STATE_READING   1
#define DSCOM_STATE_APPLY     2
#define DSCOM_MAGIC_LENGTH    4

/*
 * Use the following defines to control debug info:
 * #define DSCOM_DEBUG
 * #define DSCOM_DEBUG_1
 * #define DSCOM_DEBUG_2
 */

#define DSCOM_DEBUG
#define DSCOM_DEBUG_1
#define DSCOM_DEBUG_2

class DSCom {
public:
    //inline DSCom() {};
    // There are two versions of the read() function: one for HardwareSerial and
    //  one for SoftwareSerial
    void read(HardwareSerial &s);
    //read(SoftwareSerial s);

    uint8_t* getData();
    inline uint16_t getDataLen() {
        return data_len;
    }
    inline bool isUpdated() {
        return updated;
    }

private:
    bool messagewalk = false;  // Prevents spamming Serial monitor
    const uint8_t magic[DSCOM_MAGIC_LENGTH] = {0xde, 0xad, 0xbe, 0xef};
    uint8_t magic_status = 0;
    uint8_t state = DSCOM_STATE_READY;

    uint8_t* data;
    uint8_t* new_data;
    bool updated = false;

    uint16_t data_len = 0;
    uint16_t new_data_len = 0;

    uint16_t getTwoBytesSerial(HardwareSerial &s);
    void readData(HardwareSerial &s, uint16_t len);

    crc16 crc;
};
#endif