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

extern uint8_t CE_pin, CS_pin, IRQ_pin;
extern byte payload_width;

//OPERATION MODES
enum {
	TX_MODE = 0, RX_MODE = 1, STANDBY = 2
};

//********************************** RADIO functions

boolean init_rf(byte CS_pin, byte CE_pin, byte IRQ_pin, byte payloadWidth);

void changeMode(byte MODE);

//returns number of retransmits if succeed, or -1 if failed to get ACK
char sendWithAck(byte *address);

void pushTxPayload(byte *addr, byte dataSize);

//returns pipe number of received data, or 7 if no data
byte getRxData(byte *address);

void getRxPayload(byte *addr, byte width);

void setChannel(byte ch);

void setPower(byte power);

byte getStatusReg();

//sets the number of retransmits for transmitted package to get ACK (max 15)
void setRtr(byte rtr);

void selectTxPipe(byte pipe);

//************************************Low level register access
//Write
void writeCommand(byte command);
void writeToReg(byte reg, byte val);
void writeToReg(byte reg, byte *addr, byte dataSize);
void writeToRegMask(byte reg, byte val, byte mask);

//Read
byte readReg(byte reg);
void readReg(byte reg, byte *addr, byte dataSize);

#endif
