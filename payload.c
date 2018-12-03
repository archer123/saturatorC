#include <assert.h>

#include "payload.h"
#include "stdlib.h"


char* str_payload(Payload *_this, const size_t len){
  assert(len >= sizeof( _this ) );
  char* padding = (char*) malloc((len - sizeof(_this)) *sizeof(char) ) ;
  memset(padding, len-sizeof(_this), 0);
  char* str = (char*) malloc(len *sizeof(char));
  str = ((char*) _this);
  strcat(str, padding);
  return str;
}

bool equal_payload(Payload *_this, Payload other){
  return (_this ->sequence_number == other.sequence_number
	  && _this ->sent_timestamp == other.sent_timestamp
	  && _this ->recv_timestamp == other.recv_timestamp
	  && _this ->sender_id == other.sender_id);
}


char* str_satpayload(SatPayload *_this, const size_t len){
  assert(len >= sizeof(_this ) );
  char* padding = (char*) malloc((len - sizeof(_this)) *sizeof(char) ) ;
  memset(padding, len-sizeof(_this), 0);
  char* str = (char*) malloc(len *sizeof(char));
  str = ((char*) _this);
  strcat(str, padding);
  return str;
}

bool equal_satpayload(SatPayload *_this, SatPayload other )
{
  return (_this ->sequence_number == other.sequence_number
	  && _this ->ack_number == other.ack_number
	  && _this ->sent_timestamp == other.sent_timestamp
	  && _this ->recv_timestamp == other.recv_timestamp
	  && _this ->sender_id == other.sender_id);
}
