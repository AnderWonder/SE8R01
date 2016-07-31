#ifndef se8r01_h
#define se8r01_h

#include "se8r01_map.h"

/*
//PINS assignment
// CE:Chip Enable Activates RX or TX mode
#define CEq       7
// CSN:SPI Chip Select
#define CSNq      10
// IRQ:Maskable interrupt pin
#define IRQq      8
*/

extern uint8_t CE_pin,CS_pin,IRQ_pin;

//OPERATION MODES
enum{TX_MODE=0,RX_MODE=1,STANDBY=2}; 

//Write
void writeCommand(byte command);
void writeToReg(byte reg, byte val);
void writeToReg(byte reg,byte *addr,byte dataSize);
void writeToRegMask(byte reg,byte val,byte mask);

//Read
byte readReg(byte reg);
void readReg(byte reg,byte *addr,byte dataSize);

boolean init_rf(byte CS_pin, byte CE_pin, byte IRQ_pin,byte payloadWidth);
byte getStatusReg();
void changeMode(byte MODE);
void setRtr(byte rtr);
void getRxPayload(byte *addr, byte width);
void pushTxPayload(byte *addr, byte dataSize);
void selectTxPipe(byte pipe);
void setPower(byte power);
void setChannel(byte ch);

#endif
