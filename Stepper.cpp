/**
    Stepper.cpp
	Contains class for controlling a TMC2209 SilentStepStick

    @author sei
*/

#include "Stepper.h"


Stepper::Stepper(int ENABLE,int MS1,int MS2,int SPREAD,int STEP,int DIR) {
	this->ENBL_pin = ENABLE;
	this->MS1_pin = MS1;
	this->MS2_pin = MS2;
	this->SPREAD_pin = SPREAD;
	this->STEP_pin = STEP;
	this->DIR_pin = DIR;

	pinMode(this->ENBL_pin, OUTPUT);
	pinMode(this->MS1_pin, OUTPUT);
	pinMode(this->MS2_pin, OUTPUT);
	pinMode(this->SPREAD_pin, OUTPUT);
	pinMode(this->STEP_pin, OUTPUT);
	pinMode(this->DIR_pin, OUTPUT);
 
	disableDriver();

	setMicrostepping(0);

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
    case 0:  					// 1/8  microstep stealthChop
      digitalWrite(this->MS1_pin, LOW);
      digitalWrite(this->MS2_pin, LOW);
      digitalWrite(this->SPREAD_pin, LOW);
      this->stepping_factor = 8;
      break;
    case 1:					// 1/32  microstep stealthChop
      digitalWrite(this->MS1_pin, HIGH);
      digitalWrite(this->MS2_pin, LOW);
      digitalWrite(this->SPREAD_pin, LOW);
      this->stepping_factor = 32;
      break;
    case 2:					// 1/64  microstep stealthChop
      digitalWrite(this->MS1_pin, HIGH);
      digitalWrite(this->MS2_pin, LOW);
      digitalWrite(this->SPREAD_pin, LOW);
      this->stepping_factor = 64;
      break;
  }
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
