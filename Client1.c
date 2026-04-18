#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

#define SERVER_PORT 20000
#define BUFFER_SIZE 1024

int main(int argc,char*argv[]){
    if(argc<2){ printf("Usage: %s <client_number 1-4>\n",argv[0]); return 1; }
    int client_number = atoi(argv[1]);
    if(client_number<1||client_number>4){ printf("Client number must be 1-4\n"); return 1; }

    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);

    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if(sock<0){ perror("socket"); return 1; }

    struct sockaddr_in6 server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(SERVER_PORT);
    server_addr.sin6_addr = in6addr_loopback;

    if(connect(sock,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){ perror("connect"); return 1; }

    // Send client number immediately
    char numbuf[4]; sprintf(numbuf,"%d\n",client_number);
    send(sock,numbuf,strlen(numbuf),0);

    printf("C%d connected. Type messages like @<client_numbers> <message>\n",client_number);
    printf("Type 'quit' to exit.\n");

    char buffer[BUFFER_SIZE];
    fd_set readfds;

    while(1){
        FD_ZERO(&readfds);
        FD_SET(sock,&readfds);
        FD_SET(fileno(stdin),&readfds);
        int maxfd = sock>fileno(stdin)?sock:fileno(stdin);

        if(select(maxfd+1,&readfds,NULL,NULL,NULL)<0) break;

        // Server message
        if(FD_ISSET(sock,&readfds)){
            int n = recv(sock,buffer,BUFFER_SIZE-1,0);
            if(n>0){
                buffer[n]='\0';
                printf("%s",buffer);
            }
            // If n==0, server closed connection; just wait for user quit
        }

        // User input
        if(FD_ISSET(fileno(stdin),&readfds)){
            if(fgets(buffer,BUFFER_SIZE,stdin)==NULL) continue;

            // Remove newline
            buffer[strcspn(buffer,"\r\n")]=0;

            if(strcmp(buffer,"quit")==0) break; // exit loop

            // send message
            strcat(buffer,"\n"); // add newline
            send(sock,buffer,strlen(buffer),0);
        }
    }

    closesocket(sock); WSACleanup();
    printf("Disconnected.\n");
    return 0;
}