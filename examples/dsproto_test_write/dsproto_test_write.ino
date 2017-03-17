#include <dscom.h>

DSCom dsc;

void setup() {
    Serial.begin(115200);
}

void loop() {
    dsc.read();
    if (dsc.isUpdated()) {
        dsc.write(dsc.getData(), dsc.getDataLen());
    }
}
