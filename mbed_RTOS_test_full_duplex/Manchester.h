/*********************************************
Noms : Axel Bosco et Alexandre Gagnon
CIP : bosa2002 et gaga2515
Date : 3 janvier 2018
*********************************************
*	Fichier d'en-t�te pour l'impl�mentation de l'envoi et de la r�ception
* de trame Manchester.
*********************************************/

#ifndef MANCHESTER_H
#define MANCHESTER_H

#include "mbed.h"
#include "rtos.h"
#include "Message.h"

class Manchester{
	
	enum State
	{
		SYNCH,
		START,
		FLAG,
		DATA,
		CRC,
		END,
		RECEIVE,
		WAITING,
		ERROR
	}; // Machine � �tat finie du programme
	
	struct messageInfo 
	{
		uint8_t byte;
	}; // Structure recevant la trame d�cod�e lors d'une r�ception.
	
public:
	Manchester
	(
	PinName txPin, // Pin de transmission
	PinName rxPin, // Pin de r�ception
	uint32_t speed // Vitesse de transfert en us
	);
	~Manchester(void){ }
	
	uint8_t * _frame; // Pointeur contenant la trame manchester encod�e
	uint8_t _length; // Longueur du message encod�
	
	void prepareTransmission();
	void receive();
private:
	DigitalOut _tx; // Transmission
	InterruptIn _rx; // R�ception
	Timer _timer;
	Thread _printBit; // Thread d'�criture pour la lecture des bits
	Thread _printPreamble; // Thread d'�criture pour la lecture du pr�ambule
	Ticker _ticker; // Ticker s'occupant d'envoyer la trame manchester

	State _sendingState; // �tat de l'envoi
	State _receptionState; // �tat de la r�ception

	uint32_t _midPeriod; // Mi-p�riode pour l'envoi
	uint32_t _meanPeriod; // P�riode moyenne d�termin�e par le pr�ambule re�u
	uint32_t _period; // P�riode par chaque it�ration

	uint8_t _iterator;
	uint8_t _tickerIterator;

	uint8_t _inputBit;
	uint8_t _startByte; // Contenant pour les valeurs re�ues pour d�terminer le start
	uint8_t _startCpt; // Compteur de lecture
	uint8_t _byteMessage; // Contenant pour les bits re�us apr�s le start delimiter
	uint8_t _byteCpt; // Compteur de lecture
	uint8_t _readCpt; // Compteur de lecture
	uint8_t _messageCpt; // Compteur d'�criture

	uint8_t * _message; // Array contenant le message re�u.

	bool _isNotFirstRun;

	void sending();
	void send();
	void inputDetectedUp();
	void inputDetectedDown();
	void readBit(int bit);
	void printData(void);
	void print();
};
#endif
