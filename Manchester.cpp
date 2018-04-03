#include "Manchester.h"

Manchester::Manchester(PinName txPin, PinName rxPin, uint32_t speed): _tx(txPin), _rx(rxPin)
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
	_bitCpt = 0;
	
	_timer.start();
	
	_rx.disable_irq();
	
	_rx.rise(callback(this, &Manchester::inputDetectedUp));
	_rx.fall(callback(this, &Manchester::inputDetectedDown));
	
	//_rx.rise(this, &Manchester::inputDetectedUp);
	//_rx.fall(this, &Manchester::inputDetectedDown);
	
	_rx.enable_irq();
}

void Manchester::prepareTransmission(uint8_t * frame, int length)
{
	bool txFinished = false;
	_sendingState = SYNCH;
	_frame = frame;
	_length = length - 14;
	
	_iterator = 0;
	_tickerIterator = 0;
	
	_ticker.attach_us(callback(this, &Manchester::sending), _midPeriod);
	
	do
	{
		core_util_critical_section_enter();
		txFinished = (_sendingState == WAITING);
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

void Manchester::send()
{
	_tx = ((_frame[_iterator] & (1 << (7 - _tickerIterator))) >> (7 - _tickerIterator));
	
	_tickerIterator++;
}

bool Manchester::receive()
{
		bool rxFinished;
	
    _rx.enable_irq();
 
    do {
        core_util_critical_section_enter();
        rxFinished = (_receptionState == END);
        core_util_critical_section_exit();
    } while(!rxFinished);
		
    _rx.disable_irq();
	
		_receptionState = WAITING;
		
		return true;
}

void Manchester::readBit(int bit) {
  _inputBit = bit;
 // printBit.signal_set(0x1);
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
			}
			break;
		case RECEIVE:
			_byteMessage <<= 1;
			_byteMessage |= bit;
			_byteCpt++;

			if (_byteCpt == 8) {
				if (_byteMessage == 0b01111110) {
					//myled1 = !myled1;
					//messageInfo info;
					//info.byte = byteMessage;
					//message[messageCpt] = info;
					printf("Terminé");
					_receptionState = END;
				//its the end of the message
				} 
				else
				{
					_byteCpt = 0;
					//_received[_byteCpt] = _byteMessage;
//					messageInfo info;
					//info.byte = _byteMessage;
					//message[messageCpt] = info;
					//messageCpt++;
				}
			}
			break;
	}
}

void Manchester::inputDetectedUp(void) {
	switch(_receptionState)
	{
		case WAITING:
			break;
		case SYNCH:
			// if we have the preambule

			//myled3 = 1;
			_period += _timer.read_us();
			_timer.reset();
			//preambule[readCpt] = 0;
			_readCpt++;

			if (_readCpt == 7) {
				//preambule[readCpt] = 1;
				_meanPeriod = (_period / 7) * 0.8;
				//preambuleDetection = false;
				//startDetection = true;
				_receptionState = START;
	//      myled4 = 1;
				readBit(0);
				//readCpt = 0; 
			}
			break;
		default:
			if (_timer.read_us() > _meanPeriod) {
				_timer.reset();
				readBit(0);
			}
			break;
	}
}


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
		default:
			break;
	}
}
