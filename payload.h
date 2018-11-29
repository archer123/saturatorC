#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct Payload Payload;
struct Payload{
  uint32_t sequence_number;
  uint64_t sent_timestamp, recv_timestamp;
  int sender_id;

  const char* (*str_payload) (struct Payload *_this, const size_t len);
  bool (*equal_payload) (struct Payload *_this, Payload);
};


typedef struct SatPayload SatPayload;
struct SatPayload{
  int32_t sequence_number, ack_number;
  uint64_t sent_timestamp, recv_timestamp;
  int sender_id;

  const char* (*str_satpayload) (struct SatPayload *_this, const size_t len);
  bool (*equal_satpayload) (struct SatPayload*_this, SatPayload);
};



#endif
