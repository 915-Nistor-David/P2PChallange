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

// Lista globala de peers pentru server
PeerInfo peers[MAX_PEERS];
int peer_count = 0;

// Adauga un peer in lista globala (doar pentru testare)
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


// Functia care gestionează cererile `PEER_DISCOVERY`
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



// Functia care pornește serverul TCP
void start_tcp_server(int port) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Creare socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }

    // Configurare adresa server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Legare socket de port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Eroare la bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Asteptare conexiuni
    if (listen(server_socket, 5) == -1) {
        perror("Eroare la listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("TCP Server pornit pe portul %d...\n", port);

    // Acceptare conexiuni
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket == -1) {
            perror("Eroare la accept");
            continue;
        }

        handle_peer_discovery(client_socket);
    }

    close(server_socket);
}