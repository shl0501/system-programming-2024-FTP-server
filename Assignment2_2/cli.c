//////////////////////////////////////////////////////////////
// File Name : cli.c                                        //
// Date : 2024/05/01                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #2-1 (client)      //
// Description : open client socket and connect with server //
//               get command from client                    //
//               convert ftp server and send it to server   //
//////////////////////////////////////////////////////////////
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>

#define MAX_BUFF 4096
#define RCV_BUFF 4096

//////////////////////////////////////////////////////////////
// conv_cmd                                                 //
// ======================================================== //
// Input : char*buff                                        //
//         char*cmd_buff                                    //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : convert command to FTP command for server      //
//////////////////////////////////////////////////////////////
int conv_cmd(char*buff, char*cmd_buff){
    memset(cmd_buff, 0, sizeof(cmd_buff));
    char command[30];
    int i = 0;
    /*                 seperate command from buffer         */
    for(i = 0; i<strlen(buff); i++){
        if(buff[i] != ' ')
        {
            command[i] = buff[i];
            command[i+1] = '\0';
        }
        else{
            break;
        }
    }
    /////             finish seperating command         ///////

    if(!strcmp("quit", command)){   //case of quit
        strcpy(cmd_buff, "QUIT");
        return 1;
    }
    if(!strcmp("ls", command)){      // case of ls
        
        /*       change ls to NLST        */
        cmd_buff[0] = 'N';
        cmd_buff[1] = 'L';
        cmd_buff[2] = 'S';
        cmd_buff[3] = 'T';
        cmd_buff[4] = '\0'; 
        
        /*              add option directory            */
        int j = 4;
        for(j = 2; j<strlen(buff); j++){
            cmd_buff[j+2] = buff[j];
            cmd_buff[j+2+1] = '\0';
        }
        return 1;
    }
    return -1;
}

//////////////////////////////////////////////////////////////
// process result                                           //
// ======================================================== //
// Input : char*rcv_buff                                    //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : get result from server and process it          //
//////////////////////////////////////////////////////////////
void process_result(char *rcv_buff){
    write(STDOUT_FILENO, rcv_buff, strlen(rcv_buff));
    write(1, "\n", 2);
}

//////////////////////////////////////////////////////////////
// main                                                     //
// ======================================================== //
// Input : char*rcv_buff                                    //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : open client socket and connect to server       //
//////////////////////////////////////////////////////////////
int main(int argc, char**argv){
    char buff[MAX_BUFF], cmd_buff[MAX_BUFF], rcv_buff[RCV_BUFF];
    int n;

    /*                         open socket and connect to server                    */
    int sockfd, len;
    struct sockaddr_in server_addr;
    char *haddr = (char*)malloc(sizeof(char)*100);
    strcpy(haddr, argv[1]);
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        printf("can't create socket\n");
        return -1;
    }
    memset(buff, 0, sizeof(buff));
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(haddr);     //set ip number
    int portno = atoi(argv[2]);                         //set port number
    server_addr.sin_port = htons(portno);
    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){   //connect with server socket
        printf("Can't connect\n");
        return -1;
    }
    //////////  Finish opening socket and connecting to server          ///////////


    /*                               read and write buffer with server                              */
    for(;;){
        memset(buff, 0, sizeof(buff));
        memset(cmd_buff, 0, sizeof(cmd_buff));
        memset(rcv_buff, 0, sizeof(rcv_buff));
        int len = read(STDIN_FILENO, buff, sizeof(buff));
        buff[strlen(buff)-1] = '\0';
        if(conv_cmd(buff, cmd_buff) < 0){
        /* convert ls (including) options to NLST (including options) */
            write(STDERR_FILENO, "conv_cmd() error!!\n", 20);
            exit(1);
        }

        n = strlen(cmd_buff);
        /*              write to server socket                      */
        if(write(sockfd, cmd_buff, n) != n){
            write(STDERR_FILENO, "write() error\n", 15);
            exit(1);
        }
        /*              read from server socket                      */
        if((n = read(sockfd, rcv_buff, RCV_BUFF)) < 0){ 
            write(STDERR_FILENO, "read() error\n", 14);
            exit(1);
        }
        rcv_buff[n] = '\0';
        /*                   quit program                            */
        if(!strcmp(rcv_buff, "QUIT")){
            write(STDOUT_FILENO, "Program quit!!\n", 16);
            exit(1);
        }
        process_result(rcv_buff);
        memset(rcv_buff, 0, sizeof(rcv_buff));
        
        while(n >= 4096){
            n = read(sockfd, rcv_buff, RCV_BUFF);
            process_result(rcv_buff);
            memset(rcv_buff, 0, sizeof(rcv_buff));
        }
    }
    /////////////          finish reading and writing buffer with socket            //////////////////


    close(sockfd);
    return 0;
}