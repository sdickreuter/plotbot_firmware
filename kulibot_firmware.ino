
#include "Stepper.h"
#include <Bounce2.h>
#include <PITimer.h>
#include <PacketSerial.h>

#define CIRCULAR_BUFFER_INT_SAFE
#include "CircularBuffer.h"

// struct for storing step timings
struct dtData {
  long dt;
  int action;
};

// 1500 * 4 * 64 bit = 48 kbytes, Teensy 3.2 has 64 kbytes of RAM
// 3000 * 2* ( 32 bit + 8 bit) = 30 kbytes
CircularBuffer<dtData, 3000> xsteps;
CircularBuffer<dtData, 3000> ysteps;

//SerialTransfer transferbuffer;
PacketSerial_<COBS, 0, 512> myPacketSerial;


#define button 0
#define up 4
#define down 3
#define left 2
#define right 1

int stick_pin[] = {23,21,22,20,19};

Bounce button_bounce = Bounce(); 
Bounce up_bounce = Bounce(); 
Bounce down_bounce = Bounce(); 
Bounce left_bounce = Bounce(); 
Bounce right_bounce = Bounce(); 


#define topright_switch 3 // pin 9
#define topleft_switch 2 // pin 10
#define bottomleft_switch 1 // pin 11
#define bottomright_switch 0 // pin 12
int endswitch_pin[] = {11,10,9,8};

Bounce topright_bounce = Bounce(); 
Bounce topleft_bounce = Bounce(); 
Bounce bottomleft_bounce = Bounce(); 
Bounce bottomright_bounce = Bounce(); 

#define TOP_STEP 14
#define TOP_DIR 13
#define TOP_MS1 17
#define TOP_MS2 16
#define TOP_MS3 15
#define TOP_ENABLE 18

#define BOTTOM_STEP 3
#define BOTTOM_DIR 2
#define BOTTOM_MS1 6
#define BOTTOM_MS2 5
#define BOTTOM_MS3 4
#define BOTTOM_ENABLE 7

// LED pin ist schon belegt !!!
//const int ledPin = 13;
//boolean on = false;

elapsedMicros mu;
elapsedMicros pressed;

Stepper stepper_bottom(200, TOP_DIR, TOP_STEP, TOP_MS1,TOP_MS2, TOP_MS3, TOP_ENABLE);
Stepper stepper_top(200, BOTTOM_DIR, BOTTOM_STEP, BOTTOM_MS1, BOTTOM_MS2, BOTTOM_MS3, BOTTOM_ENABLE);

#define DELAYMU 200

// void forward_top(int arg_cnt, char **args) {
//   Stream *s = cmdGetStream();
//   int n = 0;
//   if(arg_cnt > 0) {
//       n = atoi(args[1]);
//   }
//   s->print("top, ");
//   s->print(n);
//   s->println(" steps forwards");
//   stepper_top.set_dir(false);
//   for (int i = 0; i<n; i++) {
//     step_top();
//     delayMicroseconds(DELAYMU);
//   }
// }

// void backward_top(int arg_cnt, char **args) {
//   Stream *s = cmdGetStream();
//   int n = 0;
//   if(arg_cnt > 0) {
//       n = atoi(args[1]);
//   }
//   s->print("top, ");
//   s->print(n);
//   s->println(" steps backwards");
//   stepper_top.set_dir(true);
//   for (int i = 0; i<n; i++) {
//     step_top();
//     delayMicroseconds(DELAYMU);
//   }
// }

// void forward_bottom(int arg_cnt, char **args) {
//   Stream *s = cmdGetStream();
//   int n = 0;
//   if(arg_cnt > 0) {
//       n = atoi(args[1]);
//   }
//   s->print("bottom, ");
//   s->print(n);
//   s->println(" steps forwards");
//   stepper_bottom.set_dir(true);
//   for (int i = 0; i<n; i++) {
//     step_bottom();
//     delayMicroseconds(DELAYMU);
//   }
// }

// void backward_bottom(int arg_cnt, char **args) {
//   Stream *s = cmdGetStream();
//   int n = 0;
//   if(arg_cnt > 0) {
//       n = atoi(args[1]);
//   }
//   s->print("bottom, ");
//   s->print(n);
//   s->println(" steps backwards");
//   stepper_bottom.set_dir(false);
//   for (int i = 0; i<n; i++) {
//     step_bottom();
//     delayMicroseconds(DELAYMU);
//   }
// }


void updatex() {
  //StepData x = { micros(), 1 };
  //xsteps.push(x);
  dtData x;
  x = xsteps.pop();
  delayMicroseconds(100);
}

void updatey() {
  dtData y;
  y = ysteps.pop();
  delayMicroseconds(100);
}


void setup() {
  stepper_bottom.setMicrostepping(3);
  stepper_top.setMicrostepping(3);
  
  for (int i = 0; i<4; i++) {
      pinMode(endswitch_pin[i], INPUT);
  }
  topright_bounce.attach(endswitch_pin[3]);
  topleft_bounce.attach(endswitch_pin[2]);
  bottomleft_bounce.attach(endswitch_pin[1]);
  bottomright_bounce.attach(endswitch_pin[0]);

  topright_bounce.interval(1); // interval in ms
  topleft_bounce.interval(1); // interval in ms
  bottomleft_bounce.interval(1); // interval in ms
  bottomright_bounce.interval(1); // interval in ms

  for (int i = 0; i<5; i++) {
      pinMode(stick_pin[i], INPUT);
  }
  button_bounce.attach(stick_pin[0]);
  up_bounce.attach(stick_pin[1]);
  down_bounce.attach(stick_pin[2]);
  left_bounce.attach(stick_pin[3]);
  right_bounce.attach(stick_pin[4]);

  button_bounce.interval(5); // interval in ms
  up_bounce.interval(5); // interval in ms
  down_bounce.interval(5); // interval in ms
  left_bounce.interval(5); // interval in ms
  right_bounce.interval(5); // interval in ms
  
    
 // We begin communication with our PacketSerial object by setting the
  // communication speed in bits / second (baud).
  myPacketSerial.begin(115200);

  // If we want to receive packets, we must specify a packet handler function.
  // The packet handler is a custom function with a signature like the
  // onPacketReceived function below.
  myPacketSerial.setPacketHandler(&onPacketReceived);

  PITimer1.set_callback(updatex);
  PITimer1.period(0.001);
  PITimer1.start();
  PITimer2.set_callback(updatey);
  PITimer2.period(0.002);
  PITimer2.start();
}

void update_switches() {
	topright_bounce.update();
	topleft_bounce.update();
	bottomleft_bounce.update();
	bottomright_bounce.update();     
}


void step_top() {
	topright_bounce.update();
	topleft_bounce.update();
	if (stepper_top.dir < 0) {
		if (topright_bounce.read()==HIGH)  {
      stepper_top.step();
		}
	} else {
    if (topleft_bounce.read()==HIGH)  {
      stepper_top.step();
		}
	}
}

void step_bottom() {
	bottomright_bounce.update();
	bottomleft_bounce.update();
	if (stepper_bottom.dir > 0) {
		if (bottomright_bounce.read()==HIGH)  {
      stepper_bottom.step();
		}
	} else {
    if (bottomleft_bounce.read()==HIGH)  {
      stepper_bottom.step();
		}
	}
}


void home_motors() {
  bool top_finished = false;
  bool bottom_finished = false;
  bool finished = false;
  
  stepper_top.enableDriver();
  stepper_bottom.enableDriver();

  stepper_top.set_dir(true);
  stepper_bottom.set_dir(false);

  while ((!finished)) {
    
    if (!top_finished) {
      stepper_top.step();
    }
    if (!bottom_finished) {
      stepper_bottom.step();
    }
    
    update_switches();

    if (topright_bounce.read()==LOW)  {
      top_finished = true;
    } 
    if (topleft_bounce.read()==LOW)  {
      top_finished = true;
    } 
    if (bottomleft_bounce.read()==LOW)  {
      bottom_finished = true;
    } 
    if (bottomright_bounce.read()==LOW)  {
      bottom_finished = true;
    }  
    if (bottom_finished) {
      if (top_finished) {
        finished = true;
      }
    }
    delayMicroseconds(DELAYMU);
  }

  stepper_top.set_dir(false);
  stepper_bottom.set_dir(true);

  for (int i = 0; i<100; i++) {
    stepper_top.step();
    stepper_bottom.step();
    delayMicroseconds(DELAYMU);
  }
  
}


// This is our handler callback function.
// When an encoded packet is received and decoded, it will be delivered here.
// The `buffer` is a pointer to the decoded byte array. `size` is the number of
// bytes in the `buffer`.
void onPacketReceived(const uint8_t* buffer, size_t size)
{
  char command = *(buffer);
  uint8_t transmitBuffer[128];

  // 'b' -> fill buffer with data
  if (command == 'b') {
    dtData data;
    char axis = *(buffer + 1);
    char action = *(buffer + 2);
    byte size = *(buffer + 3); 
    byte offset = 3;
    long dt=0;

    if (axis == 'x') {
      for (byte c=0; c<size; c++) {
        for (byte i=0; i<4; i++)     {
          dt+= (*(buffer+offset+i))<<(i*8);
        }
        data.dt = dt;
        xsteps.push(data);
      }

    } else if (axis == 'y') {
      for (byte c=0; c<size; c++) {
        for (byte i=0; i<4; i++)     {
          dt+= (*(buffer+offset+i))<<(i*8);
        }
        data.dt = dt;
        ysteps.push(data);
      }
    }
  // 'h' -> home motors
  } else if (command == 'h') {
    home_motors();
    transmitBuffer[0] = 'h';
    transmitBuffer[1] = 'f';
    myPacketSerial.send(transmitBuffer, 2);
  // 'e' -> enable motors
  } else if (command == 'e') {
    stepper_top.enableDriver();
    stepper_bottom.enableDriver();
  // 'd' -> disable motors
  } else if (command == 'd') {
    stepper_top.disableDriver();
    stepper_bottom.disableDriver();
  // 'l' -> send buffer lengths
  } else if (command == 'l') {
    char axis = *(buffer + 1);
    long size;
    size = (long) xsteps.size();
    transmitBuffer[0] = 'l';
    transmitBuffer[1] = 'x';
    for (int i=0; i<4; i++) {
        transmitBuffer[i+2]=((size>>(i*8)) & 0xff);
    }
    size = (long) ysteps.size();
    transmitBuffer[6] = 'y';
    for (int i=0; i<4; i++) {
        transmitBuffer[i+7]=((size>>(i*8)) & 0xff);
    }
    myPacketSerial.send(transmitBuffer, 11);
  }
}


void loop() {
  update_switches();

  myPacketSerial.update();

  button_bounce.update();
  up_bounce.update();
  down_bounce.update();
  left_bounce.update();
  right_bounce.update();

  if (button_bounce.read()==HIGH) {
    home_motors();  
  } 
  if (up_bounce.read()==HIGH) {
    stepper_top.enableDriver();
    stepper_top.set_dir(true);
    step_top();
    delayMicroseconds(1000);
  } 
  if (down_bounce.read()==HIGH) {
    stepper_top.enableDriver();
    stepper_top.set_dir(false);
		step_top();
    delayMicroseconds(1000);
  }
  if (left_bounce.read()==HIGH) {
    stepper_bottom.enableDriver();
    stepper_bottom.set_dir(true);
		step_bottom();
    delayMicroseconds(1000);
  }
  if (right_bounce.read()==HIGH) {
    stepper_bottom.enableDriver();
    stepper_bottom.set_dir(false);
		step_bottom();
    delayMicroseconds(1000);
  }



}
