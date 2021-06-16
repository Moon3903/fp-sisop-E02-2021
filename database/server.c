#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CLIENTS 1000

pthread_t tid[MAX_CLIENTS];

char *user_table = "administrator/user.txt";
char *permission_table = "administrator/permission.txt";


bool checkClose(int valread, int *new_socket){
    if(valread == 0){
        close(*new_socket);
        return 1;
    }
    return 0;
}

bool check_database(char *database){
    FILE *filein;
    filein = fopen(permission_table,"r");


    char tmpdatabase[1000],tmpuser[1000];

    if(filein){
        while(fscanf(filein,"%s %s",tmpdatabase,tmpuser) != EOF){
            if(!strcmp(tmpdatabase,database)){
                return 1;
            }
        }
    }
    return 0;
}

bool check_user(char *user){
    FILE *filein;
    filein = fopen(user_table,"r");


    char tempuser[1000],tmppass[1000];

    if(filein){
        while(fscanf(filein,"%s %s",tempuser,tmppass) != EOF){
            if(!strcmp(tempuser,user)){
                return 1;
            }
        }
    }
    return 0;
}

void file(char path[1000],char tofile[1000]){
    FILE* ptr = fopen(path,"a");
    fprintf(ptr,"%s\n",tofile);
    fclose(ptr);
}

int create_user(char *buffer,char *tipe){
    char tempBuffer[1024] = {0};
    strcpy(tempBuffer,buffer);
   
    char* token = strtok(tempBuffer, ";");
    token = strtok(tempBuffer," ");
    char input[10][1000];
    int i=0;

    if(strcmp(tipe,"root")){
        return 0;
    }

    while (token != NULL) {
        strcpy(input[i++],token);
        token = strtok(NULL, " ");
    }

    if(strcmp("IDENTIFIED",input[3]) || strcmp("BY",input[4]) || buffer[strlen(buffer)-1] != ';'){
        return -1;
    }

    if(i < 6){
        return -1;
    }

    if(!check_user(input[2]) && strcmp(input[2],"root") != 0){
        char tmp[1000];
        strcpy(tmp,input[2]);
        strcat(tmp," ");
        strcat(tmp,input[5]);
        file(user_table,tmp);
        return 1;
    }else {
        return -2;
    }
}

int use(char *buffer, char *tipe, char *login_user, char *use_database){
    char tempBuffer[1024] = {0}, database[100] = {0};
    strcpy(tempBuffer,buffer);
    
    
    char* token = strtok(tempBuffer, ";");
    token = strtok(tempBuffer," ");
    token = strtok(NULL," ");
    
    if(token != NULL){
        strcpy(database,token);
    }else{
        return -1;
    }

    if(!strcmp(tipe,"root")){
        strcpy(use_database, database);
        return 1;
    }


    FILE *filein;
    filein = fopen(permission_table,"r");


    char tmpdatabase[1000],tmpuser[1000];
    bool ada = false;
    if(filein){
        while(fscanf(filein,"%s %s",tmpdatabase,tmpuser) != EOF){
            if(!strcmp(tmpdatabase,database)){
                ada = true;
                if(!strcmp(tmpuser,login_user)){
                    fclose(filein);
                    strcpy(use_database, database);
                    return 1;
                }
            }
        }
    }
    if(!ada){
        return -2;
    }
    return 0;
}

int create_database(char *buffer, char *tipe, char *login_user){
    char tempBuffer[1024] = {0}, database[100] = {0};
    strcpy(tempBuffer,buffer);
    
    
    char* token = strtok(tempBuffer, ";");
    token = strtok(tempBuffer," ");
    token = strtok(NULL," ");
    token = strtok(NULL," ");

    if(token != NULL){
        strcpy(database,token);
    }else{
        return -1;
    }

    if(mkdir(database,0777) == 0){
        char record[1000] = {0};
        sprintf(record,"%s %s", database, login_user);
        file(permission_table,record);      
        return 1;
    }else{
        return 0;
    }

}

int grant_pemission(char *buffer, char *tipe){
    char tempBuffer[1024] = {0};
    strcpy(tempBuffer,buffer);
   
    char* token = strtok(tempBuffer, ";");
    token = strtok(tempBuffer," ");
    char input[10][1000];
    int i=0;

    if(strcmp(tipe,"root")){
        return 0;
    }

    while (token != NULL) {
        strcpy(input[i++],token);
        token = strtok(NULL, " ");
    }

    if(strcmp("INTO",input[3]) || buffer[strlen(buffer)-1] != ';'){
        return -1;
    }

    char tmp[1000] = {0};
    if(i < 5){
        return -1;
    }
    if(check_database(input[2])){
        if(check_user(input[4])){
            sprintf(tmp, "%s %s", input[2], input[4]);
            
            file(permission_table,tmp);
            return 1;
        }else{
            return -3;
        }
    }else{
        return -2;
    }
}



int login(char* tipe, char *login_user){
   
    FILE *filein;
    filein = fopen(user_table,"r");
    printf("%s\n",tipe);
    char user[1000],pass[1000];

    char* token = strtok(tipe, " ");
    strcpy(user,token);
    token = strtok(NULL, " ");
    strcpy(pass,token);

    char tmpuser[1000],tmppass[1000];
    if(filein){
        while(fscanf(filein,"%s %s",tmpuser,tmppass) != EOF){
            if(!strcmp(tmpuser,user) && !strcmp(tmppass,pass)){
                fclose(filein);
                strcpy(login_user,user);
                return 1;
            }
        }
        fclose(filein);
    }

    return 0;
    
}

void *play(void *arg){
    int *new_socket = (int *) arg;
    int valread;
    char tipe[1024] = {0};
    valread = recv( *new_socket , tipe, 1024, 0);
    char login_user[100] = {0}, use_database[100]={0};
    printf("?\n");
    if(!strcmp(tipe,"root")){
        printf("ini root\n");
        strcpy(login_user,"root");
    }
    else{
        //cek login
        printf("%s\n",tipe);
        int masuk = login(tipe,login_user);
        if(masuk == 0){
            printf("login gagal\n");
            close(*new_socket);
            return;
        }

        printf("berhasil login%s\n",login_user);
    }
    while(1){
        char buffer[1024] = {0};
        char message[1024] = {0};
        char *hello = "Hello from server";

        valread = recv( *new_socket , buffer, 1024, 0);
        printf("%s\n", buffer);

        if(checkClose(valread,new_socket)){
            printf("Koneksi terputus\n");
            break;
        }

        if(!strncmp(buffer,"CREATE USER",11)){
            printf("masuk\n");
            int status = create_user(buffer,tipe);
            if(status == -2){
                strcpy(message,"user already exist");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == -1){
                strcpy(message,"syntax error");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == 0){
                strcpy(message,"permission denied");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == 1){
                strcpy(message,"create user success");
                send(*new_socket , message , strlen(message) , 0 );
            }
        }else if(!strncmp(buffer,"USE",3)){
            int status = use(buffer,tipe,login_user,use_database);
            if(status == -2){
                strcpy(message,"unknown database");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == -1){
                strcpy(message,"syntax error");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == 1){
                sprintf(message,"database changed to %s",use_database);
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == 0){
                strcpy(message,"permission denied");
                send(*new_socket , message , strlen(message) , 0 );
            }
        }else if(!strncmp(buffer,"CREATE DATABASE",15)){
            int status = create_database(buffer,tipe,login_user);
            if(status == -1){
                strcpy(message,"syntax error");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == 1){
                strcpy(message,"create success");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == 0){
                strcpy(message,"permission denied");
                send(*new_socket , message , strlen(message) , 0 );
            }
        }else if(!strncmp(buffer,"GRANT PERMISSION",16)){
            int status = grant_pemission(buffer,tipe);
            if(status == -3){
                strcpy(message,"unknown user");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == -2){
                strcpy(message,"database not exist");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == -1){
                strcpy(message,"syntax error");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == 1){
                strcpy(message,"grant permission success");
                send(*new_socket , message , strlen(message) , 0 );
            }else if(status == 0){
                strcpy(message,"grant permission denied");
                send(*new_socket , message , strlen(message) , 0 );
            }
        }



        if(!strcmp(buffer,"STATUS")){
            sprintf(message,"login_user:%s use_database:%s",login_user,use_database);
            send(*new_socket , message , strlen(message) , 0 );
        }      

        
    }
}

int main(int argc, char const *argv[]) {
    mkdir("administrator",0777);
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