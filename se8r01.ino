#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include "se8r01.h"

#define RPD_MODE 	3
#define RPD_MODE2 4
#define RPD_MODE3 5
#define TEST_MODE 6

LiquidCrystal_I2C lcd(0x27, 20, 4);
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();

int counter = 0;
byte s = 0, w = 45;
int tryes = 100;

byte mode;

byte txData[] = { "123456" }; //min 4 byte, wtf?? up to 32 bytes
const int PAYLOAD_WIDTH = sizeof(txData);
byte rxData[PAYLOAD_WIDTH]; //size of rx must be the same as tx if not DNPL

void setup() {
	pinMode(2, INPUT_PULLUP);
	pinMode(3, INPUT_PULLUP);
	pinMode(4, INPUT_PULLUP);

	debouncer1.attach(2);
	debouncer1.interval(5); // interval in ms

	debouncer2.attach(3);
	debouncer2.interval(5); // interval in ms

	debouncer3.attach(4);
	debouncer3.interval(5); // interval in ms

	Serial.begin(9600);
	lcd.init();
	lcd.backlight();

	mode = TX_MODE;
	mode = RX_MODE;

	if (!init_rf(10,7,8,PAYLOAD_WIDTH)) {
		Serial.println("Chip not found!");
		while (1);
	}

	setPower(POWER_5dbm);

	selectTxPipe(0);

	setRtr(15);

	changeMode(mode);

	Serial.setTimeout(100);
	Serial.println("********************** Radio starting *******************");
	Serial.print("************************ ");
	printMode();
	Serial.println(" ************************");

	//Serial.println(readReg(REG_CONFIG),HEX);

	//readReg(REG_RX_ADDR_P0,rxData,4);
	//Serial.println((char*)rxData);
	//printRXdata(rxData,4);
}

void scanNoise() {
	lcd.clear();
	lcd.print("SCANNING...");
	Serial.println("SCANNING...");
	changeMode(RX_MODE);
	char curNoise, minNoise = 100, maxNoise = -100;
	byte chnlMinNoise, chnlMaxNoise;
	for (byte i = 0; i < 125; i++) {
		setChannel(i);
		curNoise = getRpd();
		minNoise = min(curNoise, minNoise);
		if (minNoise == curNoise) {
			chnlMinNoise = i;
		}
		maxNoise = max(curNoise, maxNoise);
		if (maxNoise == curNoise) {
			chnlMaxNoise = i;
		}
	}
	lcd.clear();
	lcd.print("MIN:");
	lcd.print(minNoise, DEC);
	lcd.print(" CH:");
	lcd.print(chnlMinNoise);
	lcd.setCursor(0, 1);
	lcd.print("MAX:");
	lcd.print(maxNoise, DEC);
	lcd.print(" CH:");
	lcd.print(chnlMaxNoise);
	Serial.print("MIN NOISE:");
	Serial.print(minNoise, DEC);
	Serial.print(" CHNL:");
	Serial.println(chnlMinNoise);
	Serial.print("MAX NOISE:");
	Serial.print(maxNoise, DEC);
	Serial.print(" CHNL:");
	Serial.println(chnlMaxNoise);
}

bool sendCommandChangeChannel(byte newCh) {
	bool result = true;
	lcd.clear();
	lcd.print("CHANGING CH...");
	changeMode(TX_MODE);
	selectTxPipe(1);
	String(newCh).getBytes(txData, PAYLOAD_WIDTH);
	char send = sendWithAck(txData);
	delay(1);
	selectTxPipe(0);
	if (send < 0) {
		result = false;
		byte i;
		for (i = 0; i < 10; i++)
			if (sendWithAck(txData) >= 0)
				break;
		if (i == 10) { //check if we are not still on prev channel
			byte prevCh = readReg(REG_RF_CH);
			setChannel(newCh);
			for (i = 0; i < 10; i++)
				if (sendWithAck(txData) >= 0)
					break;
			if (i < 10) //check if got response on new channel
				result = true;
			setChannel(prevCh);
		}
	}
	changeMode(mode);
	return result;
}

void loop() {

	if (Serial.available()) {
		String command = Serial.readStringUntil(' ');
		int param = Serial.parseInt();
		Serial.print("********************** ");
		if (command.equalsIgnoreCase("ch")) {
			setChannel(param);
			Serial.print("Set channel:" + String(param));
			lcd.setCursor(0, 1);
			lcd.print("CHANNEL:");
			lcd.print(readReg(REG_RF_CH));
			lcd.print("   ");
		}
		if (command.equalsIgnoreCase("pi")) {
			selectTxPipe(param);
			Serial.print("Set pipe:" + String(param));
		}
		if (command.equalsIgnoreCase("rx")) {
			mode = RX_MODE;
			changeMode(mode);
			printMode();
		}
		if (command.equalsIgnoreCase("tx")) {
			mode = TX_MODE;
			changeMode(mode);
			printMode();
		}
		if (command.equalsIgnoreCase("st")) {
			mode = STANDBY;
			changeMode(mode);
			printMode();
		}
		if (command.equalsIgnoreCase("rpd")) {
			mode = RPD_MODE;
			changeMode(RX_MODE);
			printMode();
		}
		if (command.equalsIgnoreCase("rpd2")) {
			mode = RPD_MODE2;
			s = param * w;
			changeMode(RX_MODE);
			printMode();
		}
		if (command.equalsIgnoreCase("rpd3")) {
			mode = RPD_MODE3;
			s = param;
			changeMode(RX_MODE);
			printMode();
		}
		if (command.equalsIgnoreCase("pw")) {
			switch (param) {
				case 5:
					setPower(POWER_5dbm);
					break;
				case 0:
					setPower(POWER_0dbm);
					break;
				case 6:
					setPower(POWER_m6dbm);
					break;
				case 12:
					setPower(POWER_m12dbm);
					break;
				case 18:
					setPower(POWER_m18dbm);
					break;
			}
			if (param != 5 && param != 0)
				param = -param;
			Serial.print("Set power:" + String(param));
			Serial.print(" dbm");
		}
		if (command.equalsIgnoreCase("sc")) {
			scanNoise();
		}
		if (command.equalsIgnoreCase("ts")) {
			mode = TEST_MODE;
			tryes = 100;
			if (param != 0)
				tryes = param;
		}
		Serial.println(" ************************");
		if (command.equalsIgnoreCase("rpd2"))
			printHeadRPD2();
	}

	for (byte i = 0; i < 10; i++) {
		debouncer1.update();
		debouncer2.update();
		debouncer3.update();
		delay(10);
	}

	bool but1 = !debouncer1.read();
	bool but2 = !debouncer2.read();
	bool but3 = !debouncer3.read();

	if (but1) {
		if (but2) {
			byte prevCh = readReg(REG_RF_CH);
			scanNoise();
			delay(3000);
			setChannel(prevCh);
			but2 = false;
		} else if (but3) {
			but3=false;
			lcd.setCursor(0,0);
			lcd.print("               ");
			lcd.setCursor(0,0);
			lcd.print("RPD:");
			lcd.print(getRpd(),DEC);
			delay(1000);
		}
		else {
			mode++;
			if (mode == RPD_MODE || mode == RPD_MODE2 || mode == RPD_MODE3) {
				mode = TEST_MODE;
			}
			if (mode > TEST_MODE)
				mode = 0;
			if (mode != TEST_MODE)
				printMode();
			delay(500);
		}
	}

	if (but2) {
		byte newCh = readReg(REG_RF_CH) + 10;
		if (newCh > 124)
			newCh = 0;
		if (sendCommandChangeChannel(newCh))
			setChannel(newCh);
		printMode();
	}

	if (but3) {
		byte newCh = readReg(REG_RF_CH) + 1;
		if (newCh > 124)
			newCh = 0;
		if (sendCommandChangeChannel(newCh))
			setChannel(newCh);
		printMode();
	}

	if (mode == RX_MODE) {
		byte pipe = 7;
		pipe = getRxData(rxData);
		if (pipe < 7) {
			while ((getStatusReg() & 0xE) != 0xE)
				getRxPayload(rxData, PAYLOAD_WIDTH);
			String strVal((char*) rxData);
			Serial.print("Received on pipe:");
			Serial.println(pipe);
			Serial.println(strVal);
			if (pipe == 1) {
				byte newCh = (byte) strVal.toInt();
				Serial.print("Change channel to:");
				Serial.println(newCh);
				setChannel(newCh);
			}
		}
	}

	if (mode == TX_MODE) {
		String(counter).getBytes(txData, PAYLOAD_WIDTH);
		char rtr = sendWithAck(txData);
		if (rtr >= 0)
			Serial.println("Success! Data packet delivered!");
		else {
			Serial.println(
					"************************Fail! Data packet may be lost!********************************");
		}

		Serial.print("Rtr: ");
		Serial.print(rtr, DEC);
		Serial.print(" - ");
		Serial.println((readReg(REG_OBSERVE_TX) & 0xF0) >> 4);

		counter++;
		delay(500);
	}

	if (mode == RPD_MODE) {
		//Serial.println(int(getRpd()));
		//delay(500);
		plot(getRpd() * 10);
		delay(100);
	}

	if (mode == RPD_MODE2) {
		for (byte i = s; i < min(s + w, 125); i++) {
			setChannel(i);
			delay(1);
			char rpd = getRpd();
			Serial.print(" ");
			if (rpd >= 0)
				Serial.print(" ");
			else
				Serial.print("-");
			if (abs(rpd) < 10)
				Serial.print("0");
			Serial.print(abs(rpd));
			if (abs(rpd) < 100)
				Serial.print(" ");
			Serial.print(" ");
		}
		Serial.println();
		//delay(500);
	}

	if (mode == RPD_MODE3) {
		int rpds[4];
		for (byte i = s; i < s + 4; i++) {
			setChannel(i);
			delay(1);
			rpds[i - s] = getRpd() * 10;
		}
		plot(rpds[0], rpds[1], rpds[2], rpds[3]);
		delay(100);
	}

	if (mode == TEST_MODE) {
		testLink();
		delay(1000);
	}

}

void printMode() {
	lcd.clear();
	String modeName;
	if (mode == STANDBY) {
		modeName = "STANDBY";
	}
	if (mode == RX_MODE) {
		modeName = "RX";
	}
	if (mode == TX_MODE) {
		modeName = "TX";
	}
	if (mode == RPD_MODE) {
		modeName = "RPD";
	}
	if (mode == RPD_MODE2) {
		modeName = "RPD 2";
	}
	if (mode == RPD_MODE3) {
		modeName = "RPD 3";
	}
	Serial.print(modeName + " MODE");
	lcd.print(modeName + " MODE ");
	lcd.setCursor(0, 1);
	lcd.print("CHANNEL:");
	if (mode == RPD_MODE3) {
		lcd.print(s);
		lcd.print("-");
		lcd.print(s + 3);
	} else {
		lcd.print(readReg(REG_RF_CH));
		lcd.print("   ");
	}
}

void printHeadRPD2() {
	for (byte i = s; i < min(s + w, 124); i++) {
		Serial.print("  ");
		if (i < 10)
			Serial.print(0);
		Serial.print(i);
		Serial.print("  ");
	}
	Serial.println();
}

char getRpd() {
	byte avCount = 100;
	int rpd = 0;
	for (byte i = 0; i < avCount; i++) {
		rpd += (char) readReg(REG_RPD);
		delayMicroseconds(500);
	}
	rpd /= avCount;
	return (char) rpd;
}

void testLink() {
	int suc = 0;
	lcd.clear();
	lcd.print("TESTING...");
	Serial.println("TESTING...");
	changeMode(TX_MODE);
	for (int i = 0; i < tryes; i++) {
		if (sendWithAck(txData) >= 0)
			suc++;
		delay(10);
	}
	lcd.clear();
	lcd.print("S:");
	lcd.print(tryes);
	lcd.print(" ACK:");
	lcd.print(suc);
	lcd.setCursor(0, 1);
	lcd.print("%:");
	lcd.print(suc * 100 / tryes);

	Serial.print("Send:");
	Serial.print(tryes);
	Serial.print(" ACK:");
	Serial.println(suc);
	Serial.print("%:");
	Serial.println(suc * 100 / tryes);
}

//returns number of retransmits if succeed, or -1 if failed to get ACK
char sendWithAck(byte *address) {
	char rtr = -1;
	writeToRegMask(REG_STATUS, IRQ_TX | IRQ_MAX_RT, IRQ_TX | IRQ_MAX_RT); //clear TX interrupts
	pushTxPayload(address, PAYLOAD_WIDTH);
	digitalWrite(CE_pin, 0); //!must set 0 to prevent resend packets if fifo will not be empty !(if lose packet)
	while (digitalRead(IRQ_pin) != LOW);
	byte status = getStatusReg();
	if (status & IRQ_TX)
		rtr = readReg(REG_OBSERVE_TX) & 0x0F;
	else if (status & IRQ_MAX_RT) {
		writeCommand(CMD_FLUSH_TX); //so as fifo is not empty in this case, flush it
		rtr = -1;
	}
	digitalWrite(CE_pin, 1);
	delayMicroseconds(210);
	return rtr;
}

//returns pipe number of received data, or 7 if no data
byte getRxData(byte *address) {
	byte pipe = 7;
	if (digitalRead(IRQ_pin) == LOW) {
		byte status = getStatusReg();
		if (status & IRQ_RX) {
			getRxPayload(address, PAYLOAD_WIDTH);
			writeToRegMask(REG_STATUS, IRQ_RX, IRQ_RX);  //clear RX interrupt
			pipe = (status & 0x0E) >> 1;
		}
	}
	return pipe;
}

void printRXdata(byte* address, byte dlength) {
	for (byte i = 0; i < dlength; i++) {
		Serial.print(address[i], HEX);
		Serial.print(" : ");
	}
	Serial.println();
}

void plot(int data1) {
	plot(data1, 0, 0, 0);
}

void plot(int data1, int data2, int data3, int data4) {
	int pktSize;
	int buffer[20];

	buffer[0] = 0xCDAB;    //SimPlot packet header. Indicates start of data packet
	buffer[1] = 4 * sizeof(int); //Size of data in bytes. Does not include the header and size fields
	buffer[2] = data1;
	buffer[3] = data2;
	buffer[4] = data3;
	buffer[5] = data4;

	pktSize = 2 + 2 + (4 * sizeof(int)); //Header bytes + size field bytes + data

//IMPORTANT: Change to serial port that is connected to PC
	Serial.write((uint8_t *) buffer, pktSize);
}
