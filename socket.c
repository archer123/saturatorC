#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


#include "socket.h"

void IAddress_1(Address *_this, char* ip, uint16_t port){
  _this ->_sockaddr.sin_family = AF_INET;
  _this ->_sockaddr.sin_port = htons(port);
  if (inet_aton(ip, &(_this ->_sockaddr.sin_addr)) == 0){
    fprintf(stderr, "Invalid IP address (%s)\n", ip);
    exit(1);
  }
}

char* Addr_ip(struct Address *_this){
  char* tmp = (char*)malloc(64*sizeof(char));
  snprintf(tmp, 64, "%s", inet_ntoa(_this ->_sockaddr.sin_addr));
  return tmp;
}

const char* Addr_str(const struct Address *_this){
  char* tmp = (char*)malloc(64*sizeof(char));
  snprintf(tmp, 64, "%s:%d", inet_ntoa(_this ->_sockaddr.sin_addr), ntohs(_this ->_sockaddr.sin_port));
  return tmp;
}

void ISocket(struct Socket *_this){
  _this ->sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (_this ->sock < 0){
    perror("socket");
    exit ( 1 );
  }
  int ts_opt = 1;
  if (setsockopt( _this ->sock, SOL_SOCKET, SO_TIMESTAMPNS, &ts_opt, sizeof( ts_opt )) < 0){
    perror( "setsockopt" );
    exit(1);
  }
}

//maybe wrong, should be reviewed
void Sock_connect(const struct Socket *_this, const struct Address *addr){
  if(connect(_this->sock,  (struct sockaddr *) &(addr->_sockaddr),
  sizeof(Addr_sockaddr(addr)) ) < 0){
    fprintf(stderr, "Error binding to %s\n", Addr_str(addr) );
    perror("bind");
    exit(1);
  }
}

void Sock_bind(const struct Socket *_this, const struct Address *addr){
  if(bind(_this->sock,  (struct sockaddr *) &(addr->_sockaddr), sizeof(Addr_sockaddr(addr)) ) < 0){
    fprintf(stderr, "Error binding to %s\n", Addr_str(addr) );
    perror("bind");
    exit(1);
  }
}

void Sock_send(const struct Socket *_this, const struct Packet packet){
  ssize_t bytes_sent = sendto (_this->sock, packet.payload, sizeof(packet.payload), 0,
    (struct sockaddr *) &((packet.addr._sockaddr)), sizeof((packet.addr._sockaddr)));
  if (bytes_sent != (ssize_t)(sizeof(packet.payload))){
    perror("sendto");
  }
}

void Sock_bind_to_device(const struct Socket *_this, const char* name){
  if(setsockopt (_this->sock, SOL_SOCKET, SO_BINDTODEVICE, name, sizeof(name))< 0 ){
    fprintf(stderr, "Error binding to %s\n", name);
    perror("setsockopt SO_BINDTODEVICE");
    exit(1);
  }
}

Packet Sock_recv(const struct Socket *_this){
  /* data structure to receive timestamp, source address, and payload */
  struct sockaddr_in remote_addr;
  struct msghdr header;
  struct iovec msg_iovec;

  const int BUF_SIZE = 2048;

  char msg_payload[ BUF_SIZE ];
  char msg_control[ BUF_SIZE ];
  header.msg_name = &remote_addr;
  header.msg_namelen = sizeof( remote_addr );
  msg_iovec.iov_base = msg_payload;
  msg_iovec.iov_len = BUF_SIZE;
  header.msg_iov = &msg_iovec;
  header.msg_iovlen = 1;
  header.msg_control = msg_control;
  header.msg_controllen = BUF_SIZE;
  header.msg_flags = 0;

  ssize_t received_len = recvmsg( _this -> sock, &header, 0 );
  if ( received_len < 0 ) {
    perror( "recvmsg" );
    exit( 1 );
  }

  if ( received_len > BUF_SIZE ) {
   fprintf( stderr, "Received oversize datagram (size %d) and limit is %d\n",
      (int)( received_len ), BUF_SIZE );
   exit( 1 );
 }

 /* verify presence of timestamp */
 struct cmsghdr *ts_hdr = CMSG_FIRSTHDR( &header );
 assert( ts_hdr );
 assert( ts_hdr->cmsg_level == SOL_SOCKET );
 assert( ts_hdr->cmsg_type == SO_TIMESTAMPNS );

 return IPacket_2( Address( remote_addr ),
      msg_payload,
      *(struct timespec *)CMSG_DATA( ts_hdr ) ) );
}
