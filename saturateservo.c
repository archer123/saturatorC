#include <assert.h>

#include "saturateservo.h"
#include "socket.h"
#include "payload.h"
#include "acker.h"



void ISaturateServo_1(struct SaturateServo *_this,
  char * s_name,
  FILE* log_file,
  const Socket s_listen,
  const Socket s_send,
  const Address s_remote,
  const bool s_server,
  const int s_send_id){

    _this -> _name = s_name;
    _this -> _log_file = log_file;
    _this -> _listen = s_listen;
    _this -> _send = s_send;
    _this -> _remote = s_remote;
    _this -> _server = s_server;
    _this -> _send_id = s_send_id;
    _this -> _acker = NULL;
    _this -> _next_transmission_time = Sock_timestamp();
    _this -> _foreign_id = -1;
    _this -> _packets_sent = 0;
    _this -> _max_ack_id = -1;
    _this -> _window = LOWER_WINDOW;
  }

  void SatServ_recv(struct SaturateServo *_this){
    Packet incoming = Sock_recv(&(_this -> _listen));
    SatPayload *contents = (SatPayload *) incoming.payload;
    contents->recv_timestamp = incoming.timestamp;

    if (contents -> sequence_number != -1){
      /* not an ack */
      printf( "MARTIAN!\n" );
      return;
    }

    /* possibly roam */
    if ( _this ->_server ) {
      if ( _this -> _acker ) {
        if ( (contents->sender_id > _this->_foreign_id) && (contents->ack_number == -1) ) {
  	       _this->_foreign_id = contents->sender_id;
  	       Ack_set_remote(_this->_acker, incoming.addr );
        }
      }
    }

    /* process the ack */
    if ( contents->sender_id != _this -> _send_id ) {
      /* not from us */
      return;
    } else {
      if ( contents->ack_number > _this -> _max_ack_id ) {
        _this->_max_ack_id = contents->ack_number;
    }

    int64_t rtt_ns = contents->recv_timestamp - contents->sent_timestamp;
    double rtt = rtt_ns / 1.e9;

    fprintf(_this -> _log_file, "%s ACK RECEIVED senderid=%d, seq=%d, send_time=%ld,  recv_time=%ld, rtt=%.4f, %d => ",
       _this->_name,_this->_server ? _this->_foreign_id : contents->sender_id , contents->ack_number, contents->sent_timestamp, contents->recv_timestamp, (double)rtt,  _this->_window );
    /* increase-decrease rules */

    if ( (rtt < LOWER_RTT) && (_this->_window < UPPER_WINDOW) ) {
      _this -> _window++;
    }

    if ( (rtt > UPPER_RTT) && (_this->_window > LOWER_WINDOW + 10) ) {
      _this -> _window -= 20;
    }

    fprintf( _this->_log_file, "%d\n", _this->_window );
    }
  }


  uint64_t SatServ_wait_time(struct SaturateServo *_this)
  {
    int num_outstanding = _this->_packets_sent - _this->_max_ack_id - 1;
    struct Address UNKNOWN;
    IAddress_1(&UNKNOWN, "0.0.0.0", 0);
    if ( Addr_equal(&(_this->_remote), UNKNOWN) ) {
      return 1000000000;
    }

    if ( num_outstanding < _this->_window ) {
      return 0;
    } else {
      int diff = _this->_next_transmission_time - Sock_timestamp();
      if ( diff < 0 ) {
        diff = 0;
      }
      return diff;
    }
  }


  void SatServ_tick(struct SaturateServo *_this )
  {
    struct Address UNKNOWN;
    IAddress_1(&UNKNOWN, "0.0.0.0", 0);
    if ( Addr_equal(&(_this->_remote), UNKNOWN) ) {
      return;
    }

    int num_outstanding = _this->_packets_sent -_this-> _max_ack_id - 1;

    if ( num_outstanding < _this->_window ) {
      /* send more packets */
      int amount_to_send =  _this->_window - num_outstanding;
      for ( int i = 0; i < amount_to_send; i++ ) {
        SatPayload outgoing;
        outgoing.sequence_number =  _this->_packets_sent;
        outgoing.ack_number = -1;
        outgoing.sent_timestamp = Sock_timestamp();
        outgoing.recv_timestamp = 0;
        outgoing.sender_id =  _this->_send_id;

        Packet send_pac;
        IPacket_1(&(send_pac), _this->_remote, str_satpayload(&outgoing, 1400));
        Sock_send(&(_this->_send), send_pac);

        /*
        printf( "%s pid=%d DATA SENT %d senderid=%d seq=%d, send_time=%ld, recv_time=%ld\n",
        _name.c_str(), getpid(), amount_to_send, outgoing.sender_id, outgoing.sequence_number, outgoing.sent_timestamp, outgoing.recv_timestamp ); */

         _this->_packets_sent++;
      }

       _this->_next_transmission_time = Sock_timestamp() +  _transmission_interval;
    }

    if (  _this->_next_transmission_time < Sock_timestamp() ) {
      SatPayload outgoing;
      outgoing.sequence_number =  _this->_packets_sent;
      outgoing.ack_number = -1;
      outgoing.sent_timestamp = Sock_timestamp();
      outgoing.recv_timestamp = 0;
      outgoing.sender_id =  _this->_send_id;

      Packet send_pac;
      IPacket_1(&(send_pac), _this->_remote, str_satpayload(&outgoing, 1400));
      Sock_send(&(_this->_send), send_pac);
      
      /*
      printf( "%s pid=%d DATA SENT senderid=%d seq=%d, send_time=%ld, recv_time=%ld\n",
      _name.c_str(), getpid(), outgoing.sender_id, outgoing.sequence_number, outgoing.sent_timestamp, outgoing.recv_timestamp ); */

       _this->_packets_sent++;

       _this->_next_transmission_time = Sock_timestamp() +  _transmission_interval;
    }
  }


void SatServ_set_remote(struct SaturateServo *_this, const Address s_remote){_this->_remote = s_remote;}
void SatServ_set_acker(struct SaturateServo *_this, struct Acker * const s_acker){_this -> _acker= s_acker;}
