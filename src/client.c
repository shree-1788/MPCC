#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "client.h"
#include "user.h"
#include "encryption.h"
#include "logger.h"

#define SERVER_IP "127.0.0.1"
#define PORT 8888

int client_socket;
char username[50];

void *receive_messages(void *arg)
{
    char buffer[1024];
    int read_size;
    while (1)
    {
        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));

        // Receive message from server
        read_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (read_size > 0)
        {
            // Null-terminate the received data
            buffer[read_size] = '\0';

            // Decrypt the message
            char decrypted_msg[1024];
            custom_decrypt(buffer, decrypted_msg);

            // Display the message
            printf("%s\n", decrypted_msg);
        }
        else if (read_size == 0)
        {
            // Connection closed by the server
            log_warning("Server disconnected");
            break;
        }
        else
        {
            // Error occurred
            log_error("recv failed");
            break;
        }
    }

    // If we're here, the connection has been closed or an error occurred
    log_info("Receive thread ending");
    pthread_exit(NULL);
}

void run_client()
{
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        log_fatal("Error creating client socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        log_fatal("Error connecting to server");
        exit(1);
    }

    log_info("Connected to server");

    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    char password[50];
    printf("Enter password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    // New registration option
    printf("Do you want to register as a new user? (y/n): ");
    char register_choice[2];
    fgets(register_choice, sizeof(register_choice), stdin);
    if (register_choice[0] == 'y' || register_choice[0] == 'Y')
    {
        if (register_user(username, password))
        {
            log_info("User registered successfully");
        }
        else
        {
            log_error("User registration failed. User may already exist.");
            close(client_socket);
            exit(1);
        }
    }

    if (!authenticate_user(username, password))
    {
        log_warning("Authentication failed");
        close(client_socket);
        exit(1);
    }

    log_info("Authentication successful");
    // Send the username to the server
    char username_msg[1024];
    snprintf(username_msg, sizeof(username_msg), "USERNAME:%s", username);
    send(client_socket, username_msg, strlen(username_msg), 0);

    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0)
    {
        log_error("Error creating receive thread");
        close(client_socket);
        exit(1);
    }

    char message[1024];
    while (1)
    {
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = 0;

        if (strcmp(message, "exit") == 0)
        {
            break;
        }

        char encrypted_msg[1024];
        custom_encrypt(message, encrypted_msg);
        send(client_socket, encrypted_msg, strlen(encrypted_msg), 0);
    }

    close(client_socket);
}