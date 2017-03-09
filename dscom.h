#ifndef DSCom_H
#define DSCom_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define DSCOM_STATE_READY     0
#define DSCOM_STATE_READING   1
#define DSCOM_STATE_APPLY     2
#define DSCOM_MAGIC_LENGTH    4

class DSCom {
public:
    DSCom();
    // There are two versions of the read() function: one for HardwareSerial and
    //  one for SoftwareSerial
    read(HardwareSerial s);
    read(SoftwareSerial s);

private:
    bool messagewalk = false;  // Prevents spamming Serial monitor
    const uint8_t magic[] = {0xde, 0xad, 0xbe, 0xef};
    uint8_t magic_status = 0;
    uint8_t state = DSCOM_STATE_READY;

    uint8_t* data;
    uint8_t* new_data;

    uint16_t data_len = 0;
    uint16_t new_data_len = 0;

#endif