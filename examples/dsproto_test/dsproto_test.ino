#define DSCOM_DEBUG
#define DSCOM_DEBUG_1
#include <dscom.h>



DSCom dsc;

void setup() {
    Serial.begin(115200);
}

void loop() {
    dsc.read();
    if (dsc.isUpdated()) {
        for (uint16_t i=0; i<dsc.getDataLen(); i++) {
            Serial.print(dsc.getData()[i],HEX);
        }
        Serial.println();
    }
}
