/*********************************************
Noms : Axel Bosco et Alexandre Gagnon
CIP : bosa2002 et gaga2515
Date : 3 janvier 2018
*********************************************
*	Fichier d'en-tête pour l'implémentation de l'envoi et de la réception
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
	}; // Machine à état finie du programme
	
	struct messageInfo 
	{
		uint8_t byte;
	}; // Structure recevant la trame décodée lors d'une réception.
	
public:
	Manchester
	(
	PinName txPin, // Pin de transmission
	PinName rxPin, // Pin de réception
	uint32_t speed // Vitesse de transfert en us
	);
	~Manchester(void){ }
	
	uint8_t * _frame; // Pointeur contenant la trame manchester encodée
	uint8_t _length; // Longueur du message encodé
	
	void prepareTransmission();
	void receive();
private:
	DigitalOut _tx; // Transmission
	InterruptIn _rx; // Réception
	Timer _timer;
	Thread _printBit; // Thread d'écriture pour la lecture des bits
	Thread _printPreamble; // Thread d'écriture pour la lecture du préambule
	Ticker _ticker; // Ticker s'occupant d'envoyer la trame manchester

	State _sendingState; // État de l'envoi
	State _receptionState; // État de la réception

	uint32_t _midPeriod; // Mi-période pour l'envoi
	uint32_t _meanPeriod; // Période moyenne déterminée par le préambule reçu
	uint32_t _period; // Période par chaque itération

	uint8_t _iterator;
	uint8_t _tickerIterator;

	uint8_t _inputBit;
	uint8_t _startByte; // Contenant pour les valeurs reçues pour déterminer le start
	uint8_t _startCpt; // Compteur de lecture
	uint8_t _byteMessage; // Contenant pour les bits reçus après le start delimiter
	uint8_t _byteCpt; // Compteur de lecture
	uint8_t _readCpt; // Compteur de lecture
	uint8_t _messageCpt; // Compteur d'écriture

	uint8_t * _message; // Array contenant le message reçu.

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
