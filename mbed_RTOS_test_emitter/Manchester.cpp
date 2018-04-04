#include "Manchester.h"

Serial pc(USBTX, USBRX);
bool isNotFirstRun = false;
int _messageCpt =0;
uint8_t _message [256];

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

	_timer.start();
	
	_rx.disable_irq();
	
	_rx.rise(callback(this, &Manchester::inputDetectedUp));
	_rx.fall(callback(this, &Manchester::inputDetectedDown));
	printBit.start(callback(this,&Manchester::printData));
	printPreamble.start(callback(this,&Manchester::print));
	//_rx.rise(this, &Manchester::inputDetectedUp);
	//_rx.fall(this, &Manchester::inputDetectedDown);
	
	_rx.enable_irq();
}

void Manchester::printData (void) {
  while (1) {
    Thread::signal_wait(0x1);
    pc.printf("input bit is = %i \r\n", _inputBit);
    //pc.printf("CPT = %i , period = %i ,  period found = %i\r\n ", readCpt, period/1000, periodFound);
  }
}

void Manchester::print (void) {
  while (1) {
    Thread::signal_wait(0x1);
		pc.printf("CPT = %i , period = %i ,  period found = %i \r\n", _readCpt, _period / 1000, _meanPeriod);
		
		
	}
}


void Manchester::prepareTransmission()//uint8_t * frame, int length)
{
	while(1)
	{
		bool txFinished = false;
		_sendingState = SYNCH;
		
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
		
		Thread::wait(100);
	}
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

void Manchester::receive()
{
	while(1)
	{
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
	

		if(_receptionState == ERROR)
		{
			pc.printf("Erreur\r\n");
		}
		
		_timer.reset();
		isNotFirstRun = true;
		Thread::wait(100);
	}
}

void Manchester::readBit(int bit) {
  _inputBit = bit;
	printBit.signal_set(0x1);
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

void Manchester::inputDetectedUp(void) {
	switch(_receptionState)
	{
		case WAITING:
			break;
		case SYNCH:
			// if we have the preambule
			_period += _timer.read_us();
			_timer.reset();
			_readCpt++;
		
			printPreamble.signal_set(0x1);

		if (_readCpt == 7) {
				if(isNotFirstRun)
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
			printPreamble.signal_set(0x1);
		
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
