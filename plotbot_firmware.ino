
#include "Stepper.h"
#include <Bounce2.h>
#include <PITimer.h>
#include <PacketSerial.h>
#include <PWMServo.h>
#include "defines.h"

#define CIRCULAR_BUFFER_INT_SAFE
#include "CircularBuffer.h"

// 1500 * 4 * 64 bit = 48 kbytes, Teensy 3.2 has 64 kbytes of RAM
// 3000 * 2* ( 32 bit + 8 bit) = 30 kbytes
CircularBuffer<dtData, BUFFER_SIZE> asteps;
CircularBuffer<dtData, BUFFER_SIZE> bsteps;

PacketSerial_<COBS, 0, MAX_BUFFER_ELEMENTS*sizeof(dtData)> myPacketSerial;

PWMServo penservo;  // servo object to control up/down of the Pen

// Bounce objects for endswitches
Bounce topright_bounce = Bounce(); 
Bounce topleft_bounce = Bounce(); 
Bounce bottomleft_bounce = Bounce(); 
Bounce bottomright_bounce = Bounce(); 


elapsedMicros mu;
elapsedMicros pressed;

//Stepper::Stepper(int ENABLE,int MS1,int MS2,int SPREAD,int STEP,int DIR)
Stepper stepper_bottom(TOP_ENABLE, TOP_MS1, TOP_MS2, TOP_SPRD, TOP_STEP, TOP_DIR,true);
Stepper stepper_top(BOTTOM_ENABLE, BOTTOM_MS1, BOTTOM_MS2, BOTTOM_SPRD, BOTTOM_STEP, BOTTOM_DIR,false);


// variables for storing objects from buffers
dtData dta;
dtData dtb;

// variable for state of motors, if true buffers will be checked for content regularly, and if they
// have timings in them the steps will be performed
bool moving = false;


// do a step but check state of endswitches first
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


// do a step but check state of endswitches first
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


void updatea() {
	if (moving) {
	  if (!asteps.isEmpty()) {
	    dta = asteps.shift();
	    if (dta.dt < 0.0) {
	        PITimer1.period(-1.0*dta.dt);
	        stepper_top.set_dir(false);   
	    }
	    else {
	        PITimer1.period(dta.dt);
	        stepper_top.set_dir(true);
	    }
	    //stepper_top.step(); // unsafe stepping
	    step_top(); // save stepping, checks the endswitches
	    if (dta.action == PEN_UP) {
    		penservo.write(POS_UP);
	    } else if (dta.action == PEN_DOWN) {
    	    		penservo.write(POS_DOWN);
	    } 

	  } else {
	  		PITimer1.period(0.01);
		}
	} else {		
	  // stop timer if buffer is empty and moving is false
    PITimer1.stop();
    PITimer1.reset();
  }
}

void updateb() {
	if (moving) {
	  if (!bsteps.isEmpty()) {
	    dtb = bsteps.shift();
	    if (dtb.dt < 0.0) {
	        PITimer2.period(-1.0*dtb.dt);
	        stepper_bottom.set_dir(false);   
	    }
	    else {
	        PITimer2.period(dtb.dt);
	        stepper_bottom.set_dir(true);
	    }
	    //stepper_bottom.step(); // unsafe stepping
	    step_bottom(); // save stepping, checks the endswitches
	    if (dta.action == PEN_UP) {
    		penservo.write(POS_UP);
	    } else if (dta.action == PEN_DOWN) {
    	    		penservo.write(POS_DOWN);
	    } 
	  } else {
	  	PITimer2.period(0.01);
  	}
	} else {		
	  // stop timer if buffer is empty and moving is false
    PITimer2.stop();
    PITimer2.reset();
  }
}


void setup() {
  penservo.attach(SERVO_PWM);
  penservo.write(40);

  pinMode(TOPRIGHT_SWITCH, INPUT);
  pinMode(TOPLEFT_SWITCH, INPUT);
  pinMode(BOTTOMLEFT_SWITCH, INPUT);
  pinMode(BOTTOMRIGHT_SWITCH, INPUT);

  topright_bounce.attach(TOPRIGHT_SWITCH);
  topleft_bounce.attach(TOPLEFT_SWITCH);
  bottomleft_bounce.attach(BOTTOMLEFT_SWITCH);
  bottomright_bounce.attach(BOTTOMRIGHT_SWITCH);

  topright_bounce.interval(1); // interval in ms
  topleft_bounce.interval(1); // interval in ms
  bottomleft_bounce.interval(1); // interval in ms
  bottomright_bounce.interval(1); // interval in ms
    
  // We begin communication with our PacketSerial object by setting the
  // communication speed in bits / second (baud).
  myPacketSerial.begin(115200);

  // If we want to receive packets, we must specify a packet handler function.
  // The packet handler is a custom function with a signature like the
  // onPacketReceived function below.
  myPacketSerial.setPacketHandler(&onPacketReceived);

  PITimer1.set_callback(updatea);
  PITimer2.set_callback(updateb);
}

void update_switches() {
	// update all bounce objects
  topright_bounce.update();
  topleft_bounce.update();
  bottomleft_bounce.update();
  bottomright_bounce.update();     
}


void home_motors(bool reverse) {
  // init variables that terminate the while loop
  bool top_finished = false;
  bool bottom_finished = false;
  bool finished = false;
  
  stepper_top.enableDriver();
  stepper_bottom.enableDriver();

  if (!reverse) {
	  // set both steppers so they move to the left
  	stepper_top.set_dir(false);
  	stepper_bottom.set_dir(false);  	
  } else {
	  // set both steppers so they move to the right
  	stepper_top.set_dir(true);
  	stepper_bottom.set_dir(true);  	  	
  }

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

	if (!reverse) {
	  // set both steppers so they move to the right
	  stepper_top.set_dir(true);
	  stepper_bottom.set_dir(true);
  } else {
		// set both steppers so they move to the left
		stepper_top.set_dir(false);
		stepper_bottom.set_dir(false); 	
  }

  // make some steps away from the endswitches
  for (int i = 0; i<128; i++) {
    stepper_top.step();
    stepper_bottom.step();
    delayMicroseconds(DELAYMU);
  }
  
}


void jog(char axis, long steps) {
  
	// set stepper direction
  if (steps < 0.0) {
  		if (axis == 'a') {
      	stepper_top.set_dir(false);   
  		} else {
      	stepper_bottom.set_dir(false);   
      }
      steps *= -1;
  } else {
  		if (axis == 'a') {
      	stepper_top.set_dir(true);   
  		} else {
      	stepper_bottom.set_dir(true);   
      }
  }

  // do steps
	if (axis == 'a') {
	  for (int i = 0; i<steps; i++) {
	  	update_switches();
	    step_top();
	    delayMicroseconds(DELAYMU);
 		}
	} else {
	  for (int i = 0; i<steps; i++) {
	  	update_switches();
	    step_bottom();
	    delayMicroseconds(DELAYMU);
 		}
  }

}


// Helper for converting float to bytes and vice versa
union union_float {
   byte b[4];
   float f;
};

// Helper for converting long to bytes and vice versa
union union_long {
   byte b[4];
   long l;
};


//uint8_t transmitBuffer[512*4];
uint8_t transmitBuffer[128];

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
    long offset = 1;
    char axis = *(buffer + offset);
    offset+=1;
    union_long size; 
    for (byte i=0; i<4; i++)     {
      size.b[i] = *(buffer+offset+i);
    }
    offset+=4;

    union_float dt;

    if (axis == 'a') {
    	//if ( size.l < (BUFFER_SIZE-length_cb(&asteps)) ) {
    	if ( size.l < 2000 ) {
	      for (long c=0; c<size.l; c++) {
        	for (byte i=0; i<4; i++) {
         		dt.b[i] = *(buffer+offset+i);
	        }
	        data.dt = dt.f;
	        data.action = *(buffer+offset+4);
	        asteps.push(data);
	        offset += 5;
	      }
  	    transmitBuffer[0] = 'o';
	    	transmitBuffer[1] = 'k';
  	  	myPacketSerial.send(transmitBuffer, 2);
    	}
    } else if (axis == 'b') {
    	//if ( size.l < (BUFFER_SIZE-length_cb(&bsteps)) ) {
    	if ( size.l < 2000 ) {
	      for (long c=0; c<size.l; c++) {
	        for (byte i=0; i<4; i++) {
	          dt.b[i] = *(buffer+offset+i);
	        }
	        data.dt = dt.f;
	        data.action = *(buffer+offset+4);
	        bsteps.push(data);
	        offset += 5;
	      }
  	    transmitBuffer[0] = 'o';
	    	transmitBuffer[1] = 'k';
  	  	myPacketSerial.send(transmitBuffer, 2);
	    }
    } 

  // 'j' -> jog motor
  } else if (command == 'j') {
	 	long offset = 1;
    char axis = *(buffer + offset);
    offset+=1;
    union_long steps; 
    for (byte i=0; i<4; i++)     {
      steps.b[i] = *(buffer+offset+i);
    }

    jog(axis,steps.l);

    transmitBuffer[0] = 'o';
    transmitBuffer[1] = 'k';
    myPacketSerial.send(transmitBuffer, 2);

  // 's' -> set pen-servo
  } else if (command == 's') {
	 	long offset = 1;
    byte pos = *(buffer + offset);
    penservo.write( (int) pos);

  // 'h' -> home motors
  } else if (command == 'h') {
    home_motors(false);
    transmitBuffer[0] = 'o';
    transmitBuffer[1] = 'k';
    myPacketSerial.send(transmitBuffer, 2);

  // 'r' -> home motors in reverse direction
  } else if (command == 'r') {
    home_motors(true);
    transmitBuffer[0] = 'o';
    transmitBuffer[1] = 'k';
    myPacketSerial.send(transmitBuffer, 2);

  // 'z' -> zero motor positions
  } else if (command == 'z') {
    stepper_top.zero();
    stepper_bottom.zero();

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
    long size;
    size = (long) asteps.size();
    transmitBuffer[0] = 'l';
    transmitBuffer[1] = 'a';
    for (int i=0; i<4; i++) {
        transmitBuffer[i+2]=((size>>(i*8)) & 0xff);
    }
    size = (long) bsteps.size();
    transmitBuffer[6] = 'b';
    for (int i=0; i<4; i++) {
        transmitBuffer[i+7]=((size>>(i*8)) & 0xff);
    }
    myPacketSerial.send(transmitBuffer, 11);
  
  // 'm' -> start moving
  } else if (command == 'm') {
  	if (!moving) {
  		moving = true;
	  	PITimer1.reset();
	  	PITimer1.period(0.1);
	  	PITimer2.reset();
	  	PITimer2.period(0.1);
	  	PITimer2.start(); 
	  	PITimer1.start();		
  	}
    // if (length_cb(&asteps)>0) {
    //   if (!PITimer1.running()) {
    // 		//dtx = ysteps.pop();
	 		// 	//PITimer1.period(dtx.dt);
	   //    PITimer1.reset();
	   //    PITimer1.period(0.01);
		  // 	PITimer1.start();
    //   }
    // }
    // if (length_cb(&bsteps)>0) {
    //   if (!PITimer2.running()) {
	   //    //dty = xsteps.pop();
	   //    //PITimer2.period(dty.dt);
	   //    PITimer2.reset();
	   //    PITimer2.period(0.01);
	   //    PITimer2.start();   
    //   }
    // }

  // // 's' -> start timers and get ready to
  // } else if (command == 's') {
  //   byte offset = 1;
  //   char axis = *(buffer + offset);  
  
  // 'c' -> clear buffers
  } else if (command == 'c') {
  	moving = false;
    PITimer1.stop();
		PITimer2.stop();
		PITimer1.reset();
		PITimer2.reset();
    asteps.clear();
    bsteps.clear();


  // 'p' -> send stepper positions
  } else if (command == 'p') {
    union_long pos;
    int offset = 0;
    transmitBuffer[offset] = 'p';
    offset+=1; 
    transmitBuffer[offset] = 'a';
    offset+=1;    
		pos.l = stepper_top.get_pos();
		transmitBuffer[offset+0] = pos.b[0];
		transmitBuffer[offset+1] = pos.b[1];
		transmitBuffer[offset+2] = pos.b[2];
		transmitBuffer[offset+3] = pos.b[3];
		offset += 4;
    
    transmitBuffer[offset] = 'b';
    offset+=1;    
		pos.l = stepper_bottom.get_pos();
		transmitBuffer[offset+0] = pos.b[0];
		transmitBuffer[offset+1] = pos.b[1];
		transmitBuffer[offset+2] = pos.b[2];
		transmitBuffer[offset+3] = pos.b[3];
		offset += 4;
    
    myPacketSerial.send(transmitBuffer, offset);

  // 'i' -> send infos
  } else if (command == 'i') {
    long max_buffer_size = (long) MAX_BUFFER_ELEMENTS*sizeof(dtData);
    transmitBuffer[0] = 'm';
    transmitBuffer[1] = 'a';
    transmitBuffer[2] = 'x';
    transmitBuffer[3] = 'b';
    transmitBuffer[4] = 'u';
    transmitBuffer[5] = 'f';
    long offset = 6;
    for (int i=0; i<4; i++) {
        transmitBuffer[i+offset]=((max_buffer_size>>(i*8)) & 0xff);
    }
    offset += 4;

    long max_steps = (long) MAX_STEPS;
    transmitBuffer[offset+0] = 'm';
    transmitBuffer[offset+1] = 'a';
    transmitBuffer[offset+2] = 'x';
    transmitBuffer[offset+3] = 's';
    transmitBuffer[offset+4] = 't';
    transmitBuffer[offset+5] = 'p';
    offset += 6;
    for (int i=0; i<4; i++) {
        transmitBuffer[i+offset]=((max_steps>>(i*8)) & 0xff);
    }
    offset += 4;


    myPacketSerial.send(transmitBuffer, offset);
  }
}


void loop() {
  
  update_switches();

  myPacketSerial.update();

}
