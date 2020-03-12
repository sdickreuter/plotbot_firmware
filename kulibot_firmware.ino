
#include "Stepper.h"
#include "CBuffer.h"
#include <Bounce2.h>
#include <PITimer.h>
#include <PacketSerial.h>

//SerialTransfer transferbuffer;
PacketSerial_<COBS, 0, 2048> myPacketSerial;

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

CBuffer xsteps;
CBuffer ysteps;

dtData dtx;
dtData dty;


void step_top() {
  if (stepper_top.dir < 0) {
    //topright_bounce.update();
    if (topright_bounce.read()==HIGH)  {
      stepper_top.step();
    }
  } else {
    //topleft_bounce.update();
    if (topleft_bounce.read()==HIGH)  {
      stepper_top.step();
    }
  }
}

void step_bottom() {
  if (stepper_bottom.dir > 0) {
    //bottomright_bounce.update();
    if (bottomright_bounce.read()==HIGH)  {
      stepper_bottom.step();
    }
  } else {
    //bottomleft_bounce.update();
    if (bottomleft_bounce.read()==HIGH)  {
      stepper_bottom.step();
    }
  }
}

void forward_top(int n) {
  stepper_top.set_dir(false);
  for (int i = 0; i<n; i++) {
    step_top();
    delayMicroseconds(DELAYMU);
  }
}

void backward_top(int n) {
  stepper_top.set_dir(true);
  for (int i = 0; i<n; i++) {
    step_top();
    delayMicroseconds(DELAYMU);
  }
}

void forward_bottom(int n) {
  stepper_bottom.set_dir(true);
  for (int i = 0; i<n; i++) {
    step_bottom();
    delayMicroseconds(DELAYMU);
  }
}

void backward_bottom(int n) {
  stepper_bottom.set_dir(false);
  for (int i = 0; i<n; i++) {
    step_bottom();
    delayMicroseconds(DELAYMU);
  }
}


void updatex() {
  if (length_cb(&xsteps) > 0) {
    dtx = pop_cb(&xsteps);
    if (dtx.dt < 0.0) {
        PITimer1.period(-1.0*dtx.dt);
        stepper_top.set_dir(true);   
    }
    else {
        PITimer1.period(dtx.dt);
        stepper_top.set_dir(false);
    }
    stepper_top.step();
    //step_top();
  } else {
    //PITimer1.stop();
  }
}


void updatey() {
  if (length_cb(&xsteps) > 0) {
    dty = pop_cb(&ysteps);
    if (dty.dt < 0.0) {
        PITimer2.period(-1.0*dty.dt);
        stepper_bottom.set_dir(false);   
    }
    else {
        PITimer2.period(dty.dt);
        stepper_bottom.set_dir(true);
    }
    stepper_bottom.step();
    //step_bottom();
  } else {
    //PITimer2.stop();
  }
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
  //PITimer1.period(0.001);
  //PITimer1.start();
  PITimer2.set_callback(updatey);
  //PITimer2.period(0.002);
  //PITimer2.start();
}

void update_switches() {
	topright_bounce.update();
	topleft_bounce.update();
	bottomleft_bounce.update();
	bottomright_bounce.update();     
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

  for (int i = 0; i<20000; i++) {
    stepper_top.step();
    stepper_bottom.step();
    delayMicroseconds(DELAYMU);
  }
  
}


union union_float {
   byte b[4];
   float f;
} u;


uint8_t transmitBuffer[1024];

// This is our handler callback function.
// When an encoded packet is received and decoded, it will be delivered here.
// The `buffer` is a pointer to the decoded byte array. `size` is the number of
// bytes in the `buffer`.
void onPacketReceived(const uint8_t* buffer, size_t size)
{
  char command = *(buffer);

  // 'b' -> fill buffer with data
  if (command == 'b') {
    dtData data;
    byte offset = 1;
    char axis = *(buffer + offset);
    offset+=1;
    char action = *(buffer + offset);
    offset+=1;
    uint16_t size = 0; 
    for (byte i=0; i<2; i++)     {
      size+= (*(buffer+offset+i))<<(i*8);
    }
    offset+=2;

    union_float dt;

    if (axis == 'x') {
      for (uint16_t c=0; c<size; c++) {
        for (byte i=0; i<4; i++)     {
          dt.b[i] = *(buffer+offset+i);
        }
        data.dt = dt.f;
        push_cb(&xsteps,data);
        offset += 4;
      }
    } else if (axis == 'y') {
      for (long c=0; c<size; c++) {
        for (byte i=0; i<4; i++)     {
          dt.b[i] = *(buffer+offset+i);
        }
        data.dt = dt.f;
        push_cb(&ysteps,data);
        offset += 4;
      }
    } 

    // char fchar[8]; // Buffer big enough for 7-character float
    // dtostrf(absolute(data.dt), 6, 2, fchar); // Leave room for too large numbers!
    // for (byte i=0; i<8; i++) {
    //   transmitBuffer[i] = fchar[i];
    // }
    // myPacketSerial.send(transmitBuffer, 8);

  // 'h' -> home motors
  } else if (command == 'h') {
    home_motors();
    transmitBuffer[0] = 'o';
    transmitBuffer[1] = 'k';
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
    size = (long) length_cb(&xsteps);
    transmitBuffer[0] = 'l';
    transmitBuffer[1] = 'x';
    for (int i=0; i<4; i++) {
        transmitBuffer[i+2]=((size>>(i*8)) & 0xff);
    }
    size = (long) length_cb(&ysteps);
    transmitBuffer[6] = 'y';
    for (int i=0; i<4; i++) {
        transmitBuffer[i+7]=((size>>(i*8)) & 0xff);
    }
    myPacketSerial.send(transmitBuffer, 11);
  
  // 'm' -> start moving
  } else if (command == 'm') {
    //dtData x;
    //x = ysteps.pop();
    //PITimer1.period(x.dt);
    PITimer1.period(0.01);
    //dtData y;
    //y = ysteps.pop();
    //PITimer2.period(y.dt);
    PITimer2.period(0.01);

    PITimer1.start();
    PITimer2.start();   

  // 's' -> make steps
  } else if (command == 's') {
    byte offset = 1;
    char axis = *(buffer + offset);  
  
  // 'c' -> clear buffers
  } else if (command == 'c') {
    xsteps.head=0;
    xsteps.tail=0;
    ysteps.head=0;
    ysteps.tail=0;

  // 'r' -> read back x buffer, last 1024 bits
  } else if (command == 'r') {
    dtData data;
    union_float dt; 
    int offset = 0;
    while (offset < 1020) {
      data = pop_cb(&xsteps);
      dt.f = data.dt;
      transmitBuffer[offset+0] = dt.b[0];
      transmitBuffer[offset+1] = dt.b[1];
      transmitBuffer[offset+2] = dt.b[2];
      transmitBuffer[offset+3] = dt.b[3];
      offset += 4;      
    }
    myPacketSerial.send(transmitBuffer, offset);
  }

}


void loop() {
  update_switches();

  myPacketSerial.update();

  //delayMicroseconds(50);

  // button_bounce.update();
  // up_bounce.update();
  // down_bounce.update();
  // left_bounce.update();
  // right_bounce.update();

  // if (button_bounce.read()==HIGH) {
  //   home_motors();  
  // } 
  // if (up_bounce.read()==HIGH) {
  //   stepper_top.enableDriver();
  //   stepper_top.set_dir(true);
  //   step_top();
  //   delayMicroseconds(1000);
  // } 
  // if (down_bounce.read()==HIGH) {
  //   stepper_top.enableDriver();
  //   stepper_top.set_dir(false);
		// step_top();
  //   delayMicroseconds(1000);
  // }
  // if (left_bounce.read()==HIGH) {
  //   stepper_bottom.enableDriver();
  //   stepper_bottom.set_dir(true);
		// step_bottom();
  //   delayMicroseconds(1000);
  // }
  // if (right_bounce.read()==HIGH) {
  //   stepper_bottom.enableDriver();
  //   stepper_bottom.set_dir(false);
		// step_bottom();
  //   delayMicroseconds(1000);
  // }



}
