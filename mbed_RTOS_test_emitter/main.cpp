#include "mbed.h"
#include "rtos.h"
#include "Manchester.h"
#include "Message.h"

int main()
{
	Manchester manchester(p21, p17, 120000); //(tx,rx);
	
	/*uint8_t msg[14] = {	0b01100110, 0b01100110,
											0b01101010, 0b10101001,
											0b01010101, 0b01010101,
											0b01010101, 0b01010101,
											0b01010101, 0b01010101,
											0b01010101, 0b01010101,
											0b01101010, 0b10101001};
	
		*/									
	char msg [5] = {0x50, 0x45, 0x4e, 0x49, 0x53}; // ALLO
	Message trame(5);
										
	trame.encode(msg);
	while(1)
	{
		manchester.prepareTransmission(trame.frame, trame.length);
		wait(5);
	}
}
