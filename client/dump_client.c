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
    if(getuid()){
        if(argc < 6){
            printf("syntax error\n");
            return -1;
        }else{
            if(strcmp(argv[1],"-u") || strcmp(argv[3],"-p")){
                return -1;
            }
        }
    }else{
        if(argc < 2){
            printf("syntax error\n");
            return -1;
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
        strcat(tipe," dump ");
        send(sock , tipe , strlen(tipe) , 0 );
    }
    else{
        strcpy(tipe,"root dump ");
        send(sock , tipe , strlen(tipe) , 0 );
    }
    char status_login[1000] = {0};
    int val = recv( sock , status_login, 1000,0);
    // printf("%s\n",status_login);
    if(!strcmp(status_login,"authentication failed")){
        close(sock);
        return -1;
    }

    if(getuid()){
        char command[1000] = {0}, receive[1000] = {0};
        sprintf(command,"USE %s",argv[5]);
        send(sock , command , strlen(command) , 0 );
        int val = recv( sock , receive, 1000,0);
        // printf("%s\n",receive);
        bzero(command,sizeof(command));
        strcpy(command,"ok");
        send(sock , command , strlen(command) , 0 );
        if(!strncmp(receive,"database changed to",19)){
            do{
                bzero(receive,sizeof(receive));
                int val = recv( sock , receive, 1000,0);
                if(strcmp(receive,"DONE!!!")){
                    printf("%s\n",receive);
                    bzero(command,sizeof(command));
                    strcpy(command,"ok");
                    send(sock , command , strlen(command) , 0 );
                }
            }while(strcmp(receive,"DONE!!!"));
            
            close(sock);
            return 0;
        }
    }else{
        char command[1000] = {0}, receive[1000] = {0};
        sprintf(command,"USE %s",argv[1]);
        send(sock , command , strlen(command) , 0 );
        int val = recv( sock , receive, 1000,0);
        // printf("%s\n",receive);
        bzero(command,sizeof(command));
        strcpy(command,"ok");
        send(sock , command , strlen(command) , 0 );
        if(!strncmp(receive,"database changed to",19)){
            do{
                bzero(receive,sizeof(receive));
                int val = recv( sock , receive, 1000,0);
                if(strcmp(receive,"DONE!!!")){
                    printf("%s\n",receive);
                    bzero(command,sizeof(command));
                    strcpy(command,"ok");
                    send(sock , command , strlen(command) , 0 );
                }
            }while(strcmp(receive,"DONE!!!"));
            
            close(sock);
            return 0;
        }
    }   
    return 0;
}