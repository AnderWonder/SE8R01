#ifndef se8r01_map
#define se8r01_map

//SE8R01 SPI Commands

#define CMD_READ_REG        		0x1F  //		000x xxxx Define read command to register
#define CMD_WRITE_REG       		0x20  //		001x xxxx Define write command to register
#define CMD_RD_RX_PLOAD     		0x61  //		0110 0001 Define RX payload register address
#define CMD_WR_TX_PLOAD     		0xA0  //		1010 0000 Define TX payload register address
#define CMD_FLUSH_TX        		0xE1  //		1110 0001 Define flush TX register command
#define CMD_FLUSH_RX        		0xE2  //		1110 0010 Define flush RX register command
#define CMD_REUSE_TX_PL     		0xE3  //		1110 0011 Define reuse TX payload register command
#define CMD_W_TX_PAYLOAD_NOACK	0xB0  //		1011 0000
#define CMD_W_ACK_PAYLOAD   		0xA8  //		1010 1xxx
#define CMD_ACTIVATE        		0x50  //		0101 0000
#define CMD_R_RX_PL_WID     		0x60  //		0110 0000
#define CMD_NOP             		0xFF  //		1111 1111 Define No Operation, might be used to read status register

//SE8R01 BANK 0 registers

#define REG_CONFIG        			0x00  //		'Config' register address                          
#define REG_EN_AA         			0x01  //		'Enable Auto Acknowledgment' register address      
#define REG_EN_RXADDR     			0x02  //		'Enabled RX addresses' register address            
#define REG_SETUP_AW      			0x03  //		'Setup address width' register address             
#define REG_SETUP_RETR    			0x04  //		'Setup Auto. Retrans' register address             
#define REG_RF_CH         			0x05  //		'RF channel' register address                      
#define REG_RF_SETUP      			0x06  //		'RF setup' register address                        
#define REG_STATUS        			0x07  //		'Status' register address                          
#define REG_OBSERVE_TX    			0x08  //		'Observe TX' register address                      
#define REG_RPD           			0x09  //		'Received Power Detector' register address         
#define REG_RX_ADDR_P0    			0x0A  //		'RX address pipe0' register address                
#define REG_RX_ADDR_P1    			0x0B  //		'RX address pipe1' register address                
#define REG_RX_ADDR_P2    			0x0C  //		'RX address pipe2' register address                
#define REG_RX_ADDR_P3    			0x0D  //		'RX address pipe3' register address                
#define REG_RX_ADDR_P4    			0x0E  //		'RX address pipe4' register address                
#define REG_RX_ADDR_P5    			0x0F  //		'RX address pipe5' register address                
#define REG_TX_ADDR       			0x10  //		'TX address' register address                      
#define REG_RX_PW_P0      			0x11  //		'RX payload width, pipe0' register address         
#define REG_RX_PW_P1      			0x12  //		'RX payload width, pipe1' register address         
#define REG_RX_PW_P2      			0x13  //		'RX payload width, pipe2' register address         
#define REG_RX_PW_P3      			0x14  //		'RX payload width, pipe3' register address         
#define REG_RX_PW_P4      			0x15  //		'RX payload width, pipe4' register address         
#define REG_RX_PW_P5      			0x16  //		'RX payload width, pipe5' register address         
#define REG_FIFO_STATUS   			0x17  //		'FIFO Status Register' register address            
#define REG_DYNPD         			0x1C  //		'Enable dynamic payload length' register address   
#define REG_FEATURE       			0x1D  //		'Feature' register address                         
#define REG_SETUP_VALUE   			0x1E                                                           
#define REG_PRE_GURD      			0x1F

#define REG_LINE          			0x00  
#define REG_PLL_CTL0      			0x01  
#define REG_PLL_CTL1      			0x02  
#define REG_CAL_CTL       			0x03  
#define REG_A_CNT_REG     			0x04  
#define REG_B_CNT_REG     			0x05  
#define REG_RESERVED0     			0x06  
#define REG_STATUS        			0x07
#define REG_STATE         			0x08  
#define REG_CHAN          			0x09  
#define REG_IF_FREQ       			0x0A  
#define REG_AFC_COR       			0x0B  
#define REG_FDEV          			0x0C  
#define REG_DAC_RANGE     			0x0D  
#define REG_DAC_IN        			0x0E  
#define REG_CTUNING       			0x0F  
#define REG_FTUNING       			0x10  
#define REG_RX_CTRL       			0x11  
#define REG_FAGC_CTRL     			0x12  
#define REG_FAGC_CTRL_1   			0x13  
#define REG_DAC_CAL_LOW   			0x17  
#define REG_DAC_CAL_HI    			0x18  
#define REG_RESERVED1     			0x19  
#define REG_DOC_DACI      			0x1A  
#define REG_DOC_DACQ      			0x1B  
#define REG_AGC_CTRL      			0x1C  
#define REG_AGC_GAIN      			0x1D  
#define REG_RF_IVGEN      			0x1E  
#define REG_TEST_PKDET    			0x1F  

//BITS
#define POWER_BIT         			0x02 

//Interrupts

#define IRQ_RX	  	         		0x40 
#define IRQ_TX  	  	       		0x20 
#define IRQ_MAX_RT          			0x10 

//FIFO status

#define FIFO_STATUS_TX_REUSE   		0x40
#define FIFO_STATUS_TX_FULL    		0x20
#define FIFO_STATUS_TX_EMPTY   		0x10
#define FIFO_STATUS_RX_FULL    		0x02
#define FIFO_STATUS_RX_EMPTY   		0x01

//SPEED

#define SPEED_500Kbps 				0x28
#define SPEED_1Mbps   				0x00
#define SPEED_2Mbps   				0x08

//POWER               					     

#define POWER_5dbm    				0x47
#define POWER_0dbm    				0x40
#define POWER_m6dbm   				0x04
#define POWER_m12dbm  				0x02
#define POWER_m18dbm  				0x01

//REG BANKS

#define BANK0                  		0x00
#define BANK1                  		0x80

#endif

