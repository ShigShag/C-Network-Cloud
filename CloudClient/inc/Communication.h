#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "ClientBackend.h"

/* TOKENS */

#define INVALID_INPUT (-1)
#define INTERNAL_EXIT (-2)

#define ERROR_TOKEN 0xF

#define PUSH_FILE 0x5
#define PULL_FILE 0x6
#define LIST_FILE 0x7

#define HEADER_SIZE 5

/* Header Structure:
 * [TOKEN][MESSAGE LENGTH][MESSAGE       ]
 * [1 Byt][4 Byte        ][msg Len. Bytes]
 * */

/*  Header */
uint8_t *Uint32ToUint8(uint32_t value);
uint32_t Uint8ToUint32(const uint8_t *ByteArray);
uint8_t *GetHeader(uint32_t ByteArraySize, int32_t Token, uint32_t HeaderSize);
uint32_t ProcessHeader(uint8_t *ByteArray, uint32_t ByteArraySize, int32_t *Token, uint32_t *MessageSize);

/* Byte communication */
uint32_t SendBytes(Client *c, uint8_t *ByteArray, uint32_t ByteArraySize, int32_t Token);
uint64_t ReceiveBytes(Client *c, uint8_t **Destination, uint64_t *BytesReceived_);

/* Special communication */

// Client first connection header
/* Header Structure:
 * [Token ][Client id]
 * [1 Byte][4 Byte   ]
 * */

#define FIRST_COMMUNICATION_HEADER_SIZE (sizeof(uint8_t) + sizeof(uint32_t)) // = 5
#define NORMAL_MODE 0x11
#define DOWNLOAD_MODE 0x12 
#define TEST_MODE 0x13

uint64_t SendInitialHeader(Client *c, uint8_t token, uint32_t id);
//int32_t ReceiveInitialHeader(int socket, uint8_t *token, uint32_t *id);

// Client first Answer header
/* Header Structure:
 * [Token ][Client id]
 * [1 Byte][4 Byte   ]
 * */

#define FIRST_HANDSHAKE_HEADER_SIZE (sizeof(uint8_t) + sizeof(uint32_t)) // = 5
#define ALL_OK 0x21
#define SERVER_FULL 0x22
#define DOWNLOAD_READY 0x23
#define TEST_READY 0x24
#define ABORD 0x25

uint8_t *GetHandshakeHeader(uint8_t token, int32_t id);
//uint64_t SendInitialHandshake(int socket, uint8_t token, uint32_t id);
int32_t ReceiveInitialHandshake(Client *c, uint8_t *token, int32_t *id);

// File transmition
#define FILE_ALREADY_EXISTS 0x31
#define FILE_DOES_NOT_EXIST 0x32

#define FILE_HEADER_SIZE (sizeof(unsigned long)) // = 8
#define FILE_BLOCK_SIZE 1000 * 400

/* Header Structure:
 * [FILE BASE NAME][FILE LENGTH   ][File bytes       ]
 * [NAME_MAX = 255][8 Byte        ][msg Len. Bytes   ] 
 * */

/* Header */
uint8_t *Uint64ToUint8(uint64_t value);
uint64_t Uint8ToUint64(const uint8_t *ByteArray);

uint8_t *GetFileHeader(uint64_t ByteArraySize);
int32_t ProcessFileHeader(uint8_t *ByteArray, uint64_t ByteArraySize, uint64_t *FileSize);

/* Transmision */
uint64_t SendFile(Client *c, uint8_t *ByteArray, uint64_t ByteArraySize);  
uint64_t SendFile_t(Client *c, int fd);

/* Receiving */
uint64_t ReceiveFile(Client *c, FILE *fp);

#endif