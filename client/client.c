#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8080
  
int main(int argc, char const *argv[]) {

    if(getuid()){
        if(argc < 5){
            printf("not enough argument passed\n");
            return 0;
        }
        if(strcmp(argv[1],"-u") || strcmp(argv[3],"-p")){
            printf("syntax error\n");
            return 0;
        }
    }

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    char tipe[1024];
    if(getuid()){
        strcpy(tipe,argv[2]);
        strcat(tipe," ");
        strcat(tipe,argv[4]);
        send(sock , tipe , strlen(tipe) , 0 );
    }
    else{
        strcpy(tipe,"root");
        send(sock , tipe , strlen(tipe) , 0 );
    }
    while(1){
        char message[1000] ={0};
        gets(message);
        send(sock , message , strlen(message) , 0 );
        // printf("Hello message sent\n");
        valread = recv( sock , buffer, 1024,0);
        printf("%s\n",buffer );
    }
    return 0;
}