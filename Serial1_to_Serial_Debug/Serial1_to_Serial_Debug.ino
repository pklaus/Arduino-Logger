int incomingByte = 0;   // for incoming serial data

void setup() {
        Serial.begin(9600);
        Serial1.begin(9600);
}

void loop() {
        if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();
                // say what you got:
                Serial1.print("I received: ");
                Serial1.println(incomingByte, DEC);
        }
        if (Serial1.available() > 0) {
                incomingByte = Serial1.read();
                Serial.print("I received: ");
                Serial.println(incomingByte, DEC);
        }
}
