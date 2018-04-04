/*********************************************
Noms : Axel Bosco et Alexandre Gagnon
CIP : bosa2002 et gaga2515
Date : 3 janvier 2018
*********************************************/

#include "Message.h"

/*****************************************************************************************************
Titre : Message::Message
Arguments en entrée : size
Arguments en sortie : Aucun
Fonctionnalité : Constructeur pour l'objet Manchester. Initialise les valeurs par défaut.
******************************************************************************************************/
Message::Message(uint32_t size) : frame(new uint8_t[(size * 2) + 14])
{
	length = (size * 2) + 14;
	_msgLength = size;
	_iterator = 0;
	_bitIterator = 0;
	
	memset(frame, 0, length);
}

/*****************************************************************************************************
Titre : Message::encode
Arguments en entrée : *msg
Arguments en sortie : Aucun
Fonctionnalité : Encode le message à envoyer en trame Manchester.
******************************************************************************************************/
void Message::encode(char * msg)
{
	uint16_t crc = calculateCRC(msg, &_msgLength);
	
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

/*****************************************************************************************************
Titre : Message::convert
Arguments en entrée : data
Arguments en sortie : Aucun
Fonctionnalité : Converti la donnée en 8 bits en 16 bits respectant la nomenclature Manchester.
1 -> 0 pour un 1,
0 -> 1 pour un 0.
******************************************************************************************************/
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

/*****************************************************************************************************
Titre : Message::calculateCRC
Arguments en entrée : *data, *length
Arguments en sortie : Aucun
Fonctionnalité : Calcule le CRC16 par la méthode du CRC-CCITT
******************************************************************************************************/
uint16_t Message::calculateCRC(char * data, uint8_t * length)
{
	uint16_t crc = 0xFFFF;

	for (uint32_t i = 0; i < *length; i++)
	{
		crc  = (uint8_t)(crc >> 8) | (crc << 8);
		crc ^= data[i];
		crc ^= (uint8_t)(crc & 0xFF) >> 4;
		crc ^= (crc << 8) << 4;
		crc ^= ((crc & 0xFF) << 4) << 1;
	}
	return crc;
}
