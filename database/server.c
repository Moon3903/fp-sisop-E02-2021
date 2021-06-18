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

char *project_path = "/home/ryan/Desktop/fp-sisop-E02-2021/database";
char *user_table = "/home/ryan/Desktop/fp-sisop-E02-2021/database/databases/administrator/user.txt";
char *permission_table = "/home/ryan/Desktop/fp-sisop-E02-2021/database/databases/administrator/permission.txt";


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

void append_log(char *login_user, char *command){
    FILE *fileout;
    char fpath[1000]={0};
    sprintf(fpath,"%s/dblog.log",project_path);
    fileout = fopen(fpath,"a");

    char time_now[100] = {0};
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	strftime(time_now,100,"%Y-%m-%d %H:%M:%S",tm);
    fprintf(fileout, "%s:%s:%s\n",time_now,login_user,command);
	fclose(fileout);
}

char* trim(char *text){
    int index = 0;
    while(text[index] == ' ' || text[index] == '\t'){
        index++;
    }
    char *temp = strchr(text,text[index]);
    return temp;
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

    char fpath[1000] = {0};
    sprintf(fpath,"%s/databases/%s",project_path,database);
    
    if(mkdir(fpath,0777) == 0){
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

int create_table(char *buffer,char *use_database){
    if(strlen(use_database) == 0){
        return -2;
    }
    char tempBuffer[1024] = {0}, table[100] = {0};
    strcpy(tempBuffer,buffer);
    
    
    char* token = strtok(tempBuffer, ";");
    token = strtok(tempBuffer," ");
    token = strtok(NULL," ");
    token = strtok(NULL," ");

    if(token != NULL){
        strcpy(table,token);
    }else{
        return -1;
    }

    char *temp;
    temp = strchr(buffer,'(');
    if(!temp){
        return -1;
    }else{
        temp = temp + 1;
    }

    char kolom[1000] = {0};
    strcpy(kolom,temp);

    char *token1 = strtok(kolom,";");
    token1 = strtok(kolom,")");

    char split_kolom[100][100] = {0};
    int ind = 0;
    token1 = strtok(kolom,",");
    while(token1!=NULL){
        strcpy(split_kolom[ind++],trim(token1));
        token1 = strtok(NULL,",");
    }

    //cek kelengkapan nama kolom dan tipe kolom
    for(int i = 0; i < ind; i++){
        char temp2[1000] = {0},nama_kolom[100] = {0}, tipe_kolom[20] = {0};
        strcpy(temp2,split_kolom[i]);
        char *token2 = strtok(temp2," ");
        strcpy(nama_kolom,token2);
        token2 = strtok(NULL," ");
        if(token2){
            strcpy(tipe_kolom,token2);
            if(strcmp(tipe_kolom,"int") && strcmp(tipe_kolom,"string")){
                return -4;
            }
        }else{
            return -3;
        }
    }

    char fpath[1000] = {0};
    sprintf(fpath,"%s/databases/%s/%s.txt",project_path,use_database,table);
    FILE *open;
    if(!(open = fopen(fpath,"r"))){
        open = fopen(fpath,"w");
        char struktur_table[1000] = {0};
        sprintf(struktur_table,"%s/databases/%s/struktur_%s.txt",project_path,use_database,table);
        // sprintf(struktur_table,"%s/struktur_%s.txt",use_database,table);

        for(int i = 0; i < ind; i++){
            file(struktur_table,split_kolom[i]);
        }
        return 1;
    }else{
        return 0;
    }
}

void *hapusFolder(void *arg){
    pid_t child;
    child = fork();
    if(child == 0){
        char *fpath = (char *) arg;
        char *argv[] = {"rm","-rf",fpath, NULL};
        execv("/bin/rm",argv);
    }
}

int drop_database(char *buffer, char *tipe, char *login_user, char *use_database){

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

    // if(!strcmp(tipe,"root")){
    //     strcpy(use_database, database);
    //     return 1;
    // }


    FILE *filein, *fileout;
    filein = fopen(permission_table,"r");


    char tmpdatabase[1000],tmpuser[1000];
    bool ada = false,bisa =false;
    if(filein){
        while(fscanf(filein,"%s %s",tmpdatabase,tmpuser) != EOF){
            if(!strcmp(tmpdatabase,database)){
                ada = true;
                if(!strcmp(tmpuser,login_user)){
                    fclose(filein);
                    bisa = true;
                    break;
                }
            }
        }

        if(bisa || (ada && !strcmp(login_user,"root"))){              
            char fpath[1000] = {0};
            sprintf(fpath,"%s/databases/%s",project_path,database);
            pthread_t thread1;
            int iret1 = pthread_create(&thread1,NULL,hapusFolder,fpath);;
            pthread_join(thread1,NULL);
            
            filein = fopen(permission_table,"r");
            char temp_permission[1000] = {0};
            sprintf(temp_permission,"%s/databases/administrator/temp.txt",project_path);
            fileout = fopen(temp_permission,"w");
            while(fscanf(filein,"%s %s",tmpdatabase,tmpuser) != EOF){
                if(strcmp(tmpdatabase,database)){
                    char record[1000] = {0};
                    fprintf(fileout, "%s %s\n", tmpdatabase, tmpuser);
                }
            }
            fclose(fileout);
            remove(permission_table);
            rename(temp_permission,permission_table);

            if(!strcmp(use_database,database)){
                bzero(use_database,sizeof(use_database));
            }
            return 1;
        }        
    }
    if(!ada){
        return -2;
    }
    return 0;
}

int validasi(char *buffer){
    char tmp[1000];
    strcpy(tmp,buffer);
    char g = '\'';

    if(tmp[0]== g && tmp[strlen(tmp)-1] == g){
        return 1;
    }
    for(int i = 0; i<strlen(tmp) ;i++){
        if(tmp[i] < '0' || tmp[i] > '9'){
            return 0;
        }
    }
    return 2;
}

int insert(char *buffer, char *use_database){

    if(!strlen(use_database)){
        return -1;
    }    

    char tmp[1000];
    strcpy(tmp,buffer);

    char *token = strtok(tmp,"(");
    token = strtok(NULL,"(");
    token = strtok(token,")");
    token = strtok(token,",");

    char data[100][1000];

    int i = 0;

    while(token!=NULL){
        strcpy(data[i],trim(token));
        i++;
        token = strtok(NULL,",");
    }

    FILE *filein,*fileout;

    strcpy(tmp,buffer);
    token = strtok(tmp," ");
    token = strtok(NULL," ");
    token = strtok(NULL," ");

    char open[1000] = {0},append[1000] = {0};
    sprintf(open,"%s/databases/%s/struktur_%s.txt",project_path,use_database,token);
    // stcpy(open,use_database);
    // strcat(open,"/struktur_");
    // strcat(open,token);
    // strcat(open,".txt");
    filein = fopen(open,"r");

   
    char data_type[100][1000];
    char tmp_type[1000],ret[1000];
    int k = 0;

    if(filein){
        while(fscanf(filein,"%s %s",ret,tmp_type) != EOF){
            strcpy(data_type[k],tmp_type);
            k++;
        }
    }
    else{
        return -2;
    }

    sprintf(append,"%s/databases/%s/%s.txt",project_path,use_database,token);
    // strcpy(append,use_database);
    // strcat(append,"/");
    // strcat(append,token);
    // strcat(append,".txt");
    fileout = fopen(append,"a");

    if(k != i){
        fclose(filein);
        fclose(fileout);
        return -3;
    }

    for(int j=0;j<i;j++){
        int val = validasi(data[j]);
        if(val == 1 && !strcmp(data_type[j],"string")) ;
        else if (val == 2 && !strcmp(data_type[j],"int")) ;
        else {
            fclose(filein);
            fclose(fileout);
            return -4;
        }  
    }
    for(int j=0;j<i-1;j++){
        fprintf(fileout,"%s,",data[j]);
    }
    fprintf(fileout,"%s\n",data[i-1]);
    fclose(filein);
    fclose(fileout);

    return 1;
}

int drop_table(char *buffer, char *use_database){

    if(!strlen(use_database)){
        return -2;
    }

    FILE *filein;

    char tmp[1000];
    char *token;
    strcpy(tmp,buffer);
    token = strtok(tmp,";");
    token = strtok(token," ");
    token = strtok(NULL," ");
    token = strtok(NULL," ");

    char open[1000] = {0},append[1000]={0};
    sprintf(open,"%s/databases/%s/struktur_%s.txt",project_path,use_database,token);
    // strcpy(open,use_database);
    // strcat(open,"/struktur_");
    // strcat(open,token);
    // strcat(open,".txt");
    filein = fopen(open,"r");

    sprintf(append,"%s/databases/%s/%s.txt",project_path,use_database,token);
    // strcpy(append,use_database);
    // strcat(append,"/");
    // strcat(append,token);
    // strcat(append,".txt");

    if(filein){
        fclose(filein);
        remove(open);
        remove(append);
        return 1;
    }
    else{
        return -1;
    }
}

int drop_column(char *buffer, char *use_database){

    if(!strlen(use_database)){
        return -2;
    }

    char input[10][1000];
    char tmp[10000];
    strcpy(tmp,buffer);
    int k = 0;
    char *token;
    token = strtok(tmp,";");
    token = strtok(token," ");
    while (token != NULL) {
        strcpy(input[k++],token);
        token = strtok(NULL, " ");
    }

    if(k != 5){
        return -3;
    }

    FILE *strukturin,*strukturout
        ,*tablein,*tableout;

    char open[1000]={0},append[1000]={0},a[1000]={0},b[1000]={0};
    sprintf(open,"%s/databases/%s/struktur_%s",project_path,use_database,input[4]);
    strcpy(a,open);
    strcat(a,"2.txt");
    strcat(open,".txt");

    // strcpy(open,use_database);
    // strcat(open,"/struktur_");
    // strcat(open,input[4]);
    // strcpy(a,open);
    // strcat(a,"2.txt");
    // strcat(open,".txt");
    strukturin = fopen(open,"r");
   
    int i = 0,j = 0;
    char data_type[1000], name[1000];

    if(strukturin){
        strukturout = fopen(a,"w");
        while(fscanf(strukturin,"%s %s",name,data_type) != EOF){
            if(strcmp(input[2],name)){
                fprintf(strukturout,"%s %s\n",name,data_type);
            }
            else i = j;
            j++;
        }
        fclose(strukturin);
        fclose(strukturout);
    }
    else{
        return -1;
    }

    sprintf(append,"%s/databases/%s/%s",project_path,use_database,input[4]);
    strcpy(b,append);
    strcat(b,"2.txt");
    strcat(append,".txt");

    // strcpy(append,use_database);
    // strcat(append,"/");
    // strcat(append,input[4]);
    // strcpy(b,append);
    // strcat(b,"2.txt");
    // strcat(append,".txt");
    tablein = fopen(append,"r");
    tableout = fopen(b,"w");

    char ambil[1000];
    while(fgets(ambil,1000,tablein)){
        token = strtok(ambil,",");
        int j = 0;
        char baru[1000];
        strcpy(baru,"");
        while(token!=NULL){
            if(j!=i){
                strcat(baru,token);
                strcat(baru,",");
            }
            token = strtok(NULL,",");
            j++;
        }
        baru[strlen(baru)-1] = 0;
        fprintf(tableout,"%s",baru);
        if(baru[strlen(baru)-1] != '\n'){
            fprintf(tableout,"\n");
        }
    }
    fclose(tablein);
    fclose(tableout);
    remove(open);
    rename(a,open);
    remove(append);
    rename(b,append);
    return 1;
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

int delete_from(char *buffer,char *use_database){

    if(!strlen(use_database)){
        return -1;
    }

    char tempBuffer[1024] = {0};
    strcpy(tempBuffer,buffer);
   
    char* token = strtok(tempBuffer, ";");
    token = strtok(tempBuffer," ");
    char input[10][1000];
    int i=0;

    while (token != NULL) {
        strcpy(input[i++],token);
        token = strtok(NULL, " ");
    }

    if(i == 3){
        char path[10000];
        sprintf(path,"%s/databases/%s/%s.txt",project_path,use_database,input[2]);
        FILE *filein,*fileout;
        filein = fopen(path,"r");
        if(filein){
            fclose(filein);
            fileout = fopen(path,"w");
            fprintf(fileout,"");
            fclose(fileout);
        }
        else{
            return -3;
        }
        return 1;
    }
    else if(i == 5){
        if(strcmp(input[3],"WHERE")){
            return -2;
        }

        char table_name[1000], cmp[1000];

        token = strtok(input[4],"=");
        strcpy(table_name,token);
        token = strtok(NULL,"=");
        strcpy(cmp,token);

        char path[10000],struk[10000],n_path[10000];

        sprintf(struk,"%s/databases/%s/struktur_%s.txt",project_path,use_database,input[2]);
        FILE *strukturin;
        strukturin = fopen(struk,"r");

        int i = -1;
        int k = 0;
        char ret[1000],table[1000];
        if(strukturin){
            while(fscanf(strukturin,"%s %s",table,ret) != EOF){
                if(!strcmp(table_name,table)){
                    i = k;
                }
                k++;
            }
            if(i == -1){
                fclose(strukturin);
                return -4;
            }
            fclose(strukturin);
        }
        else{
            return -3;
        }

        sprintf(path,"%s/databases/%s/%s.txt",project_path,use_database,input[2]);
        sprintf(n_path,"%s/databases/%s/%s2.txt",project_path,use_database,input[2]);
        FILE *filein,*fileout;
        filein = fopen(path,"r");
        fileout = fopen(n_path,"w");
        char ambil[1000];
        if(filein){
            while(fgets(ambil,1000,filein)){
                token = strtok(ambil,",");
                int j = 0;
                char baru[1000];
                strcpy(baru,"");
                int flag = 0;
                while(token!=NULL){
                    strcat(baru,token);
                    strcat(baru,",");
                    if(j==i){
                        // printf("%s %s\n",cmp,token);
                        if(!strcmp(cmp,token)){
                            flag = 1;
                        }
                    }
                    token = strtok(NULL,",");
                    j++;
                }
                if(!flag){
                    baru[strlen(baru)-1] = 0;
                    // fprintf(fileout,"%s",baru);
                    if(baru[strlen(baru)-1] != '\n'){
                        fprintf(fileout,"\n");
                    }
                }
            }
            fclose(filein);
            fclose(fileout);
            remove(path);
            rename(n_path,path);
            return 1;
        }
        else{
            return -3;
        }
    }
    else{
        return -2;
    }
}

int update(char *buffer,char *use_database){
    
    if(!strlen(use_database)){
        return -1;
    }

    char tempBuffer[1024] = {0};
    strcpy(tempBuffer,buffer);
   
    char* token = strtok(tempBuffer, ";");
    token = strtok(tempBuffer," ");
    char input[10][1000];
    int i=0;

    while (token != NULL) {
        strcpy(input[i++],token);
        token = strtok(NULL, " ");
    }

    if(strcmp(input[2],"SET")){
        return -2;
    }

    char table_name[1000], cmp[1000];

    token = strtok(input[3],"=");
    strcpy(table_name,token);
    token = strtok(NULL,"=");
    strcpy(cmp,token);

    char path[10000],struk[10000],n_path[10000];

    sprintf(struk,"%s/databases/%s/struktur_%s.txt",project_path,use_database,input[1]);
    FILE *strukturin;
    strukturin = fopen(struk,"r");

    i = -1;
    int k = 0;
    char ret[1000],table[1000];
    if(strukturin){
        while(fscanf(strukturin,"%s %s",table,ret) != EOF){
            if(!strcmp(table_name,table)){
                i = k;
            }
            k++;
        }
        if(i == -1){
            fclose(strukturin);
            return -4;
        }
        fclose(strukturin);
    }
    else{
        return -3;
    }

    sprintf(path,"%s/databases/%s/%s.txt",project_path,use_database,input[1]);
    sprintf(n_path,"%s/databases/%s/%s2.txt",project_path,use_database,input[1]);
    FILE *filein,*fileout;
    filein = fopen(path,"r");
    fileout = fopen(n_path,"w");
    char ambil[1000];
    if(filein){
        while(fgets(ambil,1000,filein)){
            token = strtok(ambil,",");
            int j = 0;
            char baru[1000];
            strcpy(baru,"");
            int flag = 0;
            while(token!=NULL){
                
                if(j==i){
                    // printf("%s %s\n",cmp,token);
                    strcat(baru,cmp);
                    strcat(baru,",");
                }
                else{
                    strcat(baru,token);
                    strcat(baru,",");
                }
                token = strtok(NULL,",");
                j++;
            }
            baru[strlen(baru)-1] = 0;
            fprintf(fileout,"%s",baru);
            if(baru[strlen(baru)-1] != '\n'){
                fprintf(fileout,"\n");
            }
        }
        fclose(filein);
        fclose(fileout);
        remove(path);
        rename(n_path,path);
        return 1;
    }
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
        char status_login[1000] = {0};
        strcpy(status_login,"authentication success");
        send(*new_socket , status_login , strlen(status_login) , 0 );
    }
    else{
        //cek login
        printf("%s\n",tipe);
        int masuk = login(tipe,login_user);
        char status_login[1000] = {0};
        
        if(masuk == 0){
            strcpy(status_login,"authentication failed");
            send(*new_socket , status_login , strlen(status_login) , 0 );
            printf("login gagal\n");
            close(*new_socket);
            return;
        }
        strcpy(status_login,"authentication success");
        send(*new_socket , status_login , strlen(status_login) , 0 );

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
            append_log(login_user, buffer);
            int status = create_user(buffer,tipe);
            if(status == -2){
                strcpy(message,"user already exist");
            }else if(status == -1){
                strcpy(message,"syntax error");
            }else if(status == 0){
                strcpy(message,"permission denied");
            }else if(status == 1){
                strcpy(message,"create user success");
            }
        }else if(!strncmp(buffer,"USE",3)){
            append_log(login_user, buffer);
            int status = use(buffer,tipe,login_user,use_database);
            if(status == -2){
                strcpy(message,"unknown database");
            }else if(status == -1){
                strcpy(message,"syntax error");
            }else if(status == 1){
                sprintf(message,"database changed to %s",use_database);
            }else if(status == 0){
                strcpy(message,"permission denied");
            }
        }else if(!strncmp(buffer,"CREATE DATABASE",15)){
            append_log(login_user, buffer);
            int status = create_database(buffer,tipe,login_user);
            if(status == -1){
                strcpy(message,"syntax error");
            }else if(status == 1){
                strcpy(message,"create success");
            }else if(status == 0){
                strcpy(message,"permission denied");
            }
        }else if(!strncmp(buffer,"GRANT PERMISSION",16)){
            append_log(login_user, buffer);
            int status = grant_pemission(buffer,tipe);
            if(status == -3){
                strcpy(message,"unknown user");
            }else if(status == -2){
                strcpy(message,"database not exist");
            }else if(status == -1){
                strcpy(message,"syntax error");
            }else if(status == 1){
                strcpy(message,"grant permission success");
            }else if(status == 0){
                strcpy(message,"grant permission denied");
            }
        }else if(!strncmp(buffer,"CREATE TABLE",12)){
            append_log(login_user, buffer);
            int status = create_table(buffer,use_database);
            if(status == -4){
                strcpy(message,"invalid column type");
            }else if(status == -3){
                strcpy(message,"missing column type");
            }else if(status == -2){
                strcpy(message,"no database used");
            }else if(status == -1){
                strcpy(message,"syntax error");
            }else if(status == 1){
                strcpy(message,"create success");
            }else if(status == 0){
                strcpy(message,"table already exist");
            }
        }else if(!strncmp(buffer,"DROP DATABASE",13)){
            append_log(login_user, buffer);
            int status = drop_database(buffer,tipe,login_user,use_database);
            if(status == -2){
                strcpy(message,"unknown database");
            }else if(status == -1){
                strcpy(message,"syntax error");
            }else if(status == 1){
                sprintf(message,"database dropped");
            }else if(status == 0){
                strcpy(message,"permission denied");
            }
        }else if(!strncmp(buffer,"INSERT INTO",11)){
            append_log(login_user, buffer);
            int status = insert(buffer,use_database);
            switch (status) {
                case 1:
                    strcpy(message,"Insert success");
                    break;
                case -1:
                    strcpy(message,"no database used");
                    break;
                case -2:
                    strcpy(message,"table does not exist");
                    break;
                case -3:
                    strcpy(message,"coloumn count doesnt match");
                    break;
                case -4:
                    strcpy(message,"invalid input");
                    break;
            }
        }else if(!strncmp(buffer,"DROP TABLE",10)){
            append_log(login_user, buffer);
            int status = drop_table(buffer,use_database);
            switch (status) {
                case 1:
                    strcpy(message,"drop table success");
                    break;
                case -1:
                    strcpy(message,"table does not exist");
                    break;
                case -2:
                    strcpy(message,"no database used");
                    break;
            }
        }else if(!strncmp(buffer,"DROP COLUMN",11)){
            append_log(login_user, buffer);
            int status = drop_column(buffer,use_database);
            switch (status) {
                case 1:
                    strcpy(message,"drop column success");
                    break;
                case -1:
                    strcpy(message,"table does not exist");
                    break;
                case -2:
                    strcpy(message,"no database used");
                    break;
                case -3:
                    strcpy(message,"invalid syntax");
                    break;
                default:
                    strcpy(message,"duh");
                    break;
            }
        }else if(!strncmp(buffer,"DELETE FROM",11)){
            int status = delete_from(buffer,use_database);
            switch(status){
                case 1:
                    strcpy(message,"delete success");
                    break;
                case -1:
                    strcpy(message,"no database used");
                    break;
                case -2:
                    strcpy(message,"invalid syntax");
                    break;
                case -3:
                    strcpy(message,"table does not exist");
                    break;
                case -4:
                    strcpy(message,"column does not exist");
                    break;
                default:
                    strcpy(message,"duh");
                    break;
            }
        }else if(!strncmp(buffer,"UPDATE",6)){
            int status = update(buffer,use_database);
            switch(status){
                case 1:
                    strcpy(message,"update success");
                    break;
                case -1:
                    strcpy(message,"no database used");
                    break;
                case -2:
                    strcpy(message,"invalid syntax");
                    break;
                case -3:
                    strcpy(message,"table does not exist");
                    break;
                case -4:
                    strcpy(message,"column does not exist");
                    break;
                default:
                    strcpy(message,"duh");
                    break;
            }
        }else{
            append_log(login_user, buffer);
            strcpy(message,"syntax error");
        }

        //debug user dan database yg digunakan
        if(!strcmp(buffer,"STATUS")){
            sprintf(message,"login_user:%s use_database:%s",login_user,use_database);
        } 

        send(*new_socket , message , strlen(message) , 0 );

        
    }
}

int main(int argc, char const *argv[]) {
    char dbpath[1000] = {0}, adminpath[1000] = {0};
    sprintf(dbpath,"%s/databases",project_path);
    sprintf(adminpath,"%s/administrator",dbpath);

    mkdir(dbpath,0777);
    mkdir(adminpath,0777);
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