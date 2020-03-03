/**
    Stepper.h
	Contains class for controlling a A4983 stepper driver

    @author sei
*/

#ifndef Stepper_h
#define Stepper_h

#include <Arduino.h>

class Stepper {
  public:
	Stepper(int RES,int DIR,int STEP,int MS1,int MS2,int MS3,int ENABLE);

	void setMicrostepping(int MODE);

	void enableDriver();
	void disableDriver(); 

	void set_dir(bool dir);
	  
	void step();
	
	int get_pos();
	int dir;

  private:
    int DIR_pin;
    int STEP_pin;
    int MS1_pin;
    int MS2_pin;
    int MS3_pin;
    int ENBL_pin;
	int STEPPING_FACTOR;
	volatile int position;
};


#endif
