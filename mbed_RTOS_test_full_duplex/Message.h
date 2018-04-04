/*********************************************
Noms : Axel Bosco et Alexandre Gagnon
CIP : bosa2002 et gaga2515
Date : 3 janvier 2018
*********************************************
*	Fichier d'en-tête pour l'implémentation de
* l'encodage d'un message en trame Manchester.
*********************************************/

#ifndef MESSAGE_H
#define MESSAGE_H

#include "mbed.h"

#define PREAMBLE 0b01010101
#define DELIMITER 0b01111110
#define TYPEANDFLAGS 0b00000000

class Message
{
public:
	uint8_t* frame; // Message converti en trame Manchester
	uint32_t length; // Longueur de la trame

	Message(uint32_t size);
	~Message() {}
	void encode(char * msg);
	void decode();
		
	static uint16_t calculateCRC(char * data, uint8_t * length);
private:
	uint8_t _msgLength; // Longueur du message à convertir
	uint8_t _iterator;
	uint8_t _bitIterator;

	void convert(uint8_t data);
};

#endif
