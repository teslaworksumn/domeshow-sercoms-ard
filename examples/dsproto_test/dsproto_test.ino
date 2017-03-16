#include <dscom.h>

#define DEBUG
#define SERIAL_BUFFER_SIZE 512

DSCom dsc;

void setup() {
    Serial.begin(115200);
}

void loop() {
    dsc.read(Serial);
    /*if (dsc.isUpdated()) {
        Serial.write(dsc.getData(),dsc.getDataLen());
        Serial.println();
    }*/
}
