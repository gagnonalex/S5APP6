#include "mbed.h"
#include "rtos.h"
#include <iostream>


DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalIn reader (p5);
Serial pc(USBTX, USBRX);
InterruptIn messageDetection(p14);
Timer timer;


int messageLen = 256;
int arr[256];
int lastBit; 
int preambule[8];
int confirmPreambule [] = {0,1,0,1,0,1,0,1};

int start[8];
int confirmStart[] = {0,1,1,1,1,1,1,0};

int end[8];
int confirmEnd[] = {1,0,0,0,0,0,0,1};

int readCpt = 0;

int periodFoud = 0;
int period = 0;
// state determine actions
bool preambuleDetection = false;
bool startDetection = false;
bool inMessageReading = false;
bool EndMessageDetection = false;
bool isEndOfMessage = false;

/*** 
utils to simplify code
***/
bool isTwoArrayTheSame(int arr_try[] ,int arrConfirm[], int len ){
	bool isSame = true;
	
	for(int i=0; i<len; i++){
		if(arr_try[i]!=arrConfirm[i]){
			isSame = false;
		}
	}
	return isSame;
}


 void printArr(){
	 pc.printf("voici le array recu   :")
	 for(int i = 0; i<messageLen;i++){
		 pc.printf(arra)
	 }
 }

 /*** 
step change in function
***/
 
 
 /*** 
 interrupt up and down
***/
void inputDetectedUp(void)
{
	if(preambuleDetection){
		preambule[readCpt] = 1;
		readCpt++;
	}
}

void inputDetectedDown(void)
{
	if(preambuleDetection){
		if( 0 == confirmPreambule[readCpt]){
			period += timer.read_us(); 
		preambule[readCpt] = 0;
		lastBit = 0;	
		readCpt++;	
		}
		else{
			if(lastBit ==0){
				readCpt = 1;
				preambule[readCpt-1] = lastBit;
			}
			else{
				readCpt = 0;
			}
		}
		
		if(readCpt == 7 && isTwoArrayTheSame(preambule,confirmPreambule,sizeof(confirmPreambule))){
			periodFoud = period/7;
			preambuleDetection = false;
			startDetection = true;
		}		
	}
	 
	
}

int main()
{
	messageDetection.rise(&inputDetectedUp);
	messageDetection.fall(&inputDetectedDown);

	while(1) {
		myled1 = !myled1;
		wait(500);
	}

}
