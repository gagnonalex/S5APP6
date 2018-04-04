/*********************************************
Noms : Axel Bosco et Alexandre Gagnon
CIP : bosa2002 et gaga2515
Date : 3 janvier 2018
*********************************************/

#include "Manchester.h"

Serial pc(USBTX, USBRX);

/*****************************************************************************************************
Titre : Manchester::Manchester
Arguments en entrée : txPin, rxPin, speed
Arguments en sortie : Aucun
Fonctionnalité : Constructeur pour l'objet Manchester. Initialise les pins et les valeurs par défaut.
******************************************************************************************************/
Manchester::Manchester(PinName txPin, PinName rxPin, uint32_t speed): _tx(txPin), _rx(rxPin), _message(new uint8_t[256])
{
	_sendingState = WAITING;
	_receptionState = WAITING;
	_midPeriod = speed / 2;
	_tx = 0;
	
	_inputBit = 0;
	_startByte = 0;
	_startCpt = 0;
	_byteMessage = 0;
	_byteCpt = 0;
	_readCpt = 0;
	_messageCpt = 0;
	
	_isNotFirstRun = false;
	
	_timer.start();
	_timerEnvoi.start();
	_timerReception.start();
	
	_rx.disable_irq();
	
	_rx.rise(callback(this, &Manchester::inputDetectedUp));
	_rx.fall(callback(this, &Manchester::inputDetectedDown));
	_printBit.start(callback(this,&Manchester::printData));
	_printPreamble.start(callback(this,&Manchester::print));
	
	_rx.enable_irq();
}

/*****************************************************************************************************
Titre : Manchester::printData
Arguments en entrée : Aucun
Arguments en sortie : Aucun
Fonctionnalité : Écrit sur le port Serial pc la valeur du bit reçu par l'InterruptIn après la réception du signal 0x1.
******************************************************************************************************/
void Manchester::printData (void) {
  while (1) {
    Thread::signal_wait(0x1);
    pc.printf("input bit is = %i \r\n", _inputBit);
  }
}

/*****************************************************************************************************
Titre : Manchester::print
Arguments en entrée : Aucun
Arguments en sortie : Aucun
Fonctionnalité : Écrit sur le port Serial pc les valeurs de période et période moyenne calculées après la
réception du signal 0x1.
******************************************************************************************************/
void Manchester::print (void) {
  while (1) {
    Thread::signal_wait(0x1);
		pc.printf("CPT = %i , period = %i ,  period found = %i \r\n", _readCpt, _period / 1000, _meanPeriod);
	}
}

/*****************************************************************************************************
Titre : Manchester::prepareTransmission
Arguments en entrée : Aucun
Arguments en sortie : Aucun
Fonctionnalité : Fonction pour le thread démarré dans le main.cpp. S'occupe de boucler et de démarrer
le Ticker pour l'envoi sur le port Tx. La vérification si l'envoi est terminé est dans une zone critique.
******************************************************************************************************/
void Manchester::prepareTransmission()
{
	uint32_t tempsEnvoi;
	while(1)
	{
		_timerEnvoi.reset();
		bool txFinished = false;
		_sendingState = SYNCH; // Initialise l'état de la machine initial
		
		_iterator = 0;
		_tickerIterator = 0;
		
		_ticker.attach_us(callback(this, &Manchester::sending), _midPeriod);
		
		do
		{
			core_util_critical_section_enter(); // Entre en zone critique pour valider que l'envoi est terminé.
			txFinished = (_sendingState == WAITING);
			core_util_critical_section_exit(); // Sors de la zone critique.
		} while (!txFinished); 
		
		_ticker.detach(); // Détache le ticker pour qu'il se termine.
		
		tempsEnvoi = _timerEnvoi.read_us();
		
		pc.printf("Duree d'envoi : %d en us\r\n", tempsEnvoi);
		
		Thread::wait(100);
	}
}

/*****************************************************************************************************
Titre : Manchester::sending
Arguments en entrée : Aucun
Arguments en sortie : Aucun
Fonctionnalité : Machine à état finie pour l'envoi d'une trame Manchester.
******************************************************************************************************/
void Manchester::sending()
{
	if(7 < _tickerIterator )
	{
		_iterator++;
		_tickerIterator = 0;
	}
	
	send();
	
	switch(_sendingState)
	{
		case SYNCH:			
			if(_iterator > 1)
			{
				_sendingState = START;
			}
			break;
		case START:
			if(_iterator > 3)
			{
				_sendingState = FLAG;
			}
			break;
		case FLAG:
			if(_iterator > 7)
			{
				_sendingState = DATA;
			}
			break;
		case DATA:
			if(_iterator > 8 + _length)
			{
				_sendingState = CRC;
			}
			break;
		case CRC:		
			if(_iterator > 11 + _length)
			{
				_sendingState = END;
			}
			break;
		case END:
			if(_iterator > 13 + _length)
			{
				_sendingState = WAITING;
				_tx = 0;
			}
			break;
		case WAITING:
		default:			
			break;
	}
}

/*****************************************************************************************************
Titre : Manchester::send
Arguments en entrée : Aucun
Arguments en sortie : Aucun
Fonctionnalité : Fonction appelée par la machine à état finie pour envoyer le bit sur le port Tx.
******************************************************************************************************/
void Manchester::send()
{
	_tx = ((_frame[_iterator] & (1 << (7 - _tickerIterator))) >> (7 - _tickerIterator));
	
	_tickerIterator++;
}

/*****************************************************************************************************
Titre : Manchester::receive
Arguments en entrée : Aucun
Arguments en sortie : Aucun
Fonctionnalité : Fonction pour le thread démarré dans le main.cpp. S'occupe de boucler et de vérifier
si la réception est finie et de recalculer le CRC16 du message reçu et le comparer à celui du message.
******************************************************************************************************/
void Manchester::receive()
{
	uint32_t dureeReception;
	while(1)
	{
		_timerReception.reset();
		
		bool rxFinished;
		_meanPeriod =0;
		_period = 0;
		_readCpt = 0;
		_startCpt =0;
		_byteCpt= 0 ;
		_receptionState = WAITING;
    _rx.enable_irq();
 
    do {
        core_util_critical_section_enter();
        rxFinished = ((_receptionState == END) || (_receptionState == ERROR));
        core_util_critical_section_exit();
    } while(!rxFinished);
		
    _rx.disable_irq();
	
		_timer.reset();
		_isNotFirstRun = true;
		
		if(_receptionState == ERROR)
		{
			pc.printf("Erreur\r\n");
		}
		else
		{ // Conversion des bits reçus en caractère et calcule du CRC16		
			uint8_t frameLength = _message[1];
			char messageConverted[frameLength];
			
			for(int i = 0; i < frameLength; i++)
			{
				messageConverted[i] = _message[i + 2];
			}
			
			uint16_t crc = Message::calculateCRC(messageConverted, &frameLength);
			
			if((((uint8_t) (crc >> 8)) == _message[2 + frameLength]) && (((uint8_t) crc) == _message[3 + frameLength]))
			{
				pc.printf("Trame recue est bonne!");
			}
			else
			{
				pc.printf("Trame recue erronee");
			}
		}
		
		dureeReception = _timerReception.read_us();
		
		pc.printf("Duree de reception : %d\r\n", dureeReception);
		
		Thread::wait(100);
	}
}

/*****************************************************************************************************
Titre : Manchester::readBit
Arguments en entrée : bit
Arguments en sortie : Aucun
Fonctionnalité : Fonction faisant partie de la machine à état finie pour la réception recevant le bit
déterminé par l'InterruptIn et le traite selon l'état de la machine.
******************************************************************************************************/
void Manchester::readBit(int bit) {
  _inputBit = bit;
	_printBit.signal_set(0x1);
	switch(_receptionState)
	{
		case START:
			
			_startByte <<= 1;
			_startByte |= bit;
			_startCpt++;

			if (_startCpt == 8) {
				if(_startByte == 0b01111110)
				{
					_receptionState = RECEIVE;
				}
				else
				{
					_startCpt--;
				}
			}
			break;
		case RECEIVE:
			_byteMessage <<= 1;
			_byteMessage |= bit;
			_byteCpt++;
			
		
			if (_byteCpt == 8) {
				if (_byteMessage == 0b01111110) {
					_receptionState = END;
				} 
				else
				{
					_byteCpt = 0;
					_message[_messageCpt] = _byteMessage;
					_messageCpt++;

				}
			}
			break;
			
	}
}

/*****************************************************************************************************
Titre : Manchester::inputDetectedUp
Arguments en entrée : Aucun
Arguments en sortie : Aucun
Fonctionnalité : InterruptIn sur front montant qui fait partie de la machine à état finie de la réception
et traite l'interruption selon l'état.
******************************************************************************************************/
void Manchester::inputDetectedUp(void) {
	switch(_receptionState)
	{
		case WAITING:
			break;
		case SYNCH:
			_period += _timer.read_us();
			_timer.reset();
			_readCpt++;
		
			_printPreamble.signal_set(0x1);

		if (_readCpt == 7) {
				if(_isNotFirstRun)
				{
					_meanPeriod = ((_period-5000000) / 7) * 0.8;
				}
				else{
					_meanPeriod = (_period / 7) * 0.8;
				}
				_receptionState = START;
				_startByte = 0;
		
				readBit(0);
				_readCpt = 0; 
			}
			break;
		case END:
			return;
		default:
			if (_timer.read_us() > _meanPeriod) {
				_timer.reset();
				readBit(0);
			}
			break;
	}
}

/*****************************************************************************************************
Titre : Manchester::inputDetectedDown
Arguments en entrée : Aucun
Arguments en sortie : Aucun
Fonctionnalité : InterruptIn sur front descendant qui fait partie de la machine à état finie de la réception
et traite l'interruption selon l'état.
******************************************************************************************************/
void Manchester::inputDetectedDown()
{	
	switch(_receptionState)
	{
		case WAITING:
			_receptionState = SYNCH;
			_period = 0;
			_timer.reset();
			break;
		case SYNCH:
			_period += _timer.read_us();
			_timer.reset();
			_printPreamble.signal_set(0x1);
		
			_readCpt++;
			break;
		case START:
		case RECEIVE:
			if (_timer.read_us() > _meanPeriod)
			{
				readBit(1);
				_timer.reset();
			}
			break;
			
		case END:
			return;
		
	}
}
