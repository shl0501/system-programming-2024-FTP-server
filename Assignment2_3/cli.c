//////////////////////////////////////////////////////////////
// File Name : cli.c                                        //
// Date : 2024/05/14                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #2-3 (client)      //
// Description : open client socket and connect with server //
//               convert input command and send it to server//
//               receive result from server                 //
//////////////////////////////////////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/wait.h>
#include<signal.h>
#include<string.h>

#define BUF_SIZE 4096

int sockfd;

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
    /****************seperate command from buffer***************/
    for(i = 0; i<strlen(buff); i++){
        if(buff[i] != ' ' && buff[i] != '\0' && buff[i] != '\n')
        {
            command[i] = buff[i];
            command[i+1] = '\0';
        }
        else{
            break;
        }
    }
    /**********************************************************/
 
    /************************convert FTP command*********************/
    if(!strcmp("ls", command))      // case of ls
    {   
        strcpy(cmd_buff, "NLST");
    }
    else if(!strcmp("dir", command))    //case of dir
        strcpy(cmd_buff, "LIST");
    else if(!strcmp("pwd", command))    //case of pwd
        strcpy(cmd_buff, "PWD");
    else if(!strcmp("cd", command))     //case of cd
    {   
        if(buff[i+1] == '.' && buff[i+2] == '.')//convert to cdup
            strcpy(cmd_buff, "CDUP");
        else strcpy(cmd_buff, "CWD");                 //conver to cwd
    }    
    else if(!strcmp("mkdir", command))  //case of mkdir
        strcpy(cmd_buff, "MKD");
    else if(!strcmp("delete", command)) //case of delete
        strcpy(cmd_buff, "DELE");
    else if(!strcmp("rmdir", command))  //case of rmdir
        strcpy(cmd_buff, "RMD");
    else if(!strcmp("rename", command)){    //case of rename
        strcpy(cmd_buff, "RNFR");
    }
    else if(!strcmp("quit", command))   //case of quit
        strcpy(cmd_buff, "QUIT");
    else 
        return -1;
    /***************************************************************/
    
    /*************** strcat option directory  *****************/
    int c = strlen(cmd_buff);
    for(i; i<strlen(buff); i++){
        cmd_buff[c++] = buff[i];
        cmd_buff[c] = '\0';
    }
    /**********************************************************/    

    return 0;
}
void sigint_handler(int sig) {
    char temp_buff[BUF_SIZE];
    strcpy(temp_buff, "QUIT");
    write(sockfd, temp_buff, strlen(temp_buff));
    exit(0);
}
//////////////////////////////////////////////////////////////
// main                                                     //
// ======================================================== //
// Input : int argc, char**argv                             //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : open client socket and connect to server       //
//           get string from user and send it to server     //
//////////////////////////////////////////////////////////////
int main(int argc, char**argv)
{
    char cmd_buff[BUF_SIZE];
    char buff[BUF_SIZE];
    int n;
    struct sockaddr_in serv_addr;

    /********************* prepare client socket **********************/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    /*****************************************************************/
    int sig;
    signal(SIGINT, sigint_handler);

    while(1){
        memset(buff, 0, BUF_SIZE);
        write(STDOUT_FILENO, "> ", 2);
        if(read(STDIN_FILENO, buff, BUF_SIZE) < 0){ /* receive string from user*/
        /*************** read error handling ****************/
            close(sockfd);
            exit(0);
        /***************************************************/
        }
        buff[strlen(buff)-1] = '\0';
        
        if(conv_cmd(buff, cmd_buff) < 0){
            write(1, "NO command\n", 12);
            continue;
        };


        if(write(sockfd, cmd_buff, BUF_SIZE) > 0){  /* send string to server */
            memset(buff, 0, BUF_SIZE);
            if((n = read(sockfd, buff, BUF_SIZE)) > 0) { /* receive string from server */
                buff[strlen(buff)] = '\0';
                write(1, buff, strlen(buff));
                memset(buff, 0, BUF_SIZE);
            }
            else    /* error handling for read */
            {
                close(sockfd);
                shutdown(sockfd, SHUT_RDWR);
                exit(0);
            }
        }
        else /* write error handling */
        {
            close(sockfd);
            exit(0);
        }
    }
    close(sockfd);
    return 0;
}