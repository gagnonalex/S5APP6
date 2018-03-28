#include "mbed.h"
#include "rtos.h"

DigitalOut tx(p20);

int preambule[8] = {0,1,0,1,0,1,0,1};
int start[8] = {0,1,1,1,1,1,1,0};

int cycle = 10;

void sendPreambule(){
	for(int i = 0; i < 8; i++){
		tx = preambule[i];
		Thread::wait(cycle);
	}
}	

void sendStart(){
	for(int i = 0; i < 8; i++){
		tx = start[i];
		
		if(i < 7)
		{
			if(start[i+1] == 0){
				Thread::wait(cycle/2);
				tx = 1;
			}
			else{
				Thread::wait(cycle/2);
				tx = 0;
			}
		}
		
		Thread::wait(cycle/2);
	}
}

int main()
{	
	tx = 1;
	
	while(1) {
		sendPreambule();
		tx = 1;
		Thread::wait(cycle);
		sendStart();
		tx = 1 ;
		Thread::wait(100);
	}
}

