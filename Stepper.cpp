/**
    Stepper.cpp
	Contains class for controlling a A4983 stepper driver

    @author sei
*/


#include "Stepper.h"


Stepper::Stepper(int RES,int DIR,int STEP,int MS1,int MS2,int MS3,int ENABLE) {
	this->DIR_pin = DIR;
	this->STEP_pin = STEP;
	this->MS1_pin = MS1;
	this->MS2_pin = MS2;
	this->MS3_pin = MS3;
	this->ENBL_pin = ENABLE;

	pinMode(this->DIR_pin, OUTPUT);
	pinMode(this->STEP_pin, OUTPUT);
	pinMode(this->MS1_pin, OUTPUT);
	pinMode(this->MS2_pin, OUTPUT);
	pinMode(this->MS3_pin, OUTPUT);
	pinMode(this->ENBL_pin, OUTPUT);

	disableDriver();
	setMicrostepping(1);
	position = 500;
	dir = 1;
}

void Stepper::enableDriver(){
  digitalWrite(this->ENBL_pin, LOW);
}

void Stepper::disableDriver(){
  digitalWrite(this->ENBL_pin, HIGH);
}

void Stepper::setMicrostepping(int MODE){
 
  switch (MODE) {
    case 0:  					// 1/1  microstep
      digitalWrite(this->MS1_pin, LOW);
      digitalWrite(this->MS2_pin, LOW);
      digitalWrite(this->MS3_pin, LOW);
      this->STEPPING_FACTOR = 1;
      break;
    case 1:					// 1/2  microstep
      digitalWrite(this->MS1_pin, HIGH);
      digitalWrite(this->MS2_pin, LOW);
      digitalWrite(this->MS3_pin, LOW);
      this->STEPPING_FACTOR = 2;
      break;
    case 2:					// 1/4  microstep
      digitalWrite(this->MS1_pin, LOW);
      digitalWrite(this->MS2_pin, HIGH);
      digitalWrite(this->MS3_pin, LOW);
      this->STEPPING_FACTOR = 4;
      break;
    case 3:					// 1/8  microstep
      digitalWrite(this->MS1_pin, HIGH);
      digitalWrite(this->MS2_pin, HIGH);
      digitalWrite(this->MS3_pin, LOW);
      this->STEPPING_FACTOR = 8;
      break;
    case 4:					// 1/16 microstep
      digitalWrite(this->MS1_pin, HIGH);
      digitalWrite(this->MS2_pin, HIGH);
      digitalWrite(this->MS3_pin, HIGH);
      this->STEPPING_FACTOR = 16;
      break;
    default: 					// Full Step default setting
      digitalWrite(this->MS1_pin, LOW);
      digitalWrite(this->MS2_pin, LOW);
      digitalWrite(this->MS3_pin, LOW);
      this->STEPPING_FACTOR = 16;  }
}


int Stepper::get_pos() {
	return position;
}

void Stepper::set_dir(bool dir) {
  if (dir) {
	this->dir = 1;
    digitalWrite(this->DIR_pin, HIGH);
  } else {
	this->dir = -1;
    digitalWrite(this->DIR_pin, LOW);
  }
}

void Stepper::step() {
	digitalWrite(this->STEP_pin, HIGH);
	delayMicroseconds(2);
	digitalWrite(this->STEP_pin, LOW);
	delayMicroseconds(2);
	position++;
}
