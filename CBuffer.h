/**
	CBuffer.h
	Simple circular buffer

	@author sei
*/

#ifndef CBuffer_h
#define CBuffer_h

#define BUFFER_SIZE 3000


// struct for storing step timings
struct dtData {
  float dt;
  char action;
};

// simple circular buffer
struct CBuffer {
  	int end_index = BUFFER_SIZE-1;
  	dtData data[BUFFER_SIZE];
	int head = -1;
	int tail = 0;
};

void reset_cb(CBuffer* buf) {
	buf->head = -1;
	buf->tail = 0;
}

void push_cb(CBuffer* buf, dtData val) {
  buf->head += 1;
  if (buf->head > buf->end_index) {
    buf->head = 0;
  }
  buf->data[buf->head] = val;
}

dtData pop_cb(CBuffer* buf) {
  dtData val = buf->data[buf->tail];
  buf->tail += 1;
  if (buf->tail > buf->end_index) {
    buf->tail = 0;
  }
  return val;
}

int length_cb(CBuffer* buf) {
	int len = 0;
  if (buf->head > -1) {
	  len = 1 + buf->head - buf->tail;
	  if (len < 0) {
	    len = (buf->head+buf->end_index+2) - buf->tail;
	  }
  }
  return len;
}


#endif