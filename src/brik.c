/**
 * @file brik.c
 * @author Bato
 * @date  23 April 2019
 * @brief
 *
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 */

/* ******* INCLUDE ******* */
/* *** LOCAL *** */
#include "brik_api.h"

int main(int argc, char* argv[])
{
    int sock_tcp;
    int connection_client;
    unsigned int client_addr_size;
    struct sockaddr_in  sock_tcp_addr;
    struct sockaddr_in  client_addr;
    int connection_count = 0;

    int ret = 0;

    printf("Hello Brik\n");

    // init display
    ret = dh_display_init();

    if (ret != 0)
    {
        printf("Failed to initialize Display\n");
        exit(1);
    }

    // clear display w/ black screen
    dh_display_clean();

    //init video thread
    vh_init_handler_thread();

    //init socket and server thread
    sock_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_tcp == -1)
    {
        printf("Failed to open receiver socket\n");
        exit(1);
    }

    memset(&sock_tcp_addr, 0, sizeof(sock_tcp_addr));
    sock_tcp_addr.sin_family = AF_INET;
    sock_tcp_addr.sin_port = htons(9500);
    sock_tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_tcp, (struct sockaddr*)&sock_tcp_addr, sizeof(sock_tcp_addr)) == -1)
    {
        printf("Failed to bind socket listener\n");
        exit(1);
    }

    if (listen(sock_tcp, 30) == -1)
    {
        printf("Listen Failed\n");
        exit(1);
    }

    while(true)
    {
        printf("Waiting for connection %d\n", connection_count);
        client_addr_size = sizeof(client_addr);
        connection_client = accept(sock_tcp, (struct sockaddr *)&client_addr, &client_addr_size);

        if (connection_client == -1)
        {
            printf("Connection Failed\n");
            close(connection_client);
            exit(1);
        }
        else
        {
            printf("info_connection: count %d, new connection %d\n", connection_count, connection_client);
            // open handler thread for packet deliverey
            if(ph_init_handler_thread(connection_count, connection_client) == -1)
            {
                printf("Failed to create packet handler %d\n", connection_count);
                exit(1);
            }
            connection_count++;
        }
    }

    return 0;
}
