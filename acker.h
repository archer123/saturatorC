#ifndef ACKER_H
#define ACKER_H
#include <stdio.h>
#include "socket.h"
#include "saturateservo.h"


typedef struct Acker Acker;
struct Acker{
  char* _name;
  FILE* _log_file;
  Socket _listen;
  Socket _send;
  struct Address _remote;
  bool _server;
  int _ack_id;
  struct SaturateServo *_saturatr;

  uint64_t _next_ping_time;

  int _foreign_id;
};

static const int _ping_interval = 1000000000;


struct Acker IAcker1( struct Acker *_this,
   char * s_name,
  FILE* log_file_handle,
  Socket  s_listen,
  Socket  s_send,
  Address  s_remote,
  bool s_server,
  int s_ack_id );
void Ack_recv(struct Acker *_this);
void Ack_tick(struct Acker *_this);

void Ack_set_remote(struct Acker *_this, const Address  s_remote );

void Ack_set_saturatr(struct Acker *_this, struct SaturateServo * const s_saturatr );

uint64_t Ack_wait_time(struct Acker *_this);

// void IAcker2(struct Acker *_this, const Acker );
// const Acker Ack_equal(struct Acker *_this, const Acker);

#endif
