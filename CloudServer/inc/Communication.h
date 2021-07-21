#ifndef SERVER_COMMUNICATION_H
#define SERVER_COMMUNICATION_H

#include "Server.h"

/* TOKENS */

#define ERROR_TOKEN 0xF

#define PUSH_FILE 0x5
#define PULL_FILE 0x6
#define LIST_FILE 0x7
#define DELETE_FILE 0x8
#define PUSH_FILE_FAST 0x9
#define CAT_FILE 0x10

/* HEADER */

#define HEADER_SIZE 5

/* Header Structure:
 * [TOKEN][MESSAGE LENGTH][MESSAGE       ]
 * [1 Byt][4 Byte        ][msg Len. Bytes]
 * */

/*  Header */
uint8_t *GetHeader(uint32_t ByteArraySize, int32_t Token, uint32_t HeaderSize);
uint32_t ProcessHeader(uint8_t *ByteArray, uint32_t ByteArraySize, int32_t *Token, uint32_t *MessageSize);

/* Communication */
uint32_t SendBytes(Client *c, uint8_t *ByteArray, uint32_t ByteArraySize, int32_t Token);
uint64_t ReceiveBytes(Client *c, uint8_t **Destination, uint64_t *BytesReceived_);

/* Special communication */

// Client first connection header
/* Header Structure:
 * [Token ][Client id]
 * [1 Byte][8  Byte  ]
 * */

#define CLIENT_ID_SIZE sizeof(uint64_t)
#define FIRST_COMMUNICATION_HEADER_SIZE (sizeof(uint8_t) + CLIENT_ID_SIZE) // = 9
#define NORMAL_MODE 0x11
#define DOWNLOAD_MODE 0x12 
#define TEST_MODE 0x13

int32_t ReceiveInitialHeader(int socket, uint8_t *token, uint64_t *id);

// Client first Answer header
/* Header Structure:
 * [Token ][Client id]
 * [1 Byte][8  Byte  ]
 * */

#define FIRST_HANDSHAKE_HEADER_SIZE (sizeof(uint8_t) + CLIENT_ID_SIZE) // = 9
#define ALL_OK 0x21
#define SERVER_FULL 0x22
#define DOWNLOAD_READY 0x23
#define TEST_READY 0x24
#define ABORD 0x25

uint8_t *GetHandshakeHeader(uint8_t token, uint64_t id);
uint64_t SendInitialHandshake(int socket, uint8_t token, uint64_t id);


// File transmition
#define FILE_ALREADY_EXISTS 0x31
#define FILE_DOES_NOT_EXIST 0x32

#define FILE_HEADER_SIZE (sizeof(unsigned long)) // = 8
#define FILE_BLOCK_SIZE 65535     // 4 Megabytes

#define DYNAMIC_CLIENT_TRANSMISSION_COUNT 4

/* Header Structure:
 * [FILE LENGTH   ][File bytes       ]
 * [8 Byte        ][msg Len. Bytes   ] 
 * */

/* Header */

uint8_t *GetFileHeader(uint64_t ByteArraySize);
int32_t ProcessFileHeader(uint8_t *ByteArray, uint64_t ByteArraySize, uint64_t *FileSize);

/* Transmision */
uint64_t SendFile(Client *c, uint8_t *ByteArray, uint64_t ByteArraySize); 
uint64_t SendFile_t(Client *c, int fd);

/* Receiving */
uint64_t ReceiveFile(Client *c, FILE *fp);

/* -------------------------------------------------------- */
/* Transmission client */
/* Header Structure:
 * [File offset   ][Byte count    ][File bytes       ]
 * [U  long       ][U long        ][msg Len. Bytes   ] 
 * */

#define DYNAMIC_CLIENT_TRANSMISSION_COUNT 4
#define TRANSMISSION_HEADER_SIZE (sizeof(uint64_t) * 2)  // = 16

//uint8_t *GetTransmissionHeader(uint64_t ByteArraySize, uint64_t offset);
//uint64_t SendFile_f(SEND_ARG *arg);
int32_t ProcessTransmissionHeader(uint8_t *Header, uint64_t ByteArraySize, int64_t *offset, uint64_t *count);
uint64_t ReceiveFile_f(RECV_ARG *arg);

#endif
