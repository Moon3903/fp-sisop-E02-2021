#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define PORT 8080
#define MAX_CLIENTS 1000

pthread_t tid[MAX_CLIENTS];

char *user = "user.txt";

bool checkClose(int valread, int *new_socket){
    if(valread == 0){
        close(*new_socket);
        return 1;
    }
    return 0;
}

void file(char path[1000],char tofile[1000]){
    FILE* ptr = fopen(path,"a");
    fprintf(ptr,"%s\n",tofile);
    fclose(ptr);
}

void create(char *buffer,char *tipe){
    char* token = strtok(buffer, " ");
    char input[10][1000];
    int i=0;

    if(strcmp(tipe,"root")){
        printf("permission denied\n");
        return;
    }

    while (token != NULL) {
        strcpy(input[i++],token);
        token = strtok(NULL, " ");
    }

    if(!(!strcmp("IDENTIFIED",input[3]) && !strcmp("BY",input[4]))){
        printf("syntax error\n");
        return;
    }

    char tmp[1000];
    strcpy(tmp,input[2]);
    strcat(tmp," ");
    strcat(tmp,input[5]);
    file(user,tmp);
}

int login(char* tipe){
   
    FILE *filein;
    filein = fopen(user,"r");
    printf("%s\n",tipe);
    char user[1000],pass[1000];

    char* token = strtok(tipe, " ");
    strcpy(user,token);
    token = strtok(NULL, " ");
    strcpy(pass,token);

    char tmpuser[1000],tmppass[1000];

    while(fscanf(filein,"%s %s",tmpuser,tmppass) != EOF){
        if(!strcmp(tmpuser,user) && !strcmp(tmppass,pass)){
            fclose(filein);
            return 1;
        }
    }
    printf("HADE\n");
    fclose(filein);
    return 0;
    
}

void *play(void *arg){
    int *new_socket = (int *) arg;
    int valread;
    char tipe[1024] = {0};
    valread = recv( *new_socket , tipe, 1024, 0);
    printf("?\n");
    if(!strcmp(tipe,"root")){
        printf("ini root\n");
    }
    else{
        //cek login
        printf("%s\n",tipe);
        int masuk = login(tipe);
        if(masuk == 0){
            printf("login gagal\n");
            close(new_socket);
            return;
        }
        printf("berhasil login\n");
    }
    while(1){
        char buffer[1024] = {0};
        char *hello = "Hello from server";

        valread = recv( *new_socket , buffer, 1024, 0);
        if(!strncmp(buffer,"CREATE USER",11)){
            printf("masuk\n");
            create(buffer,tipe);
        }
        printf("%s\n",buffer );

        send(*new_socket , hello , strlen(hello) , 0 );
        printf("Hello message sent\n");
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket[MAX_CLIENTS];
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    
      
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int ctr = 0;
    while(1){
        if ((new_socket[ctr] = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept\n");
            exit(EXIT_FAILURE);
        }
        pthread_create(&(tid[ctr]),NULL,play,&new_socket[ctr]);
        ctr++;
        printf("Client %d terhubung\n", ctr);
    }
    return 0;
}