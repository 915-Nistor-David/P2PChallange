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


// Functia pentru pornirea serverului UDP
void start_udp_server(int port) {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Creare socket UDP
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }

    // Configurare adresa server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Eroare la bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("UDP Server pornit pe portul %d...\n", port);

    // Asteptare cereri
    while (1) {
        char buffer[BUFFER_SIZE];
        recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);

        if (strncmp(buffer, "METADATA_REQ", 12) == 0) {
            // Trimite metadatele fisierului
            char file_name[256];
            sscanf(buffer + 13, "%s", file_name);

            FILE *file = fopen(file_name, "rb");
            if (!file) {
                sendto(server_socket, "NO_FILE", 7, 0, (struct sockaddr *)&client_addr, client_len);
                continue;
            }

            // Calculam numarul de chunks
            fseek(file, 0, SEEK_END);
            uint64_t file_size = ftell(file);
            uint64_t chunk_count = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
            fclose(file);

            sendto(server_socket, "METADATA_RES", 12, 0, (struct sockaddr *)&client_addr, client_len);
            sendto(server_socket, &chunk_count, sizeof(chunk_count), 0, (struct sockaddr *)&client_addr, client_len);
        } else if (strncmp(buffer, "CHUNK_REQ", 9) == 0) {
            // Trimite chunk-ul cerut
            ChunkRequest req;
            memcpy(&req, buffer + 9, sizeof(req));

            FILE *file = fopen(req.file_name, "rb");
            if (!file) {
                continue;
            }

            fseek(file, req.chunk_number * CHUNK_SIZE, SEEK_SET);
            ChunkResponse res;
            res.chunk_number = req.chunk_number;
            fread(res.data, 1, CHUNK_SIZE, file);
            fclose(file);

            sendto(server_socket, &res, sizeof(res), 0, (struct sockaddr *)&client_addr, client_len);
        }
    }

    close(server_socket);
}



// Functia pentru cererea metadatelor fisierului
void request_file_metadata(const char *peer_ip, int peer_port, const char *file_name) {
    int client_socket;
    struct sockaddr_in server_addr;

    // Creare socket UDP
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Eroare la crearea socket-ului");
        return;
    }

    // Configurare adresa server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(peer_port);
    inet_pton(AF_INET, peer_ip, &server_addr.sin_addr);

    // Trimite cerere pentru metadate
    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "METADATA_REQ %s", file_name);
    sendto(client_socket, request, strlen(request), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Primeste raspunsul
    char response[BUFFER_SIZE];
    recvfrom(client_socket, response, sizeof(response), 0, NULL, NULL);
    if (strncmp(response, "METADATA_RES", 12) == 0) {
        uint64_t chunk_count;
        recvfrom(client_socket, &chunk_count, sizeof(chunk_count), 0, NULL, NULL);
        printf("Fișierul %s are %lu chunks.\n", file_name, chunk_count);
    } else if (strncmp(response, "NO_FILE", 7) == 0) {
        printf("Fișierul %s nu este disponibil.\n", file_name);
    }

    close(client_socket);
}



// Functia pentru descarcarea fisierului
void request_file_chunks(const char *peer_ip, int peer_port, const char *file_name, uint64_t chunk_count) {
    int client_socket;
    struct sockaddr_in server_addr;

    // Creare socket UDP
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Eroare la crearea socket-ului");
        return;
    }

    // Configurare adresa server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(peer_port);
    inet_pton(AF_INET, peer_ip, &server_addr.sin_addr);

    // Descarcare chunks
    FILE *file = fopen(file_name, "wb");
    if (!file) {
        perror("Eroare la deschiderea fișierului");
        close(client_socket);
        return;
    }

    for (uint64_t i = 0; i < chunk_count; i++) {
        char request[BUFFER_SIZE];
        snprintf(request, sizeof(request), "CHUNK_REQ %s %lu", file_name, i);
        sendto(client_socket, request, strlen(request), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        ChunkResponse res;
        recvfrom(client_socket, &res, sizeof(res), 0, NULL, NULL);

        fwrite(res.data, 1, CHUNK_SIZE, file);
        printf("Chunk %lu descărcat.\n", i);
    }

    fclose(file);
    close(client_socket);
}