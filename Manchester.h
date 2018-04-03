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
	};
	
	struct messageInfo 
	{
		uint8_t byte;
		char ascii;
	};
	
public:
	Manchester(PinName txPin, PinName rxPin, uint32_t speed);
	~Manchester(void){ }
	void prepareTransmission(uint8_t * frame, int length);
	bool receive();
private:
	DigitalOut _tx;
	InterruptIn _rx;
	Timer _timer;
	Thread printBit;
	Thread printPreamble;

	State _sendingState;
	State _receptionState;

	uint32_t _midPeriod;
	uint32_t _meanPeriod;
	uint32_t _period;

	uint8_t * _frame;
	uint8_t * _received;

	uint8_t _iterator;
	uint8_t _length;
	Ticker _ticker;
	uint8_t _tickerIterator;

	uint8_t _inputBit;
	uint8_t _startByte;
	uint8_t _startCpt;
	uint8_t _byteMessage;
	uint8_t _byteCpt;
	uint8_t _readCpt;
	uint8_t _bitCpt;

	void sending();
	void send();
	void inputDetectedUp();
	void inputDetectedDown();
	void readBit(int bit);
	void printData(void);
	void print();
};
#endif
