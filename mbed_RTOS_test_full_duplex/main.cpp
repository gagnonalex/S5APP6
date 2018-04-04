/*********************************************
Noms : Axel Bosco et Alexandre Gagnon
CIP : bosa2002 et gaga2515
Date : 3 janvier 2018
*********************************************/

#include "mbed.h"
#include "rtos.h"
#include "Manchester.h"
#include "Message.h"

Thread send;
Thread receive;

/***********************************************************
Titre : Main
Arguments en entr�e : Aucun
Arguments en sortie : Aucun
Fonctionnalit� : Initialise toutes les t�ches du programme.
***********************************************************/
int main()
{
	char msg [7] = {0x44, 0x4f, 0x4d, 0x49, 0x4e, 0x47, 0x4f}; // DOMINGO
	
	Manchester manchester(p21, p5, 120000);
	
	Message trame(7);
	trame.encode(msg);

	manchester._frame = trame.frame;
	manchester._length = (trame.length-14); // -14, car il y a 14 octets constants dans la trame manchester.

	send.start(callback(&manchester, &Manchester::prepareTransmission)); // D�marrage du thread d'envoi dans l'objet manchester
	receive.start(callback(&manchester, &Manchester::receive)); // D�marrage du thread de r�ception dans l'objet manchester
	
	while(1)
	{
		Thread::wait(osWaitForever);
	}
}
