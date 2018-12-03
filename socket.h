#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/***
  class Address
***/

typedef struct Address Address;
struct Address{
  struct sockaddr_in _sockaddr;
};

/**
  class Packet
**/

typedef struct Packet Packet;
struct Packet{
  Address addr;
  char* payload;
  uint64_t timestamp;
};

/**
  class socket
**/
typedef struct Socket Socket;
struct Socket{
  struct Address address;
  struct Packet packet;

  int sock;
};


uint64_t Sock_timestamp();
void Sock_ISocket(struct Socket *_this);
void Sock_bind(struct Socket *_this, const struct Address addr);
void Sock_connect (struct Socket *_this, const struct Address addr);
void Sock_send (struct Socket *_this, const struct Packet packet);
void Sock_bind_to_device(struct Socket *_this, const char* name);
Packet Sock_recv (struct Socket *_this);
int get_sock (struct Socket *_this);

/*******************************Address part**********************************************/
//constructor
void IAddress_1(struct Address *_this, char* ip, uint16_t port);
void IAddress_2(struct Address *_this, struct sockaddr_in s_sockaddr);

const char* Addr_str (const struct Address *_this);
char* Addr_ip (struct Socket *_this);
bool Addr_equal(struct Address *_this, const Address other);

const struct sockaddr_in Addr_sockaddr_in (const struct Address *_this);

const struct sockaddr_in Addr_sockaddr(const struct Address *_this);

/*********************************************Packet part*******************************************************/
// packet constructor
void IPacket_1(struct Packet *_this, Address s_addr, char* s_payload);
void IPacket_2(struct Packet *_this, Address s_addr, char* s_payload, struct timespec ts);


#endif
