#ifndef MANCHESTER_H
#define MANCHESTER_H

#include "mbed.h"

class Manchester{
	
	enum State{
		SYNCH,
		START,
		FLAG,
		DATA,
		CRC,
		END,
		RECEIVE,
		WAITING
};
	
public:
	Manchester(PinName txPin, PinName rxPin, uint32_t speed);
	~Manchester(void){ }
	void prepareTransmission(uint8_t * frame, int length);
	bool receive();
private:
	DigitalOut _tx;
	InterruptIn _rx;
	State _state;
	uint32_t _midPeriod;
	uint8_t _iterator;
	uint8_t * _frame;
	uint8_t _length;
	Ticker _ticker;
	uint8_t _tickerIterator;


	void sending();
	void send();
};
#endif
