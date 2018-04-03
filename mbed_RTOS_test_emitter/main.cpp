#include "mbed.h"
#include "rtos.h"


DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

DigitalOut testOutput(p10);
InterruptIn messageDetection(p5);
Timer timer;
Thread printBit;

Serial pc(USBTX, USBRX);



/** test section **/
Ticker flipper;
int arrTest[] = { 0,0,0,0,0,0,0,    0,1, 1,0, 0,1, 1,0, 0,1, 1,0, 0,1, 1,0,    0,1, 1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 0,1};

int testCpt = 0;

/** fin test section **/
int messageLen = 256;
int arr[256];
int lastBit;
int preambule[8];
int confirmPreambule[] = {
    0,
    1,
    0,
    1,
    0,
    1,
    0,
    1
};

int start[8];
int confirmStart[] = {0,1,1,0,1,0,1,0,1,0,1,0,1,0,0,1};

int end[8];
int confirmEnd[] = {
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    1
};

int readCpt = 0;
int startCpt =0;
int byteCpt =0;

int inputBit;
int periodFound = 0;
int period = 0;
// state determine actions
bool notReading = true;
bool preambuleDetection = false;
bool startDetection = false;
bool inMessageReading = false;
bool EndMessageDetection = false;
bool isEndOfMessage = false;

/*** 
utils to simplify code
***/
bool isTwoArrayTheSame(int arr_try[], int arrConfirm[], int len) {
    bool isSame = true;

    for (int i = 0; i < len; i++) {
        if (arr_try[i] != arrConfirm[i]) {
            isSame = false;
        }
    }
    return isSame;
}


void printArr(int a[]) {

    for (int i = 0; i < 8; i++) {
        pc.printf("%i", a[i]);
    }
    pc.printf("\r\n");
}

/***
Input Detections
***/
void printData(void){
	while(1){
		 Thread::signal_wait(0x1);
		 pc.printf("input bit is = %i \r\n", inputBit);
		 //pc.printf("CPT = %i , period = %i ,  period found = %i\r\n ", readCpt, period/1000, periodFound);
	}
	
}
void readBit(int bit){
	inputBit = bit;
	printBit.signal_set(0x1);
	
}

void inputDetectedUp(void) {
    if (notReading) return;

    if (preambuleDetection) {
        // if we have the preambule
    

            myled3 = 1;
            period += timer.read_us();
            timer.reset();
            readCpt++;
			
			   if (readCpt == 7) {

                periodFound = (period / 7 ) *0.8;
                preambuleDetection = false;
                startDetection = true;
                myled4 = 1;
								readBit(0);
                //readCpt = 0; 
            }
					
        
    } else if (timer.read_us() > periodFound) {
			timer.reset();
        readBit(0);
    }


}

void inputDetectedDown(void) {
    if (notReading) {
        notReading = false;
        preambuleDetection = true;
        period = 0;
        timer.reset();
    } else if (preambuleDetection) {
				//readBit(1); test purpose
        myled3 = 0;
        period += timer.read_us();
        timer.reset();
        preambule[readCpt] = 0;
        lastBit = 0;

        readCpt++;
    } else if (timer.read_us() > periodFound) {
        readBit(1);
						timer.reset();
    }
}

void flip() {
    myled2 = !myled2;

    if (testCpt > (sizeof(arrTest) / sizeof( * arrTest))) {
        testCpt = 0;
    }
    testOutput = arrTest[testCpt];
    testCpt++;
}


/*** 
			Main
***/
int main() {

   	timer.start();
    messageDetection.rise( & inputDetectedUp);
    messageDetection.fall( & inputDetectedDown);
    printBit.start( & printData);

   
    myled2 = 0;
    bool triggerOnce = false;
	wait(0.4);
 flipper.attach( & flip, 0.2);
    while (1) {
        if (!startDetection ) {
            pc.printf("CPT = %i , period = %i ,  period found = %i \r\n", readCpt, period/1000, periodFound);
						printArr(preambule);
        }
        wait(0.2);
    }

}