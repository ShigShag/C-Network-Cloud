#include "../inc/Communication.h"

uint8_t *Uint32ToUint8(uint32_t value)
{
    uint8_t *Array = (uint8_t *) calloc(sizeof (uint32_t), sizeof (uint8_t));
    if(Array)
    {
        for(uint32_t i = 0;i < sizeof (uint32_t);i++)
        {
            Array[i] = (value >> ((sizeof (uint32_t) * 8) - ((i + 1) * 8))) & 0xFF;
        }
    }
    return Array;
}
uint32_t Uint8ToUint32(const uint8_t *ByteArray)
{
    uint32_t value = 0;
    uint32_t i;

    if(ByteArray != NULL)
    {
        for (i = 0; i < sizeof (uint32_t) - 1; i++)
        {
            value = (value | ByteArray[i]) << 8;
        }
        value = (value | ByteArray[i]);
    }
    return value;
}
uint8_t *GetHeader(uint32_t ByteArraySize, int32_t Token, uint32_t HeaderSize)
{
    uint8_t *Header = calloc(HeaderSize, sizeof(uint8_t));
    uint8_t *SizeArray;

    if(Header)
    {
        Header[0] = Token;

        SizeArray = Uint32ToUint8(ByteArraySize);

        memcpy(Header + 1, SizeArray, HeaderSize - 1);

        free(SizeArray);
    }
    return Header;
}
uint32_t ProcessHeader(uint8_t *ByteArray, uint32_t ByteArraySize, int32_t *Token, uint32_t *MessageSize)
{
    if(ByteArray != NULL)
    {
        if (ByteArraySize == HEADER_SIZE)
        {
            *Token = ByteArray[0];

            *MessageSize = Uint8ToUint32(ByteArray + 1);

            return 1;
        }
    }
    return 0;
}
uint32_t SendBytes(Client *c, uint8_t *ByteArray, uint32_t ByteArraySize, int32_t Token)
{
    if(c->Active == 0 || c->is_connected  == 0) return 0;
    /* TODO tatsächliche bytesend returne */
    uint32_t BytesSend = 0;
    uint32_t TotalBytesSend = 0;

    uint8_t *Header = GetHeader(ByteArraySize, Token, HEADER_SIZE);

    if(Header == NULL) return 0;

    while(TotalBytesSend < HEADER_SIZE)
    {
        BytesSend = send(c->socket, Header + TotalBytesSend, HEADER_SIZE - TotalBytesSend, 0);
        if (BytesSend <= 0)
        {
            printf("Send Header failed\n");
            free(Header);
            ReportDisconnect(c);
            return 0;
        }
        TotalBytesSend += BytesSend;
    }
    TotalBytesSend = 0;
    while(TotalBytesSend < ByteArraySize)
    {
        BytesSend = send(c->socket, ByteArray + TotalBytesSend, ByteArraySize - TotalBytesSend, 0);
        if (BytesSend <= 0)
        {
            printf("Send Data failed\n");
            free(Header);
            ReportDisconnect(c);
            return 0;
        }
        TotalBytesSend += BytesSend;
    }
    free(Header);
    return TotalBytesSend + HEADER_SIZE;
}
uint64_t ReceiveBytes(Client *c, uint8_t **Destination, uint64_t *BytesReceived_)
{
    if(c->Active || c->is_connected == 0)
    {
        uint64_t BytesReceived = 0;
        uint64_t TotalBytesReceived = 0;
        uint32_t BytesToBeReceived = HEADER_SIZE;
        int32_t Token = 0;

        if(Destination) *Destination = NULL;
        if(BytesReceived_) *BytesReceived_ = 0;

        /* Receive Header */
        uint8_t *Buffer = (uint8_t *) malloc(HEADER_SIZE * sizeof (uint8_t));
        if(Buffer)
        {
            while (TotalBytesReceived < BytesToBeReceived)
            {
                BytesReceived = recv (c->socket, Buffer + TotalBytesReceived, BytesToBeReceived - TotalBytesReceived, 0);
                if(BytesReceived <= 0)
                {
                    ReportDisconnect(c);
                    free(Buffer);
                    if(BytesReceived_) *BytesReceived_ = TotalBytesReceived;
                    return 0;
                }
                TotalBytesReceived += BytesReceived;
            }

            if (!ProcessHeader(Buffer, HEADER_SIZE, &Token, &BytesToBeReceived))
            {
                free(Buffer);
                return 0;
            }

            /* Receive actual message */
            if(BytesToBeReceived > 0)
            {
                TotalBytesReceived = 0;

                free(Buffer);

                Buffer = (uint8_t *) malloc(BytesToBeReceived * sizeof (uint8_t));
                if (Buffer)
                {
                    while(TotalBytesReceived < BytesToBeReceived)
                    {
                        BytesReceived = recv(c->socket, Buffer + TotalBytesReceived,BytesToBeReceived - TotalBytesReceived, 0);
                        if(BytesReceived <= 0)
                        {
                            ReportDisconnect(c);
                            free(Buffer);
                            if(BytesReceived_) *BytesReceived_ = BytesReceived;
                            return 0;
                        }
                        TotalBytesReceived += BytesReceived;
                    }
                    if(Destination) *Destination = Buffer;
                    if(BytesReceived_) *BytesReceived_ = BytesReceived;
                }
            }
            return Token;
        }
    }
    return 0;
}

// Client first connection header
/* Header Structure:
 * [Token ][Client id]
 * [1 Byte][4 Byte   ]
 * */
uint64_t SendInitialHeader(Client *c, uint8_t token, uint32_t id)
{
    if(c == NULL) return 0;

    uint64_t BytesSend;
    uint64_t TotalBytesSend = 0;

    uint8_t *Header = GetHandshakeHeader(token, id);

    if(Header == NULL) return 0;

    while(TotalBytesSend < FIRST_HANDSHAKE_HEADER_SIZE)
    {
        BytesSend = send(c->socket, Header + TotalBytesSend, FIRST_HANDSHAKE_HEADER_SIZE - TotalBytesSend, 0);
        if(BytesSend <= 0)
        {
            printf("Send Header failed\n");
            free(Header);
            return 0;
        }
        TotalBytesSend += BytesSend;
    }
    free(Header);
    return TotalBytesSend;
}

int32_t ReceiveInitialHandshake(Client *c, uint8_t *token, int32_t *id)
{
    if(c == NULL) return 0;

    uint64_t BytesReceived = 0;
    uint64_t TotalBytesReceived = 0;
    uint32_t BytesToBeReceived = FIRST_COMMUNICATION_HEADER_SIZE;

    uint8_t *Buffer = (uint8_t *) malloc(FIRST_COMMUNICATION_HEADER_SIZE * sizeof(uint8_t));
    if(!Buffer) return 0;

    while(TotalBytesReceived < BytesToBeReceived)
    {
        BytesReceived = recv(c->socket, Buffer + TotalBytesReceived, BytesToBeReceived - TotalBytesReceived, 0);
        if(BytesReceived <= 0)
        {
            free(Buffer);
            return 0;
        }
        TotalBytesReceived += BytesReceived;
    }
    *token = Buffer[0];
    *id = Uint8ToUint32(Buffer + 1);

    free(Buffer);
    return 1;
}

/*
int32_t ReceiveInitialHeader(int socket, uint8_t *token, uint32_t *id)
{
    if(socket == -1) return 0;

    uint64_t BytesReceived = 0;
    uint64_t TotalBytesReceived = 0;
    uint32_t BytesToBeReceived = FIRST_COMMUNICATION_HEADER_SIZE;

    uint8_t *Buffer = (uint8_t *) malloc(FIRST_COMMUNICATION_HEADER_SIZE * sizeof(uint8_t));
    if(!Buffer) return 0;

    while(TotalBytesReceived < BytesToBeReceived)
    {
        BytesReceived = recv(socket, Buffer + TotalBytesReceived, BytesToBeReceived - TotalBytesReceived, 0);
        if(BytesReceived <= 0)
        {
            free(Buffer);
            return 0;
        }
        TotalBytesReceived += BytesReceived;
    }
    *token = Buffer[0];
    *id = Uint8ToUint32(Buffer + 1);

    free(Buffer);
    return 1;
}*/
uint8_t *GetHandshakeHeader(uint8_t token, int32_t id)
{
    uint8_t *id_array;
    uint8_t *Header = (uint8_t *) malloc(FIRST_HANDSHAKE_HEADER_SIZE * sizeof(uint8_t));
    if(Header == NULL) return NULL;

    Header[0] = token;
    id_array = Uint32ToUint8(id);

    memcpy(Header + 1, id_array, sizeof(int32_t));
    free(id_array);
    return Header;
}
/*
uint64_t SendInitialHandshake(int socket, uint8_t token, uint32_t id)
{
    if(socket == -1) return 0;

    uint64_t BytesSend = 0;
    uint64_t TotalBytesSend = 0;
    uint8_t *Buffer = GetHandshakeHeader(token, id);
    if(Buffer == NULL) return 0;

    while(TotalBytesSend < FIRST_HANDSHAKE_HEADER_SIZE)
    {
        BytesSend = send(socket, Buffer + TotalBytesSend, FIRST_HANDSHAKE_HEADER_SIZE - TotalBytesSend, 0);
        if(BytesSend <= 0)
        {
            free(Buffer);
            return 0;
        }
        TotalBytesSend += BytesSend;
    }

    free(Buffer);
    return TotalBytesSend;
}*/
/* File transmition */
uint8_t *Uint64ToUint8(uint64_t value)
{
    uint8_t *Array = (uint8_t *) calloc(sizeof (uint64_t), sizeof (uint8_t));
    if(Array)
    {
        for(uint64_t i = 0;i < sizeof (uint64_t);i++)
        {
            Array[i] = (value >> ((sizeof (uint64_t) * 8) - ((i + 1) * 8))) & 0xFF;
        }
    }
    return Array;
}
uint64_t Uint8ToUint64(const uint8_t *ByteArray)
{
    uint64_t value = 0;
    uint32_t i;

    if(ByteArray != NULL)
    {
        for (i = 0; i < sizeof (uint64_t) - 1; i++)
        {
            value = (value | ByteArray[i]) << 8;
        }
        value = (value | ByteArray[i]);
    }
    return value;
}
uint8_t *GetFileHeader(uint64_t ByteArraySize)
{
    if(ByteArraySize < 0) return 0;

    uint8_t *Header = calloc(FILE_HEADER_SIZE, sizeof(uint8_t));
    uint8_t *SizeArray;

    if(Header)
    {
        SizeArray = Uint64ToUint8(ByteArraySize);

        memcpy(Header, SizeArray, sizeof(uint64_t));
    }
    return Header;
}
int32_t ProcessFileHeader(uint8_t *ByteArray, uint64_t ByteArraySize, uint64_t *FileSize)
{
    if(ByteArray == NULL || ByteArraySize != FILE_HEADER_SIZE) return 0;

    *FileSize = Uint8ToUint64(ByteArray);

    return 1;
}
uint64_t SendFile_t(Client *c, int fd, ProgressBar *pb)
{
    if(c == NULL || fd == -1) return 0;
    if(c->Active == 0) return 0;

    uint64_t BytesSend = 0;
    uint64_t TotalBytesSend = 0;
    struct stat f_stat;

    int progress_bar_given = pb != NULL;

    if(fstat(fd, &f_stat) <  0)
    {
        printf("[-] fstat failed: %s\n", strerror(errno));
        return 0;
    }
    printf("File size: %ld\n", f_stat.st_size);
    
    uint8_t *Buffer = GetFileHeader(f_stat.st_size);
    if(Buffer == NULL) return 0;
    
    while(TotalBytesSend < FILE_HEADER_SIZE)
    {
        BytesSend = send(c->socket, Buffer + TotalBytesSend, FILE_HEADER_SIZE - TotalBytesSend, 0);
        if (BytesSend <= 0)
        {
            free(Buffer);
            ReportDisconnect(c);
            printf("%s\n", strerror(errno));
            return 0;
        }
        TotalBytesSend += BytesSend;
    }
    free(Buffer);

    TotalBytesSend = 0;

    while(TotalBytesSend < f_stat.st_size)
    {
        BytesSend = sendfile(c->socket, fd, NULL, FILE_BLOCK_SIZE);

        if (BytesSend <= 0)
        {
            ReportDisconnect(c);
            printf("%s\n", strerror(errno));
            return 0;
        }
        TotalBytesSend += BytesSend;
        if(progress_bar_given)
        {
            //pb->progress = (double) TotalBytesSend / f_stat.st_size;
            //Print_Progress_Bar(pb);
        }
    }
    
    return TotalBytesSend;
}
uint64_t SendFile(Client *c, uint8_t *ByteArray, uint64_t ByteArraySize)
{
    if(c == NULL || ByteArray == NULL) return 0;
    if(c->Active == 0 || c->is_connected == 0) return 0;

    uint64_t BytesSend = 0;
    uint64_t TotalBytesSend = 0;
    
    uint8_t *Header = GetFileHeader(ByteArraySize);
    if(Header == NULL) return 0;

    while (TotalBytesSend < FILE_HEADER_SIZE)
    {
        BytesSend = send(c->socket, Header + TotalBytesSend, FILE_HEADER_SIZE - TotalBytesSend, 0);
        if (BytesSend <= 0)
        {
            free(Header);
            ReportDisconnect(c);
            printf("%s\n", strerror(errno));
            return 0;
        }
        TotalBytesSend += BytesSend;
    }
    TotalBytesSend = 0;
    while (TotalBytesSend < ByteArraySize)
    {
        BytesSend = send(c->socket, ByteArray + TotalBytesSend, ByteArraySize - TotalBytesSend, 0);
        if (BytesSend <= 0)
        {
            free(Header);
            ReportDisconnect(c);
            printf("%s\n", strerror(errno));
            return 0;
        }
        TotalBytesSend += BytesSend;
    }
    free(Header);
    return TotalBytesSend;
}
uint64_t ReceiveFile(Client *c, FILE *fp)
{
    if(c == NULL || fp == NULL) return 0;
    if(c->Active == 0 || c->is_connected == 0) return 0;

    int passed;
    uint64_t BytesReceived = 0;
    uint64_t BytesToBeReceived = FILE_HEADER_SIZE;
    uint64_t TotalBytesReceived = 0;

    /* Receive Header */
    uint8_t *Buffer = (uint8_t *) malloc(FILE_HEADER_SIZE * sizeof (uint8_t));

    if(!Buffer)
    {
        printf("%s\n", strerror(errno));
        return 0;        
    }
    while (TotalBytesReceived < BytesToBeReceived)
    {
        BytesReceived = recv(c->socket, Buffer + TotalBytesReceived, BytesToBeReceived - TotalBytesReceived, 0);
        if(BytesReceived <= 0)
        {
            ReportDisconnect(c);
            free(Buffer);
            printf("%s\n", strerror(errno));
            return 0;
        }
        TotalBytesReceived += BytesReceived;
    }
    
    passed = ProcessFileHeader(Buffer, BytesReceived, &BytesToBeReceived);
    if (passed == 0 || BytesToBeReceived == 0)
    {
        free(Buffer);
        printf("%s\n", strerror(errno));
        return 0;
    }
    printf("File to be received is of size: %ld\n", BytesToBeReceived);

    free(Buffer);
    Buffer = (uint8_t *) malloc(FILE_BLOCK_SIZE * sizeof(uint8_t));
    if(!Buffer)
    {
        printf("%s\n", strerror(errno));
        return 0;        
    }
    TotalBytesReceived = 0;
    while(TotalBytesReceived <  BytesToBeReceived)
    {
        BytesReceived = recv(c->socket, Buffer, FILE_BLOCK_SIZE, 0);
        if(BytesReceived <= 0)
        {
            ReportDisconnect(c);
            free(Buffer);
            printf("%s\n", strerror(errno));
            return 0;
        }
        TotalBytesReceived += BytesReceived;
        fwrite(Buffer, sizeof(uint8_t), BytesReceived, fp);
    }
    free(Buffer);
    return TotalBytesReceived;
}