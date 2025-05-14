//--------------------------------------------------------
//  $Id: hw.h 417 2010-08-25 02:06:00Z david $
//  $Date: 2010-08-25 10:06:00 +0800 (Wed, 25 Aug 2010) $
//  $Rev: 417 $
//  Brief description: 	low level routines for processor
//  Last changed by $Author: david $
//--------------------------------------------------------
#ifndef HW_H
#define HW_H

#include <stdint.h>
#include "queue.h"

// XTAL frequency in Hz
#define XTALFREQ 					24 // Mhz 
#define	TICKPERIOD					20000
#define	HW_TICKS_PER_SECOND			50
#define	HW_TICKS_PER_HUNDREDMILLI	5

#define	KEY_WAITTIME			10000

//#define FRAMING_ERROR1 			(1<<FE0)
//#define DATA_OVERRUN1 			(1<<DOR0)
#define	TX_SERBUF_SIZE	120

typedef struct {
	// Need the following data for each output port:-
	uint8_t bufIn;                      // - buffer data in
	uint8_t bufOut;                     // - buffer data out
	uint8_t bufChars;                   // - how many
	uint8_t buffer[TX_SERBUF_SIZE];     // - output buffer
} APortData;

// enable/disable receive interrupt
#define UART2_Enable_RxInterrupt       SCI2C2_RIE = 1 //; (void) SCI2S1  //Clear Flags
#define UART2_Disable_RxInterrupt      SCI2C2_RIE = 0 

#define UART1_Enable_RxInterrupt 	    SCI1C2_RIE = 1 //; (void) SCI1S1  //Clear Flags
#define UART1_Disable_RxInterrupt      SCI1C2_RIE = 0 

#define RX_QUEUE_SIZE	100
#define UART2_RxBufferSize 64
#define UART3_RxBufferSize 64
#define UART4_RxBufferSize 64
extern CIRC Rx1_CircularBuffer;
extern CIRC ModemDiag_CircularBuffer;
extern CIRC Rx2_CircularBuffer;
#ifndef Disable_i2c_uart
extern CIRC Rx3_CircularBuffer;
extern CIRC Rx4_CircularBuffer;
#endif

// how many 20mSec times a key must be in a set state
#define	KEY_FILTER_SIZE			4

typedef struct {
	uint8_t keysPressed;
	volatile uint8_t Activity;
} KeyData;

extern KeyData CurrentKeyBoardData;

#define STOP_IN1            PTAD_PTAD4
#define STOP_IN2            PTAD_PTAD5
#define START_IN1           PTAD_PTAD6
#define AUX_DIG_IN		  PTAD_PTAD7

#define EXT_POW_ON		  !PTFD_PTFD5	

#define RELAY_CHARGE        PTAD_PTAD0		//Charge relay
#define RELAY_BATTPOWER     PTED_PTED7 		//Backup Battery Power
//#define KEY7				  PTJD_PTJD6		
//#define KEY6				  PTJD_PTJD5
//#define KEYPORT			  (PTJD & 0x1F)
#define KEYPORT			  (PTJD & 0x7F)
#define RELAY_OUT1          PTCD_PTCD5
#define RELAY_OUT2          PTCD_PTCD4 // this is actually ROUT1 on the circuit...
#define CC_OUT1			  PTGD_PTGD6
#define CC_OUT2			  PTGD_PTGD7
#define LED_HEATBEAT         PTJD_PTJD7
#ifdef CodeChange_BoardA12
#define PWM_Enable_Pin PTFD_PTFD0
#endif

/* 
 * PortA:
 * PTA7 <- IN4
 * PTA6 <- IN3
 * PTA5 <- IN2
 * PTA4 <- IN1
 * PTA3 [ADP5] AD6
 * PTA2 [ADP4] AD5
 * PTA1 -> DOUT3
 * PTA0 -> CHARGE
 * PTADD = 0x03;
 * 
 * PortB:
 * PTB7 -> ModemReset
 * PTB6 -> DOUT5
 * PTB5 Crystal
 * PTB4 Crystal
 * PTB3 Empty
 * PTB2 <- IN7
 * PTB1 Empty
 * PTB0 -> DOUT4
 * PTBDD = 0xc1;
 * 
 * PortC:
 * PTC7 -> ROUT4
 * PTC6 -> ROUT3
 * PTC5 -> ROUT2
 * PTC4 -> ROUT1
 * PTC3 [ADP6] AD7
 * PTC2 [ADP6] AD_BATT
 * PTC1 -> WR_3V3
 * PTC0 <- IRQ
 * PTCDD = 0xf2;
 * 
 * PortD:
 * PTD7 [RX1] UART_1_Rx
 * PTD6 [TX1] UART_1_Tx
 * PTD5 Empty
 * PTD4 <- PulseIn_3
 * PTD3 <- FLOW_WTR
 * PTD2 <- FLOW_CHEM
 * PTD1 [RESET] RESET
 * PTD0 [BKGD] BKGD
 * PTDDD = 0x00;
 * 
 * PortE:
 * PTE7 -> BATT_ON
 * PTE6 [RX2] UART_2_Rx
 * PTE5 [TX2] UART_2_Tx
 * PTE4 Empty
 * PTE3 -> RD_3V3
 * PTE2 -> CE_3V3
 * PTE1 -> CD_3V3
 * PTE0 -> RST_3V3
 * PTEDD = 0x8f;
 * 
 * PortF:
 * PTF7 <- IN6
 * PTF6 <- IN5
 * PTF5 <- POWER_ON_INPUT
 * PTF4 [SDA] SDA
 * PTF3 [SCL] SCL
 * PTF2 -> 
 * PTF1 -> 4-20/PWM
 * PTF0 -> PWM_Enable_Pin
 * PTFDD = 0x07;
 * 
 * PortG:
 * PTG7 -> DOUT2
 * PTG6 -> DOUT1
 * PTG5 -> V_OE
 * PTG0 -> mA_Select
 * PTGDD = 0xe1;
 * 
 * PortH:
 * PTH7 <-> DB7
 * PTH6 <-> DB6
 * PTH5 <-> DB5
 * PTH4 <-> DB4
 * PTH3 <-> DB3
 * PTH2 <-> DB2
 * PTH1 <-> DB1
 * PTH0 <-> DB0
 * 
 * PortJ:
 * PTJ7 -> HEARTBEAT_LED
 * PTJ6 <- KEY7
 * PTJ5 <- KEY6
 * PTJ4 <- KEY5
 * PTJ3 <- KEY4
 * PTJ2 <- KEY3
 * PTJ1 <- KEY2
 * PTJ0 <- KEY1
 * PTJDD = 0x80;
 * 
 * */

void SetupMCUPeripherals(void);
void SetupCommunicationBuffers(void);
void resetModemCommPort(void);
void hwKeysInit(void);
uint8_t testingStartPressed(void);
uint8_t hwStartPressed(void);
void ScanKeys(void);

#endif
