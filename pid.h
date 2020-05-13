#define KP 1.0
#define KI 0.0 //0.00001
#define KD 0.0 //0.0001

// struct for storing step timings
struct PID {
  float prev_err = 0.0;
  float sum = 0.0;
  float diff = 0.0;  
};


void init_pid(PID pid) {
	pid.prev_err = 0.0;
	pid.sum = 0.0;
	pid.diff = 0.0;
}


float calc_pid(PID pid, float err) {
	pid.diff = err - pid.prev_err;
	pid.prev_err = err;
	pid.sum += err;
	return KP*err + KI*pid.sum + KD*pid.diff;
}