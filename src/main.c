#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "client.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <mode>\n", argv[0]);
        fprintf(stderr, "mode: 'server' or 'client'\n");
        exit(1);
    }

    init_logger();

    if (strcmp(argv[1], "server") == 0)
    {
        run_server();
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        run_client();
    }
    else
    {
        log_fatal("Invalid mode. Use 'server' or 'client'.");
        exit(1);
    }

    return 0;
}