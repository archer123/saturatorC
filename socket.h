#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

#include <stdint.h>
#include <stdbool.h>


/***
  class Address
***/

typedef struct Address Address;
struct Address{
  struct sockaddr_in _sockaddr;
};

//constructor
void IAddress_1 (struct Address *_this, char* ip, uint16_t port);
void IAddress_2(struct Address *_this, struct sockaddr_in s_sockaddr){
  _this ->_sockaddr = s_sockaddr;
}

const char* Addr_str (const struct Address *_this);
char* Addr_ip (struct Address *_this);
bool Addr_equal(struct Address *_this, Address *other){
  return (0 == memcmp(&(_this->_sockaddr), &(other->_sockaddr), sizeof(_this->_sockaddr)));
}

const struct sockaddr_in Addr_sockaddr_in (const struct Address *_this){
  return _this->_sockaddr;
}

const struct sockaddr_in Addr_sockaddr(const struct Address *_this){
  return _this->_sockaddr;
}



/**
  class Packet
**/

typedef struct Packet Packet;
struct Packet{
  Address addr;
  char* payload;
  uint64_t timestamp;
};

// packet constructor
void IPacket_1(struct Packet *_this, Address s_addr, char* s_payload){
  _this ->addr = s_addr;
  _this ->payload = s_payload;
  _this ->timestamp = 0;
}

void IPacket_2(struct Packet *_this, Address s_addr, char* s_payload, struct timespec ts){
  _this ->addr = s_addr;
  _this ->payload = s_payload;
  _this ->timestamp = ts.tv_sec*1000000000 + ts.tv_nsec;
}

/**
  class socket
**/
typedef struct Socket Socket;
struct Socket{
  struct Address Address;
  struct Packet Packet;

  int sock;

};

int get_sock(struct Socket *_this){
  return _this ->sock;
}

uint64_t Sock_timestamp (Socket *_this);
void Sock_ISocket(struct Socket *_this);
void Sock_bind(const struct Socket *_this, const struct Address *addr);
void Sock_connect (const struct Socket *_this, const struct Address *addr);
void Sock_send (const struct Socket *_this, const struct Packet packet);
void Sock_bind_to_device(const struct Socket *_this, const char* name);
Packet Sock_recv (const struct Socket *_this);
int get_sock (struct Socket *_this);

#endif
