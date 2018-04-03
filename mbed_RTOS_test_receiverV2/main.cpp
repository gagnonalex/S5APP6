
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

char messageByte = 0;

struct messageInfo {
  uint8_t byte;
  char ascii;
};

/** test section **/
Ticker flipper;
int arrTest[] = {
    0,0,0,0,0,0,0,
		0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0, // preambule
    0,1,1,0,1,0,1,0,1,0,1,0,1,0,0,1, // delimiter start
		0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1, // flag 0000
		0,1,1,0,0,1,0,1,0,1,0,1,0,1,1,0,  0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,  0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,  0,1,1,0,0,1,0,1,1,0,1,0,1,0,1,0,// = 0,1,0,0,0,0,0,1, 0,1,0,0,1,1,0,0, 0,1,0,0,1,1,0,0, 0,1,0,0,1,1,1,1, // allo 
	  0,1,1,0,1,0,1,0,1,0,1,0,1,0,0,1, // delimiter end
};

int testCpt = 0;

/** fin test section **/
int messageLen = 256;
messageInfo message[256];

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
int confirmLimit[] = {
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  0
};

int readCpt = 0;
int startCpt = 0;
int byteCpt = 0;
int messageCpt = 0;

uint8_t byteMessage = 0;
int inputBit;
int inputByte;
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
void printData(void) {
  while (1) {
    Thread::signal_wait(0x1);
    pc.printf("input bit is = %i \r\n", inputBit);
    //pc.printf("CPT = %i , period = %i ,  period found = %i\r\n ", readCpt, period/1000, periodFound);
  }

}
void readBit(int bit) {
  inputBit = bit;
  printBit.signal_set(0x1);

  if (startDetection) {

    start[startCpt] = bit;
    startCpt++;

    if (startCpt == 8) {

      if (isTwoArrayTheSame(start, confirmLimit, 8)) {
        myled1 = 1;

        startDetection = false;
        inMessageReading = true;
      }
    }

  } else if (inMessageReading) {
    byteMessage <<= 1;
    byteMessage |= bit;
    byteCpt++;

    if (byteCpt == 8) {
      if (byteMessage == 0b01111110) {
        myled1 = !myled1;
				messageInfo info;
        info.byte = byteMessage;
        message[messageCpt] = info;
        //its the end of the message
      } else {
        byteCpt = 0;
        messageInfo info;
        info.byte = byteMessage;
        message[messageCpt] = info;
        messageCpt++;

      }
    }
  }

}

void inputDetectedUp(void) {
  if (notReading) return;

  if (preambuleDetection) {
    // if we have the preambule

    myled3 = 1;
    period += timer.read_us();
    timer.reset();
		preambule[readCpt] = 0;
    readCpt++;

    if (readCpt == 7) {
			preambule[readCpt] = 1;
      periodFound = (period / 7) * 0.8;
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
    preambule[readCpt] = 1;
    myled3 = 0;
    period += timer.read_us();
    timer.reset();
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
  wait(0.4);
  flipper.attach( & flip, 0.02);
  while (1) {
    if (!startDetection && !inMessageReading) {
      pc.printf("CPT = %i , period = %i ,  period found = %i \r\n", readCpt, period / 1000, periodFound);
      // printArr(preambule);
    }
    wait(0.1);
  }

}