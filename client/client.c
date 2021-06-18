#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#define PORT 8080

bool checkClose(int valread, int sock){
    if(valread == 0){
        close(sock);
        return 1;
    }
    return 0;
}

int main(int argc, char const *argv[]) {
    bool mode_input = false;

    if(getuid()){
        if(argc < 5){
            printf("not enough argument passed\n");
            return -1;
        }
        if(strcmp(argv[1],"-u") || strcmp(argv[3],"-p")){
            printf("syntax error\n");
            return -1;
        }
        if(argc == 7){
            if(!strcmp(argv[5],"-d")){
                mode_input = true;
            }else{
                printf("syntax error\n");
                return -1;
            }
        }
    }else{
        if(argc == 3){
            if(!strcmp(argv[1],"-d")){
                mode_input = true;
            }else{
                printf("syntax error\n");
                return -1;
            }
        }
    }

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;

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
    char status_login[1000] = {0};
    int val = recv( sock , status_login, 1000,0);
    printf("%s\n",status_login);
    if(!strcmp(status_login,"authentication failed")){
        close(sock);
        return -1;
    }

    if(getuid()){
        if(mode_input){
            char command[1000] = {0}, receive[1000] = {0};
            sprintf(command,"USE %s",argv[6]);
            send(sock , command , strlen(command) , 0 );
            int val = recv( sock , receive, 1000,0);
            if(!strncmp(receive,"database changed to",19)){
                char temp[1000];
                while((fscanf(stdin,"%[^\n]%*c",temp)) != EOF){
                    send(sock , temp , strlen(temp) , 0 );
                    bzero(receive,sizeof(receive));
                    int val = recv( sock , receive, 1000,0);
                    printf("%s\n",receive);
                };
                close(sock);
                return 0;
            }
        }
    }else{
        if(mode_input){
            char command[1000] = {0}, receive[1000] = {0};
            sprintf(command,"USE %s",argv[2]);
            send(sock , command , strlen(command) , 0 );
            int val = recv( sock , receive, 1000,0);
            if(!strncmp(receive,"database changed to",19)){
                char temp[1000];
                while((fscanf(stdin,"%[^\n]%*c",temp)) != EOF){
                    send(sock , temp , strlen(temp) , 0 );
                    bzero(receive,sizeof(receive));
                    int val = recv( sock , receive, 1000,0);
                    printf("%s\n",receive);
                };
                close(sock);
                return 0;
            }
        }
    }
    while(1){
        char message[1000] ={0};
        char buffer[1024] = {0};
        gets(message);
        send(sock , message , strlen(message) , 0 );
        // printf("Hello message sent\n");
        valread = recv( sock , buffer, 1024,0);
        if(checkClose(valread, sock)){
            break;
        }
        printf("%s\n",buffer );
    }
    return 0;
}