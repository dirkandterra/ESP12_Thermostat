//--------------------------------------------------------
//  $Id: queue.h 303 2010-07-08 03:43:32Z david $
//  $Date: 2010-07-08 11:43:32 +0800 (Thu, 08 Jul 2010) $
//  $Rev: 303 $
//  Brief description: 	serial RX queue handling                       
//  Last changed by $Author: david $
//--------------------------------------------------------
#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
	int8_t count;
	int8_t head;
	int8_t tail;
	uint8_t size;
	uint8_t *buffer;
	uint8_t port;
	uint8_t responseSkip;			//Don't log RX unit seen number of chars
	uint8_t charSkip;               //Character to skip
} CIRC;

void initque(CIRC *que, uint8_t *addr, uint8_t len, uint8_t port);
uint8_t getque(CIRC *que, uint8_t *ch);

#endif

