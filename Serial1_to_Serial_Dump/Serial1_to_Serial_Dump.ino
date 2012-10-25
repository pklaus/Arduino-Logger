int incomingByte = 0;   // for incoming serial data

void setup() {
        Serial.begin(9600);
        Serial1.begin(9600);
}

void loop() {
        if (Serial.available() > 0)
                Serial1.print((char)Serial.read());
        if (Serial1.available() > 0)
                Serial.print((char)Serial1.read());
}
