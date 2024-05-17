//////////////////////////////////////////////////////////////
// File Name : cli.c                                        //
// Date : 2024/05/18                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #3-1 (client)      //
// Description : execute cli with ip address and port number//
//               get username and password from user        //
//               trying to log-in and get result from server//
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

#define MAX_BUF 20
#define CONT_PORT 20001

char ip_str[30];
//////////////////////////////////////////////////////////////
// log_in                                                   //
// ======================================================== //
// Input : int sockfd                                       //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : check if user can log-in                       //
//////////////////////////////////////////////////////////////
void log_in(int sockfd) {
    int n;
    char username[MAX_BUF], * passwd, buf[MAX_BUF];

    memset(buf, 0, MAX_BUF);
    memset(username, 0, MAX_BUF);

    write(sockfd, ip_str, strlen(ip_str));  //send ip address to server

    /************** read result from server **************/
    n = read(sockfd, buf, MAX_BUF);
    buf[strlen(buf)] = '\0';
    /****************************************************/

    /****************** check result ********************/
    if (!strcmp(buf, "ACCEPTED"))
        write(1, "** It is connected to Server **\n", 33);

    if (!strcmp(buf, "REJECTION")) {
        write(1, "** Connection refused **\n", 26);
        close(sockfd);
        exit(0);
    }
    /***************************************************/


    for (;;) {
        /******************** send ID and username from user *****************/
        write(1, "Input ID : ", 12);
        memset(username, 0, MAX_BUF);
        if (read(STDIN_FILENO, username, MAX_BUF) < 0) { /* receive string from user*/
            close(sockfd);
            exit(0);
        }
        username[strlen(username) - 1] = '\0';

        //get password from user
        passwd = getpass("Input Password : ");


        //pass username to server
        write(sockfd, username, strlen(username));

        memset(buf, 0, MAX_BUF);
        read(sockfd, buf, MAX_BUF);

        //pass password to server
        write(sockfd, passwd, strlen(passwd));
        /*************************************************************/

        memset(buf, 0, MAX_BUF);
        n = read(sockfd, buf, MAX_BUF);
        write(sockfd, "NULL", 5);   //meaningless

        buf[strlen(buf)] = '\0';
        if (!strcmp(buf, "OK")) {
            memset(buf, 0, MAX_BUF);

            read(sockfd, buf, MAX_BUF); //get result from server
            buf[strlen(buf)] = '\0';
            /***************case of success***************/
            if (!strcmp(buf, "OK")) {
                write(1, "** User '", 10);
                write(1, username, strlen(username));
                write(1, "' logged in **\n", 16);
                break;
            }
            /********************************************/

            /*************Failed to log-in**************/
            if (!strcmp(buf, "FAIL")) {   //fail->re-try
                write(1, "** Log-in failed **\n", 21);
                continue;
            }
            if (!strcmp(buf, "DISCONNECTION")) { // buf is ¡°DISCONNECTION¡±
                write(1, "** Connection closed **\n", 25);
                break;
            }
            /*******************************************/
        }
    }
}
//////////////////////////////////////////////////////////////
// main                                                     //
// ======================================================== //
// Input : int argc, char*argv[]                            //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : connect sockets and control logic              //
//////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    int sockfd, n, p_pd;
    struct sockaddr_in servaddr;


    /********************* prepare client socket **********************/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    //pointer to dotted-decimal string//
    strcpy(ip_str, inet_ntoa(servaddr.sin_addr));

    servaddr.sin_port = htons(atoi(argv[2]));
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));    //connect with server
/*****************************************************************/
    log_in(sockfd); //log_in process
    close(sockfd);
    return 0;
}