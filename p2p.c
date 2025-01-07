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
