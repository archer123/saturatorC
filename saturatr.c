#include <string.h>
//#include <vector.h>
#include <poll.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#include "acker.h"
#include "saturateservo.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

int main(int argc, char *argv[]){
  Address UNKNOWN;
  IAddress_1(&UNKNOWN, "0.0.0.0", 0);
  if ( argc != 1 && argc != 6 ) {
    fprintf( stderr, "Usage: %s [RELIABLE_IP RELIABLE_DEV TEST_IP TEST_DEV SERVER_IP]\n",
	     argv[ 0 ]);
    exit( 1 );
  }
  Socket data_socket, feedback_socket;
  Sock_ISocket(&data_socket);
  Sock_ISocket(&feedback_socket);
  bool server;

  int sender_id = getpid();
  Address remote_data_address, remote_feedback_address;

  remote_data_address = UNKNOWN;
  remote_feedback_address = UNKNOWN;

  uint64_t ts=Sock_timestamp();
  if(argc == 1 ) {/* server */
      server = true;
      Address data_bind;

      IAddress_1(&data_bind, "0.0.0.0", 9001);
      //printf("data socket %d\n", get_sock(&data_socket));
      //printf("data bind addr %s\n", Addr_str(&data_bind));
      Sock_bind(&data_socket, data_bind );


      Address feedback_bind;
      IAddress_1(&feedback_bind, "0.0.0.0", 9002);
      Sock_bind( &feedback_socket, feedback_bind );
  } else{/* client */
    server = false;
    char *reliable_ip = argv[ 1 ];
    char *reliable_dev = argv[ 2 ];

    char *test_ip = argv[ 3 ];
    char *test_dev = argv[ 4 ];

    char *server_ip = argv[ 5 ];

    sender_id = ((int) (ts/1e9));

    Address test_bind;
    IAddress_1(&test_bind, test_ip, 9003);
    Sock_bind(&data_socket, test_bind);
    Sock_bind_to_device(&data_socket, test_dev);

    Address serv_data_addr;
    IAddress_1(&serv_data_addr, server_ip, 9001);
    remote_data_address = serv_data_addr;

    Address feedback_bind;
    IAddress_1(&feedback_bind, reliable_ip, 9004);
    Sock_bind(&feedback_socket, feedback_bind);
    Sock_bind_to_device(&feedback_socket, reliable_dev);

    Address serv_feed_addr;
    IAddress_1(&serv_feed_addr, server_ip, 9002);
    remote_feedback_address = serv_feed_addr;

  }

  FILE* log_file;
  char log_file_name[50];
  sprintf(log_file_name,"%s-%d-%d",server ? "server" : "client",(int)(ts/1e9),sender_id);
  log_file=fopen(log_file_name,"w");

  SaturateServo saturatr;
  ISaturateServo_1(&saturatr, "OUTGOING", log_file, feedback_socket, data_socket, remote_data_address, server, sender_id );

  Acker acker;
  IAcker1(&acker, "INCOMING", log_file, data_socket, feedback_socket, remote_feedback_address, server, sender_id );

  SatServ_set_acker(&saturatr, &acker );
  Ack_set_saturatr(&acker, &saturatr );

  while ( 1 ) {
    fflush( NULL );

    /* possibly send packet */
    SatServ_tick(&saturatr);
    Ack_tick(&acker);

    /* wait for incoming packet OR expiry of timer */
    struct pollfd poll_fds[ 2 ];
    poll_fds[ 0 ].fd = get_sock(&data_socket);
    poll_fds[ 0 ].events = POLLIN;
    poll_fds[ 1 ].fd = get_sock(&feedback_socket);
    poll_fds[ 1 ].events = POLLIN;

    struct timespec timeout;
    uint64_t next_transmission_delay = MIN( SatServ_wait_time(&saturatr), Ack_wait_time(&acker) );

    if ( next_transmission_delay == 0 ) {
      fprintf( stderr, "ZERO %ld %ld\n", SatServ_wait_time(&saturatr), Ack_wait_time(&acker));
    }

    timeout.tv_sec = next_transmission_delay / 1000000000;
    timeout.tv_nsec = next_transmission_delay % 1000000000;
    ppoll( poll_fds, 2, &timeout, NULL );

    if ( poll_fds[ 0 ].revents & POLLIN ) {
      Ack_recv(&acker);
    }

    if ( poll_fds[ 1 ].revents & POLLIN ) {
      SatServ_recv(&saturatr);
    }
  }
}
