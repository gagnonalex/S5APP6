#include "Manchester.h"

Manchester::Manchester(PinName txPin, PinName rxPin, uint32_t speed): _tx(txPin), _rx(rxPin)
{
	_state = WAITING;
	_midPeriod = speed / 2;
	_tx = 0;
}

void Manchester::prepareTransmission(uint8_t * frame, int length)
{
	bool txFinished = false;
	_state = SYNCH;
	_frame = frame;
	_length = length - 14;
	
	_iterator = 0;
	_tickerIterator = 0;
	
	_ticker.attach_us(callback(this, &Manchester::sending), _midPeriod);
	
	do
	{
		core_util_critical_section_enter();
		txFinished = (_state == WAITING);
		core_util_critical_section_exit();
	} while (!txFinished);
	
	_ticker.detach();
}

void Manchester::sending()
{
	if(7 < _tickerIterator )
	{
		_iterator++;
		_tickerIterator = 0;
	}
	
	send();
	
	switch(_state)
	{
		case SYNCH:			
			if(_iterator > 1)
			{
				_state = START;
			}
			break;
		case START:
			if(_iterator > 3)
			{
				_state = FLAG;
			}
			break;
		case FLAG:
			if(_iterator > 7)
			{
				_state = DATA;
			}
			break;
		case DATA:
			if(_iterator > 8 + _length)
			{
				_state = CRC;
			}
			break;
		case CRC:		
			if(_iterator > 11 + _length)
			{
				_state = END;
			}
			break;
		case END:
			if(_iterator > 13 + _length)
			{
				_state = WAITING;
				_tx = 0;
			}
			break;
		case WAITING:
		default:			
			break;
	}
}

void Manchester::send()
{
	_tx = ((_frame[_iterator] & (1 << (7 - _tickerIterator))) >> (7 - _tickerIterator));
	
	_tickerIterator++;
}
