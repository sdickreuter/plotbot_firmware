
#include "Stepper.h"
//#include <PITimer.h>
#include <Bounce2.h>
#include <Cmd.h>

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

void forward_top(int arg_cnt, char **args) {
  Stream *s = cmdGetStream();
  int n = 0;
  if(arg_cnt > 0) {
      n = atoi(args[1]);
  }
  s->print("top, ");
  s->print(n);
  s->println(" steps forwards");
  stepper_top.set_dir(false);
  for (int i = 0; i<n; i++) {
    step_top();
    delayMicroseconds(DELAYMU);
  }
}

void backward_top(int arg_cnt, char **args) {
  Stream *s = cmdGetStream();
  int n = 0;
  if(arg_cnt > 0) {
      n = atoi(args[1]);
  }
  s->print("top, ");
  s->print(n);
  s->println(" steps backwards");
  stepper_top.set_dir(true);
  for (int i = 0; i<n; i++) {
    step_top();
    delayMicroseconds(DELAYMU);
  }
}

void forward_bottom(int arg_cnt, char **args) {
  Stream *s = cmdGetStream();
  int n = 0;
  if(arg_cnt > 0) {
      n = atoi(args[1]);
  }
  s->print("bottom, ");
  s->print(n);
  s->println(" steps forwards");
  stepper_bottom.set_dir(true);
  for (int i = 0; i<n; i++) {
    step_bottom();
    delayMicroseconds(DELAYMU);
  }
}

void backward_bottom(int arg_cnt, char **args) {
  Stream *s = cmdGetStream();
  int n = 0;
  if(arg_cnt > 0) {
      n = atoi(args[1]);
  }
  s->print("bottom, ");
  s->print(n);
  s->println(" steps backwards");
  stepper_bottom.set_dir(false);
  for (int i = 0; i<n; i++) {
    step_bottom();
    delayMicroseconds(DELAYMU);
  }
}

void enable(int arg_cnt, char **args)
{
  Stream *s = cmdGetStream();
  s->println("Enabling Drivers");
  stepper_top.enableDriver();
  stepper_bottom.enableDriver();
}

void disable(int arg_cnt, char **args)
{
  Stream *s = cmdGetStream();
  s->println("Disabling Drivers");
  stepper_top.disableDriver();
  stepper_bottom.disableDriver();
}

void home_cmd(int arg_cnt, char **args)
{
  Stream *s = cmdGetStream();
  s->println("Homing Steppers");
  home_motors();
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
  
    
  Serial.begin(57600);
  cmdInit(&Serial);
  cmdAdd("ftop",forward_top);
  cmdAdd("btop",backward_top);
  cmdAdd("fbot",forward_bottom);
  cmdAdd("bbot",backward_bottom);
  cmdAdd("enable",enable);
  cmdAdd("disable",disable);
  cmdAdd("home",home_cmd);

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


void loop() {
  update_switches();
  cmdPoll();


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
