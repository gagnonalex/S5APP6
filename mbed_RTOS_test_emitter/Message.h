#ifndef MESSAGE_H
#define MESSAGE_H

#include "mbed.h"

#define PREAMBLE 0b01010101
#define DELIMITER 0b01111110
#define TYPEANDFLAGS 0b00000000

class Message
{
public:
	uint8_t* frame;
	uint32_t length;

	Message(uint32_t size);
	~Message() {}
	void encode(char * msg);
	void decode();
private:
	uint8_t _msgLength;
	uint8_t _iterator;
	uint8_t _bitIterator;

	void convert(uint8_t data);
	uint16_t calculateCRC(char * data);
};

#endif
