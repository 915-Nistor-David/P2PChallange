#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define CHUNK_SIZE 4096
#define BUFFER_SIZE 1024

typedef struct {
    char file_name[256];
    uint64_t chunk_number;
} ChunkRequest;

typedef struct {
    uint64_t chunk_number;
    uint8_t data[CHUNK_SIZE];
} ChunkResponse;
