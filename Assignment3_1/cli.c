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

#define MAX_BUF 20
#define CONT_PORT 20001

char ip_str[30];
void log_in(int sockfd) {
    int n;
    char username[MAX_BUF], * passwd, buf[MAX_BUF];

    memset(buf, 0, MAX_BUF);
    memset(username, 0, MAX_BUF);

    write(sockfd, ip_str, strlen(ip_str));  //send ip address to server

    /************** read result from server **************/
    n = read(sockfd, buf, MAX_BUF);
    buf[n - 1] = '\0';
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
        /******************** get ID from user *****************/
        write(1, "Input ID : ", 12);
        if (read(STDIN_FILENO, username, MAX_BUF) < 0) { /* receive string from user*/
            close(sockfd);
            exit(0);
        }
        username[n - 1] = '\0';

        //pass username to server
        write(sockfd, username, strlen(username));
        /*******************************************************/

        /******************** get password from user *****************/
        passwd = getpass("Input Password : ");
        write(sockfd, passwd, strlen(passwd));
        /*************************************************************/
        memset(buf, 0, MAX_BUF);
        n = read(sockfd, buf, MAX_BUF);
        buf[n] = '\0';

        if (!strcmp(buf, "OK")) {
            memset(buf, 0, MAX_BUF);
            n = read(sockfd, buf, MAX_BUF);
            buf[n] = '\0';

            if (!strcmp(buf, "OK")) {
                printf("** User '%s' logged in **\n", username);
                break;
            }
            if (!strcmp(buf, "FAIL")) {
                printf("** Log-in failed **\n");
                continue;
            }
            if (!strcmp(buf, "DISCONNECTION")) { // buf is ¡°DISCONNECTION¡±
                printf("** Connection closed **\n");
                break;
            }
        }
    }
}

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
    log_in(sockfd);
    close(sockfd);
    return 0;
}