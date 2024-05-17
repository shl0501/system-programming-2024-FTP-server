//////////////////////////////////////////////////////////////
// File Name : srv.c                                        //
// Date : 2024/05/14                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #2-3 (server)      //
// Description : open server socket and connect with client //
//               get converted command from client          //
//               fork process and accept multiple client    //
//               send result to client from child process   //
//////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>
#include<dirent.h>

#define MAX_BUF 20

//////////////////////////////////////////////////////////////
// client_info                                              //
// ======================================================== //
// Input : struct sockaddr_in client_addr                   //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : print client port_num and client ip address    //
//////////////////////////////////////////////////////////////
int client_info(struct sockaddr_in client_addr, char*ip){
    write(1, "** Client is trying to connect **\n", 35);
    /********************** client IP address ************************/
    write(1, " - IP : ", 9);
    write(1, ip, strlen(ip));
    write(STDOUT_FILENO, "\n", 2);
    //////////////////////////////////////////////////////////////////

    /********************** client port number ***********************/
    char client_port[100];
    sprintf(client_port, "%d", client_addr.sin_port);
    write(1, " - Port : ", 11);
    write(STDOUT_FILENO, client_port, strlen(client_port));
    write(STDOUT_FILENO, "\n", 2);
    //////////////////////////////////////////////////////////////////
    return 1;
}


int user_match(char*user, char*passwd)
{
    FILE *fp;
    struct passwd *pw;
    fp = fopen("passwd", "r");
    while((pw = fgetpwent(fp))!= NULL) {
        if(!strcmp(user, pw->pw_name)) {
            if(!strcmp(passwd, pw->pw_passwd)) {
                return 1;
            }
        }
    }
    return 0;
}
int log_auth(int connfd)
{
    char user[MAX_BUF], passwd[MAX_BUF];
    int n, count = 1;

    while(1) {
        memset(user, 0, MAX_BUF);
        memset(passwd, 0, MAX_BUF);
    /****************   read username and password form client *************/
        n = read(connfd, user, MAX_BUF);
        user[n-1] = '\0';

        n = read(connfd, passwd, MAX_BUF);
    /***********************************************************************/

        write(connfd, "OK", 3);

        if((n = user_match(user, passwd)) == 1) {
            write(connfd, "OK", 3);
        }

        else if(n == 0) {
            if(count >= 3) {    /* 3 time failed*/

                write(connfd, "DISCONNECTION", 14);
                return 0;
            }
            write(connfd, "FAIL", 5);
            count++;
            continue;
        }
    }
    return 1;
}

//////////////////////////////////////////////////////////////
// Ip_Slicing                                               //
// ======================================================== //
// Input : char *str, char**sliced                          //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : slice ip address by dot(.)                     //
//////////////////////////////////////////////////////////////
void Ip_Slicing(char *str, char**sliced) {
    int num = 0;
    char *ptr = strtok(str, ".");
    while(ptr != NULL) {
        strcpy(sliced[num++], ptr);
        ptr = strtok(NULL, ".");
    }
}
int main(int argc, char*argv[]) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    FILE *fp_checkIP;   //FILE stream to check client's IP

    /********************* prepare server socket and connect with client socket **********************/
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//set address
    servaddr.sin_port = htons(atoi(argv[1]));//set port
        
    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){  //bind socket
        printf("Server: Can't bind local address\n");
        return 0;
    }   

    listen(listenfd, 5);   //listen from client
    /*************************************************************************************************/
    for(;;) {
        int clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
        
        /******************** get ip address from client ******************/
        char client_ip[MAX_BUF];
        int n = read(connfd, client_ip, MAX_BUF);
        client_ip[n] = '\0';
        
        //print information of client//
        client_info(cliaddr, client_ip);
        
        /*****************************************************************/

        /***************** open access.txt ******************/
        fp_checkIP = fopen("access.txt", "r");
        if(fp_checkIP == NULL) {
            printf("Error: cannot open access file\n");
            close(connfd);
            continue;
        }
        /****************************************************/

        char IPs[MAX_BUF];
        int find = 1;

        /**************** check if client ip is acceptable ****************/
        while((fgets(IPs, MAX_BUF, fp_checkIP) != NULL))
        {
            find = 1;
            IPs[strlen(IPs)-1] = '\0';
            
            /************ allocate string array *************/
            char **access_IP = (char**)malloc(sizeof(char*) * 5);
            for(int i = 0; i<5; i++){
            access_IP[i] = (char*)malloc(sizeof(char) * 10);
            }

            char **input_IP = (char**)malloc(sizeof(char*) * 5);
            for(int i = 0; i<5; i++){
            input_IP[i] = (char*)malloc(sizeof(char) * 10);
            }
            /***********************************************/

            /********** Slicing IP address by dot(.) ***********/
            Ip_Slicing(IPs, access_IP);
            Ip_Slicing(client_ip, input_IP);
            /***************************************************/
            
            /***************** Check string *****************/
            for(int i = 0; i<4; i++){
                if(!strcmp(input_IP[i], "*"))   //wildcard
                    continue;
                if(!strcmp(access_IP[i], "*"))  //wildcard
                    continue;
                if(strcmp(access_IP[i], input_IP[i]))   //not equal string
                    find = 0;
            }
            /***********************************************/

            /************ free string array *************/
            for(int i = 0; i<5; i++){
                free(access_IP[i]);
            }
            free(access_IP);
            
            for(int i = 0; i<5; i++){
                free(input_IP[i]);
            }
            free(input_IP);
            /***********************************************/

            /************** client connected ***************/
            if(find) {
                write(1, "** Client is connected **\n", 27);
                write(connfd, "ACCEPTED", 9);
                break;
            }
            /**********************************************/
        }

        /***************    failed to connect   ***************/
        if(!find) {
            write(1, "** It is not authenticated client **\n", 38);
            write(connfd, "REJECTION", 10);
            close(connfd);
            close(listenfd);
            exit(0);
        }
        /******************************************************/

        /******************************************************************/

        if(log_auth(connfd) == 0) {
            printf("** Fail to log-in **\n");
            close(connfd);
            continue;
        }
        printf("** Success to log-in **\n");
        close(connfd);
    }
}