#define D7 2
#define D6 3
#define D5 4
#define D4 5
#define D3 6
#define D2 7
#define D1 8
#define D0 9
#define CLRB 10  //Clear address counter (bar)
#define WEB 11   //Inc address counter, Write enable (bar)
#define RBW 12  //EEPROM mode: Read (bar), Write 

#define SIZE 32768	//28C256 32K EEPROM
#define BAUD 57600	//This is as fast as I could go in testing

unsigned cnt;
unsigned char page[64];

void setData(unsigned char byte) {
    digitalWrite(D0, (byte & 0b00000001) ? HIGH : LOW);
    digitalWrite(D1, (byte & 0b00000010) ? HIGH : LOW);
    digitalWrite(D2, (byte & 0b00000100) ? HIGH : LOW);
    digitalWrite(D3, (byte & 0b00001000) ? HIGH : LOW);
    digitalWrite(D4, (byte & 0b00010000) ? HIGH : LOW);
    digitalWrite(D5, (byte & 0b00100000) ? HIGH : LOW);
    digitalWrite(D6, (byte & 0b01000000) ? HIGH : LOW);
    digitalWrite(D7, (byte & 0b10000000) ? HIGH : LOW);
}

void getData(unsigned char* byte) {
    *byte |= digitalRead(D0) << 0;
    *byte |= digitalRead(D1) << 1;
    *byte |= digitalRead(D2) << 2;
    *byte |= digitalRead(D3) << 3;
    *byte |= digitalRead(D4) << 4;
    *byte |= digitalRead(D5) << 5;
    *byte |= digitalRead(D6) << 6;
    *byte |= digitalRead(D7) << 7;
}

void setup() {
    //Do this first to avoid accidentally overwriting the EEPROM's first byte
    pinMode(RBW, OUTPUT);
    digitalWrite(RBW, LOW);    
    
    pinMode(WEB, OUTPUT);
    pinMode(CLRB, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(LED_BUILTIN, LOW);

    //Set data pins to input
    for (int i = D7; i <= D0; i++)
        pinMode(i, INPUT);

    //WEB defaults to high, goes low when writing and goes high again to inc the counter
    digitalWrite(WEB, LOW);

    //Clear the address counter
    digitalWrite(CLRB, LOW);
    digitalWrite(CLRB, HIGH);

    //Establish serial connection
    Serial.begin(BAUD);
    while (!Serial);

    //Read the EEPROM
    for (unsigned i = 0; i < SIZE; i++) {
        unsigned char byte;
        getData(&byte);
        Serial.write(byte);
        digitalWrite(WEB, HIGH);
        digitalWrite(WEB, LOW);
    }

    //Set data pins to output
    for (int i = D7; i <= D0; i++)
        pinMode(i, OUTPUT);    

    //Set EEPROM to Write mode
    digitalWrite(WEB, HIGH);
    digitalWrite(RBW, HIGH);

    //Clear address counter
    digitalWrite(CLRB, LOW);
    digitalWrite(CLRB, HIGH);

    //Light the LED to indicate ready for data input   
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {    
    //Spin loop when finished writing
	if (cnt == SIZE) {
        digitalWrite(LED_BUILTIN, LOW);
        while (true);
    }

	//The data will be written in 64-byte pages
	//The EEPROM supports writing quicker this way,
	//	and the Arduino Uno has a 64-byte serial buffer

	//Read 64 bytes
    for (unsigned i = 0; i < 64; i++, cnt++) {
        while (!Serial.available());
        page[i] = Serial.read();
    }

	//Write 64 bytes
    for (unsigned i = 0; i < 64; i++) {
        setData(page[i]);
        digitalWrite(WEB, LOW);
        digitalWrite(WEB, HIGH);
    }
}
