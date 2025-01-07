#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_PEERS 100
#define BUFFER_SIZE 1024

typedef struct {
    uint32_t ip;  // Adresa IP a peer-ului
    uint16_t port; // Portul peer-ului
} PeerInfo;

// Lista globală de peers pentru server
PeerInfo peers[MAX_PEERS];
int peer_count = 0;

// Adaugă un peer în lista globală (doar pentru testare)
void add_peer(const char *ip, uint16_t port) {
    if (peer_count >= MAX_PEERS) {
        fprintf(stderr, "Lista de peers este plină!\n");
        return;
    }

    PeerInfo peer;
    inet_pton(AF_INET, ip, &peer.ip);
    peer.port = port;
    peers[peer_count++] = peer;
}


// Funcția care gestionează cererile `PEER_DISCOVERY`
void handle_peer_discovery(int client_socket) {
    char buffer[BUFFER_SIZE];
    recv(client_socket, buffer, sizeof(buffer), 0);

    if (strncmp(buffer, "PEER_DISCOVERY", 14) == 0) {
        // Trimite lista de peers
        uint64_t vector_size = peer_count;
        send(client_socket, "PEER_LIST", 9, 0);
        send(client_socket, &vector_size, sizeof(vector_size), 0);

        for (int i = 0; i < peer_count; i++) {
            send(client_socket, &peers[i], sizeof(PeerInfo), 0);
        }
    }

    close(client_socket);
}
