#include "Message.h"

uint16_t CRC16 = 0x8005;

Message::Message(uint32_t size) : frame(new uint8_t[(size * 2) + 14])
{
	length = (size * 2) + 14;
	_msgLength = size;
	_iterator = 0;
	_bitIterator = 0;
	
	memset(frame, 0, length);
}

void Message::encode(char * msg)
{
	uint16_t crc = calculateCRC(msg);
	
	convert(PREAMBLE);
	convert(DELIMITER);
	convert(TYPEANDFLAGS);
	convert((uint8_t) _msgLength);
	
	for(uint8_t i = 0; i < _msgLength; i++)
	{
		convert((uint8_t) msg[i]);
	}
	
	convert((uint8_t) (crc >> 8));
	convert((uint8_t) crc);
	convert(DELIMITER);
}

void Message::convert(uint8_t data)
{
	uint8_t manchesterEncoded[2] = {0x00,0x00};
	uint8_t bit = 0;
	
	for(_bitIterator = 0; _bitIterator < 8; _bitIterator++)
	{
		bit = ((data & (1 << _bitIterator)) >> _bitIterator);
		
		if(bit)
		{
			manchesterEncoded[_bitIterator/4] |= ((0b10 << (2 * (_bitIterator % 4))) & 0xFF);
		}
		else
		{
			manchesterEncoded[_bitIterator/4] |= ((0b01 << (2 * (_bitIterator % 4))) & 0xFF);
		}
	}
	
	frame[_iterator] = manchesterEncoded[1];
	_iterator++;
	frame[_iterator] = manchesterEncoded[0];
	_iterator++;
}

uint16_t Message::calculateCRC(char * data)
{
	uint16_t crc = 0xFFFF;

	for (uint32_t i = 0; i < _msgLength; i++)
	{
		crc  = (uint8_t)(crc >> 8) | (crc << 8);
		crc ^= data[i];
		crc ^= (uint8_t)(crc & 0xFF) >> 4;
		crc ^= (crc << 8) << 4;
		crc ^= ((crc & 0xFF) << 4) << 1;
	}
	return crc;
}
