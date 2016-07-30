#include <arduino.h>
#include <SPI.h>
#include "se8r01.h"

byte pipes[][6] = { "0pipe", "1pipe", "2pipe", "3pipe", "4pipe", "5pipe" };

void initPipes(byte PW);

void writeCommand(byte command) {
	digitalWrite(CSNq, 0);
	SPI.transfer(command);
	digitalWrite(CSNq, 1);
}

void writeToReg(byte reg, byte val) {
	digitalWrite(CSNq, 0);
	SPI.transfer(CMD_WRITE_REG | reg);
	SPI.transfer(val);
	digitalWrite(CSNq, 1);
}

void writeToReg(byte reg, byte *addr, byte dataSize) {
	digitalWrite(CSNq, 0);
	SPI.transfer(CMD_WRITE_REG | reg);
	for (byte i = 0; i < dataSize; i++) {
		SPI.transfer(*addr++);
	}
	digitalWrite(CSNq, 1);
}

void writeToRegMask(byte reg, byte val, byte mask) {
	byte regVal = readReg(reg);
	regVal &= ~mask;
	regVal |= val;
	writeToReg(reg, regVal);
}

byte readReg(byte reg) {
	digitalWrite(CSNq, 0);
	SPI.transfer(CMD_READ_REG & reg);
	byte val = SPI.transfer(0);
	digitalWrite(CSNq, 1);
	return val;
}

void readReg(byte reg, byte *addr, byte dataSize) {
	digitalWrite(CSNq, 0);
	SPI.transfer(CMD_READ_REG & reg);
	SPI.transfer(addr, dataSize);
	digitalWrite(CSNq, 1);
}

byte getStatusReg() {
	digitalWrite(CSNq, 0);
	byte val = SPI.transfer(CMD_NOP);
	digitalWrite(CSNq, 1);
	return val;
}

void changeMode(byte MODE) {
	digitalWrite(CEq, 0);
	if (MODE != STANDBY) {
		writeToRegMask(REG_CONFIG, MODE, 1);
		if (MODE == RX_MODE) {
			initPipes(readReg(REG_RX_PW_P0));
		}
		digitalWrite(CEq, 1);
		delayMicroseconds(210);
	}
}

void getRxPayload(byte *address, byte width) {
	digitalWrite(CSNq, 0);
	SPI.transfer(CMD_RD_RX_PLOAD);
	SPI.transfer(address, width);
	digitalWrite(CSNq, 1);
}

void pushTxPayload(byte *addr, byte dataSize) {
	digitalWrite(CSNq, 0);
	SPI.transfer(CMD_WR_TX_PLOAD);
	for (byte i = 0; i < dataSize; i++) {
		SPI.transfer(addr[i]);
	}
	digitalWrite(CSNq, 1);
}

void selectTxPipe(byte pipe) {
	digitalWrite(CEq, 0);
	writeToReg(REG_TX_ADDR, pipes[pipe], 4);
	//changing address for 0pipe, to enable ACK
	writeToReg(REG_RX_ADDR_P0 + pipe, readReg(REG_RX_ADDR_P0));	//pipe exchange to prevent duplicate
	writeToReg(REG_RX_ADDR_P0, pipes[pipe], 4);
	digitalWrite(CEq, 1);
	delayMicroseconds(210);
}

void setPower(byte power) {
	digitalWrite(CEq, 0);							//go to standby mode
	writeToRegMask(REG_RF_SETUP, power, 0x47);
	digitalWrite(CEq, 1);							//come back to working mode
	delayMicroseconds(210);							//waiting for change mode
}

//works 2Mbps mode only
void setRfSpeed(byte rFspeed) {
	digitalWrite(CEq, 0);							//go to standby mode
	writeToRegMask(REG_RF_SETUP, rFspeed, 0x28);
	digitalWrite(CEq, 1);							//come back to working mode
	delayMicroseconds(210);							//waiting for change mode
}

void setRtr(byte rtr) {
	digitalWrite(CEq, 0);							//go to standby mode
	writeToRegMask(REG_SETUP_RETR, rtr, 0x0F);
	digitalWrite(CEq, 1);							//come back to working mode
	delayMicroseconds(210);							//waiting for change mode
}

void setChannel(byte ch) {
	digitalWrite(CEq, 0);							//go to standby mode
	writeToReg(REG_RF_CH, ch);
	digitalWrite(CEq, 1);							//come back to working mode
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

void rf_switch_bank(byte bank);
void bank1Init(void);

boolean init_rf(byte payloadWidth) {
	boolean result = false;
	delay(150);							//SE8R01 power on reset delay
	SPI.begin();
	if (checkChip()) {
		pinMode(IRQq, INPUT_PULLUP);
		pinMode(CEq, OUTPUT);
		digitalWrite(CEq, 0);			//stand-by 1 mode
		bank1Init();
		initPipes(payloadWidth);
		writeToRegMask(REG_STATUS, IRQ_RX |IRQ_TX | IRQ_MAX_RT, IRQ_TX | IRQ_MAX_RT); //clear interrupts
		//writeToReg(REG_CONFIG, 0x0B);	//power up, RX-mode, enable CRC, CRC - 1 byte, enable all interrupts
		writeToReg(REG_CONFIG, 0x0F);	//power up, RX-mode, enable CRC, CRC - 2 byte, enable all interrupts
		delayMicroseconds(150);			//power up delay
		result = true;
	}
	return result;
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

void rf_switch_bank(byte bank) {
	if ((getStatusReg() & 0x80) != bank) {
		digitalWrite(CSNq, 0);
		SPI.transfer(0x50);			//command is not documented in datasheet
		SPI.transfer(0x53);			//command is not documented in datasheet
		digitalWrite(CSNq, 1);
	}
}

void initPipes(byte PW) {
	writeToReg(REG_SETUP_AW, 0x02);		//works only with 4 byte address! wtf ???
	byte pipesEnabled = 0;
	for (byte i = 0; i < sizeof(pipes) / 6; i++) {
		pipesEnabled <<= 1;
		pipesEnabled++;
		if (i == 0)
			writeToReg(REG_RX_ADDR_P0, pipes[i], 4);
		else
			writeToReg(REG_RX_ADDR_P0 + i, pipes[i][0]);
		writeToReg(REG_RX_PW_P0 + i, PW);
	}
	writeToReg(REG_EN_RXADDR, pipesEnabled);
	writeToReg(REG_EN_AA, pipesEnabled);
}

