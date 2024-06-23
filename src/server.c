#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "server.h"
#include "user.h"
#include "encryption.h"
#include "logger.h"

typedef struct
{
    int socket;
    struct sockaddr_in address;
    char username[50];
} client_t;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(char *message, int sender_socket)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] && clients[i]->socket != sender_socket)
        {
            char encrypted_msg[1024];
            custom_encrypt(message, encrypted_msg);
            send(clients[i]->socket, encrypted_msg, strlen(encrypted_msg), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg)
{
    client_t *client = (client_t *)arg;
    char buffer[1024];
    int read_size;

    // Wait for the username
    read_size = recv(client->socket, buffer, sizeof(buffer), 0);
    buffer[read_size] = '\0';
    if (strncmp(buffer, "USERNAME:", 9) == 0)
    {
        strncpy(client->username, buffer + 9, sizeof(client->username) - 1);
        client->username[sizeof(client->username) - 1] = '\0';
    }

    while ((read_size = recv(client->socket, buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[read_size] = '\0';
        char decrypted_msg[1024];
        custom_decrypt(buffer, decrypted_msg);

        char broadcast_buffer[1100];
        snprintf(broadcast_buffer, sizeof(broadcast_buffer), "%s: %s", client->username, decrypted_msg);
        broadcast_message(broadcast_buffer, client->socket);
    }

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == client)
        {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(client->socket);
    free(client);
    pthread_detach(pthread_self());

    return NULL;
}

void run_server()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        log_fatal("Error creating server socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        log_fatal("Error binding server socket");
        exit(1);
    }

    if (listen(server_socket, 10) < 0)
    {
        log_fatal("Error listening on server socket");
        exit(1);
    }

    log_info("Server started. Waiting for connections...");

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0)
        {
            log_error("Error accepting client connection");
            continue;
        }

        client_t *client = malloc(sizeof(client_t));
        client->socket = client_socket;
        client->address = client_addr;

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] == NULL)
            {
                clients[i] = client;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)client) != 0)
        {
            log_error("Error creating thread for client");
            free(client);
        }
    }
}