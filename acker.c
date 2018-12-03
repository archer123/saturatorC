#include <assert.h>

#include "acker.h"
#include "payload.h"
#include "saturateservo.h"
#include "socket.h"


struct Acker IAcker1( struct Acker *_this,
 char * s_name,
 FILE* log_file_handle,
 Socket  s_listen,
 Socket  s_send,
 Address  s_remote,
 bool s_server,
 int s_ack_id ){
   _this-> _name = s_name;
   _this-> _log_file = log_file_handle;
   _this-> _listen = s_listen;
   _this-> _send = s_send;
   _this-> _remote = s_remote;
   _this-> _server = s_server;
   _this-> _ack_id = s_ack_id;
   _this-> _saturatr = NULL;
   _this-> _next_ping_time = Sock_timestamp();
   _this-> _foreign_id = -1;
 }
void Ack_recv(struct Acker *_this){
  /* get the data packet */
  struct Packet incoming = Sock_recv( &(_this->_listen));
  SatPayload *contents = (SatPayload *) incoming.payload;
  contents->recv_timestamp = incoming.timestamp;

  int64_t oneway_ns = contents->recv_timestamp - contents->sent_timestamp;
  double oneway = oneway_ns / 1.e9;

  Address UNKNOWN;
  IAddress_1(&UNKNOWN, "0.0.0.0", 0);

  if ( _this->_server ) {
    if ( _this->_saturatr ) {
      if ( contents->sender_id > _this->_foreign_id ) {
	       _this->_foreign_id = contents->sender_id;
	       SatServ_set_remote(_this->_saturatr, incoming.addr );
      }
    }
    if (Addr_equal( &(_this->_remote), UNKNOWN )) {
      return;
    }
  }

  assert( !Addr_equal( &(_this->_remote), UNKNOWN ) );

  struct Address fb_destination = _this->_remote;

  /* send ack */
  SatPayload outgoing = *contents ;
  outgoing.sequence_number = -1;
  outgoing.ack_number = contents->sequence_number;
  Packet send_pac;
  IPacket_1(&send_pac, _this->_remote, str_satpayload(&outgoing, sizeof( SatPayload ) ) );
  Sock_send(&(_this->_send),send_pac );

   fprintf( _this->_log_file,"%s DATA RECEIVED senderid=%d, seq=%d, send_time=%ld, recv_time=%ld, 1delay=%.4f \n",
      _this->_name,  _this->_server ? contents->sender_id : _this->_ack_id, contents->sequence_number, contents->sent_timestamp, contents->recv_timestamp,oneway );
}


void Ack_set_remote(struct Acker *_this, const Address  s_remote ) {_this-> _remote = s_remote; }
void Ack_set_saturatr(struct Acker *_this, struct SaturateServo * const s_saturatr ) { _this -> _saturatr = s_saturatr; }

void Ack_tick(struct Acker *_this){
  if(_this->_server){
    return;
  }

  Address UNKNOWN;
  IAddress_1(&UNKNOWN, "0.0.0.0", 0);

  /* send NAT heartbeats */
  if ( Addr_equal(&(_this->_remote), UNKNOWN) ) {
    _this->_next_ping_time = Sock_timestamp() + _ping_interval;
    return;
  }

  if(_this->_next_ping_time < Sock_timestamp()){
    SatPayload contents;
    contents.sequence_number = -1;
    contents.ack_number = -1;
    contents.sent_timestamp = Sock_timestamp();
    contents.recv_timestamp = 0;
    contents.sender_id = _this->_ack_id;

    Packet send_pac;
    IPacket_1(&send_pac, _this->_remote, str_satpayload(&contents, sizeof( SatPayload ) ) ) ;
    Sock_send(&(_this->_send),send_pac);

    _this->_next_ping_time = Sock_timestamp() + _ping_interval;
  }
}


uint64_t Ack_wait_time(struct Acker *_this){
  if(_this->_server ){
    return 1000000000;
  }
  int diff = _this->_next_ping_time - Sock_timestamp();
  if( diff < 0){
    diff = 0;
  }

  return diff;
}
