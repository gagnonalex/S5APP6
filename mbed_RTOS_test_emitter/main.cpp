#include "mbed.h"
#include "rtos.h"
#include "Manchester.h"
#include "Message.h"

Thread send;
Thread receive;

int main()
{
	Manchester manchester(p21, p5, 120000); //(tx,rx);
		
	char msg [7] = {0x44, 0x4f, 0x4d, 0x49, 0x4e, 0x47, 0x4f}; // DOMINGO
	Message trame(7);
	
	trame.encode(msg);

	manchester._frame = trame.frame;
	manchester._length = (trame.length-14);

	send.start(callback(&manchester, &Manchester::prepareTransmission));
	receive.start(callback(&manchester, &Manchester::receive));
	
	while(1)
	{
		Thread::wait(osWaitForever);
	}
}
