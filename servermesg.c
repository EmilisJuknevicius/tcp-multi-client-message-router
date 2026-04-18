#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

#define MAX_CLIENTS 4
#define BUFFER_SIZE 1024
const int PORTS[MAX_CLIENTS] = {20001,20002,20003,20004};

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    int server_sockets[MAX_CLIENTS] = {0};
    int client_sockets[MAX_CLIENTS] = {0};
    char buffer[BUFFER_SIZE];
    char lines[MAX_CLIENTS][BUFFER_SIZE];
    int pos[MAX_CLIENTS] = {0};

    // Create listening sockets for each client
    for(int i=0;i<MAX_CLIENTS;i++){
        server_sockets[i] = socket(AF_INET6, SOCK_STREAM, 0);
        struct sockaddr_in6 addr;
        memset(&addr,0,sizeof(addr));
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(PORTS[i]);
        addr.sin6_addr = in6addr_loopback;
        if(bind(server_sockets[i], (struct sockaddr*)&addr,sizeof(addr))==SOCKET_ERROR){
            printf("Bind failed for C%d\n", i+1); continue;
        }
        if(listen(server_sockets[i],1)==SOCKET_ERROR){
            printf("Listen failed for C%d\n", i+1); continue;
        }
        printf("Listening for C%d on port %d\n", i+1, PORTS[i]);
    }

    fd_set readfds;
    int max_sd;

    while(1){
        FD_ZERO(&readfds);
        max_sd = 0;
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        // Add server sockets and client sockets to the fd set
        for(int i=0;i<MAX_CLIENTS;i++){
            FD_SET(server_sockets[i], &readfds);
            if(server_sockets[i] > max_sd) max_sd = server_sockets[i];
            if(client_sockets[i] > 0){
                FD_SET(client_sockets[i], &readfds);
                if(client_sockets[i] > max_sd) max_sd = client_sockets[i];
            }
        }

        if(select(max_sd+1,&readfds,NULL,NULL,NULL) < 0) break;

        for(int i=0;i<MAX_CLIENTS;i++){
            // New connection
            if(FD_ISSET(server_sockets[i], &readfds)){
                struct sockaddr_in6 cli_addr; int len = sizeof(cli_addr);
                int new_sock = accept(server_sockets[i], (struct sockaddr*)&cli_addr, &len);
                if(new_sock != INVALID_SOCKET){
                    client_sockets[i] = new_sock;
                    pos[i] = 0;
                    printf("Client C%d connected\n", i+1);
                    send(client_sockets[i], "Connected! Use @<digits> <message>\n", 36, 0);
                }
            }

            // Data from client
            if(client_sockets[i] > 0 && FD_ISSET(client_sockets[i], &readfds)){
                int n = recv(client_sockets[i], buffer, BUFFER_SIZE-1, 0);
                if(n <= 0){
                    printf("Client C%d disconnected\n", i+1);
                    closesocket(client_sockets[i]);
                    client_sockets[i] = 0;
                    pos[i] = 0;
                    continue;
                }

                // Process each byte to handle lines
                for(int b=0;b<n;b++){
                    lines[i][pos[i]++] = buffer[b];

                    if(buffer[b]=='\n' || pos[i]>=BUFFER_SIZE-1){
                        lines[i][pos[i]] = '\0';

                        // Check if it's a message to other clients
                        if(lines[i][0]=='@'){
                            // Find where numbers end and message begins
                            int j = 1;
                            while(j < pos[i] && lines[i][j] != ' ' && lines[i][j] != '\n') j++;
                            int msg_start = j+1;

                            // Track which clients received the message for logging
                            char recipients[BUFFER_SIZE] = "";
                            int rec_count = 0;

                            for(int k=1;k<j;k++){
                                char c = lines[i][k];
                                if(c >= '1' && c <= '4'){
                                    int target = c - '1';
                                    if(client_sockets[target] != 0){
                                        char msg[BUFFER_SIZE];
                                        snprintf(msg, BUFFER_SIZE, "C%d: %s", i+1, lines[i]+msg_start);
                                        send(client_sockets[target], msg, strlen(msg), 0);

                                        // Track for log
                                        if(rec_count > 0) strcat(recipients, ", ");
                                        char temp[8]; sprintf(temp, "C%d", target+1);
                                        strcat(recipients, temp);
                                        rec_count++;
                                    }
                                }
                            }

                            if(rec_count > 0)
                                printf("C%d -> %s: %s", i+1, recipients, lines[i]+msg_start);
                            else
                                printf("C%d sent to no connected clients: %s", i+1, lines[i]+msg_start);

                        } else {
                            // Not starting with @, send error
                            char msg[BUFFER_SIZE];
                            snprintf(msg, BUFFER_SIZE, "Invalid message format. Use @<digits> <message>\n");
                            send(client_sockets[i], msg, strlen(msg), 0);
                            printf("Invalid message from C%d: %s", i+1, lines[i]);
                        }
                        pos[i] = 0; // reset line buffer
                    }
                }
            }
        }
    }

    // Cleanup
    for(int i=0;i<MAX_CLIENTS;i++){
        if(server_sockets[i] > 0) closesocket(server_sockets[i]);
        if(client_sockets[i] > 0) closesocket(client_sockets[i]);
    }
    WSACleanup();
    return 0;
}