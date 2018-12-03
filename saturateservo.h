#ifndef SATURATESERVO_H
#define SATURATESERVO_H

#include <stdio.h>
#include "socket.h"
#include "acker.h"


typedef struct SaturateServo SaturateServo;

struct SaturateServo
{

  char* _name;
  FILE* _log_file;

  Socket _listen;
  Socket _send;
  Address _remote;

  bool _server;
  int _send_id;

  struct Acker *_acker;

  uint64_t _next_transmission_time;

  int _foreign_id;

  int _packets_sent, _max_ack_id;

  int _window;



};

static const int _transmission_interval = 1000 * 1000 * 1000;

static const int LOWER_WINDOW = 20;
static const int UPPER_WINDOW = 1500;

static const double LOWER_RTT = 0.75;
static const double UPPER_RTT = 3.0;

void ISaturateServo_1(struct SaturateServo *_this,
  char* s_name,
  FILE* log_file,
  const Socket s_listen,
  const Socket s_send,
  const Address s_remote,
  const bool s_server,
  const int s_send_id);

void SatServ_recv(struct SaturateServo *_this);
uint64_t SatServ_wait_time(struct SaturateServo *_this);
void SatServ_tick(struct SaturateServo *_this);
void SatServ_set_acker(struct SaturateServo *_this, struct Acker * const s_acker);
void SatServ_set_remote(struct SaturateServo *_this, const Address s_remote);

void ISaturateServo_2(struct SaturateServo *_this, const SaturateServo );
const SaturateServo SatServ_equ(struct SaturateServo *_this, const SaturateServo);

#endif
