#include <arduino.h>
#include <SPI.h>
#include "se8r01.h"

uint8_t CE_pin, CS_pin, IRQ_pin;

byte payload_width;

byte pipes[][6] = { "0pipe", "1pipe", "2pipe", "3pipe", "4pipe", "5pipe" };

void initPipes() {
	writeToReg(REG_SETUP_AW, 0x02);		//works only with 4 byte address! wtf ???
	byte pipesEnabled = 0;
	for (byte i = 0; i < sizeof(pipes) / 6; i++) {
		pipesEnabled <<= 1;
		pipesEnabled++;
		if (i == 0)
			writeToReg(REG_RX_ADDR_P0, pipes[i], 4);
		else
			writeToReg(REG_RX_ADDR_P0 + i, pipes[i][0]);
		writeToReg(REG_RX_PW_P0 + i, payload_width);
	}
	writeToReg(REG_EN_RXADDR, pipesEnabled);
	writeToReg(REG_EN_AA, pipesEnabled);
}

void writeCommand(byte command) {
	digitalWrite(CS_pin, 0);
	SPI.transfer(command);
	digitalWrite(CS_pin, 1);
}

void writeToReg(byte reg, byte val) {
	digitalWrite(CS_pin, 0);
	SPI.transfer(CMD_WRITE_REG | reg);
	SPI.transfer(val);
	digitalWrite(CS_pin, 1);
}

void writeToReg(byte reg, byte *addr, byte dataSize) {
	digitalWrite(CS_pin, 0);
	SPI.transfer(CMD_WRITE_REG | reg);
	for (byte i = 0; i < dataSize; i++) {
		SPI.transfer(*addr++);
	}
	digitalWrite(CS_pin, 1);
}

void writeToRegMask(byte reg, byte val, byte mask) {
	byte regVal = readReg(reg);
	regVal &= ~mask;
	regVal |= val;
	writeToReg(reg, regVal);
}

byte readReg(byte reg) {
	digitalWrite(CS_pin, 0);
	SPI.transfer(CMD_READ_REG & reg);
	byte val = SPI.transfer(0);
	digitalWrite(CS_pin, 1);
	return val;
}

void readReg(byte reg, byte *addr, byte dataSize) {
	digitalWrite(CS_pin, 0);
	SPI.transfer(CMD_READ_REG & reg);
	SPI.transfer(addr, dataSize);
	digitalWrite(CS_pin, 1);
}

byte getStatusReg() {
	digitalWrite(CS_pin, 0);
	byte val = SPI.transfer(CMD_NOP);
	digitalWrite(CS_pin, 1);
	return val;
}

void changeMode(byte MODE) {
	digitalWrite(CE_pin, 0);
	if (MODE != STANDBY) {
		writeToRegMask(REG_CONFIG, MODE, 1);
		if (MODE == RX_MODE) {
			initPipes();
		}
		digitalWrite(CE_pin, 1);
		delayMicroseconds(210);
	}
}

void getRxPayload(byte *addr, byte width) {
	digitalWrite(CS_pin, 0);
	SPI.transfer(CMD_RD_RX_PLOAD);
	SPI.transfer(addr, width);
	digitalWrite(CS_pin, 1);
}

void pushTxPayload(byte *addr, byte dataSize) {
	digitalWrite(CS_pin, 0);
	SPI.transfer(CMD_WR_TX_PLOAD);
	for (byte i = 0; i < dataSize; i++) {
		SPI.transfer(addr[i]);
	}
	digitalWrite(CS_pin, 1);
}

void selectTxPipe(byte pipe) {
	digitalWrite(CE_pin, 0);
	writeToReg(REG_TX_ADDR, pipes[pipe], 4);
	//changing address for 0pipe, to enable ACK
	writeToReg(REG_RX_ADDR_P0 + pipe, readReg(REG_RX_ADDR_P0));	//pipe exchange to prevent duplicate
	writeToReg(REG_RX_ADDR_P0, pipes[pipe], 4);
	digitalWrite(CE_pin, 1);
	delayMicroseconds(210);
}

void setPower(byte power) {
	digitalWrite(CE_pin, 0);							//go to standby mode
	writeToRegMask(REG_RF_SETUP, power, 0x47);
	digitalWrite(CE_pin, 1);							//come back to working mode
	delayMicroseconds(210);							//waiting for change mode
}

//works 2Mbps mode only
void setRfSpeed(byte rFspeed) {
	digitalWrite(CE_pin, 0);							//go to standby mode
	writeToRegMask(REG_RF_SETUP, rFspeed, 0x28);
	digitalWrite(CE_pin, 1);							//come back to working mode
	delayMicroseconds(210);							//waiting for change mode
}

void setRtr(byte rtr) {
	digitalWrite(CE_pin, 0);							//go to standby mode
	writeToRegMask(REG_SETUP_RETR, rtr, 0x0F);
	digitalWrite(CE_pin, 1);							//come back to working mode
	delayMicroseconds(210);							//waiting for change mode
}

void setChannel(byte ch) {
	digitalWrite(CE_pin, 0);							//go to standby mode
	writeToReg(REG_RF_CH, ch);
	digitalWrite(CE_pin, 1);							//come back to working mode
	delayMicroseconds(210);							//waiting for change mode
}

boolean checkChip(void) {
	boolean res = true;
	byte _ch = readReg(REG_RF_CH);
	for (byte i = 0; i < 125; i++) {
		setChannel(i);
		if (readReg(REG_RF_CH) != i) {
			res = false;
			break;
		}
	}
	setChannel(_ch);
	return res;
}

void rf_switch_bank(byte bank) {
	if ((getStatusReg() & 0x80) != bank) {
		digitalWrite(CS_pin, 0);
		SPI.transfer(0x50);			//command is not documented in datasheet
		SPI.transfer(0x53);			//command is not documented in datasheet
		digitalWrite(CS_pin, 1);
	}
}

void bank1Init(void) {
	rf_switch_bank(BANK1);

	//repeat, but necessary
	byte p1[] = { 0x40, 0x00, 0x10, 0xE6 };
	writeToReg(REG_PLL_CTL0, p1, sizeof(p1));
	byte p2[] = { 0x20, 0x08, 0x50, 0x40, 0x50 };
	writeToReg(REG_CAL_CTL, p2, sizeof(p2));

	byte p3[] = { 0x00, 0x00, 0x1E };
	writeToReg(REG_IF_FREQ, p3, sizeof(p3));

	//repeat, but necessary
	writeToReg(REG_FDEV, 0x29);

	writeToReg(REG_DAC_CAL_LOW, 0x00);

	writeToReg(REG_DAC_CAL_HI, 0x7F);

	//repeat, but necessary
	byte p4[] = { 0x02, 0xC1, 0xEB, 0x1C };
	writeToReg(REG_AGC_GAIN, p4, sizeof(p4));

	//repeat, but necessary
	byte p5[] = { 0x97, 0x64, 0x00, 0x81 };
	writeToReg(REG_RF_IVGEN, p5, sizeof(p5));

	byte p6[] = { 0x40, 0x01, 0x30, 0xE2 };
	writeToReg(REG_PLL_CTL0, p6, sizeof(p6));
	byte p7[] = { 0x29, 0x89, 0x55, 0x40, 0x50 };
	writeToReg(REG_CAL_CTL, p7, sizeof(p7));

	writeToReg(REG_FDEV, 0x29);

	byte p8[] = { 0x55, 0xC2, 0x09, 0xAC };
	writeToReg(REG_RX_CTRL, p8, sizeof(p8));

	byte p9[] = { 0x00, 0x14, 0x08, 0x29 };
	writeToReg(REG_FAGC_CTRL_1, p9, sizeof(p9));

	byte p10[] = { 0x02, 0xC1, 0xCB, 0x1C };
	writeToReg(REG_AGC_GAIN, p10, sizeof(p10));

	byte p11[] = { 0x97, 0x64, 0x00, 0x01 };
	writeToReg(REG_RF_IVGEN, p11, sizeof(p11));

	byte p12[] = { 0x2A, 0x04, 0x00, 0x7D };
	writeToReg(REG_TEST_PKDET, p12, sizeof(p12));

	rf_switch_bank(BANK0);
}

boolean init_rf(uint8_t _CS_pin, uint8_t _CE_pin, uint8_t _IRQ_pin, byte payloadWidth) {
	boolean result = false;
	CS_pin = _CS_pin;
	CE_pin = _CE_pin;
	IRQ_pin = _IRQ_pin;
	pinMode(IRQ_pin, INPUT_PULLUP);
	pinMode(CE_pin, OUTPUT);
	payload_width = payloadWidth;
	delay(150);							//SE8R01 power on reset delay
	SPI.begin();
	if (checkChip()) {
		digitalWrite(CE_pin, 0);			//stand-by 1 mode
		bank1Init();
		initPipes();
		writeToRegMask(REG_STATUS, IRQ_RX | IRQ_TX | IRQ_MAX_RT, IRQ_TX | IRQ_MAX_RT); //clear interrupts
		writeToReg(REG_CONFIG, 0x0F);	//power up, RX-mode, enable CRC, CRC - 2 byte, enable all interrupts
		delayMicroseconds(150);			//power up delay
		result = true;
	}
	return result;
}

//returns number of retransmits if succeed, or -1 if failed to get ACK
char sendWithAck(byte *address) {
	char rtr = -1;
	writeToRegMask(REG_STATUS, IRQ_TX | IRQ_MAX_RT, IRQ_TX | IRQ_MAX_RT); //clear TX interrupts
	pushTxPayload(address, payload_width);
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
			getRxPayload(address, payload_width);
			writeToRegMask(REG_STATUS, IRQ_RX, IRQ_RX);  //clear RX interrupt
			pipe = (status & 0x0E) >> 1;
		}
	}
	return pipe;
}
