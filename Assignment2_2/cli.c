//////////////////////////////////////////////////////////////
// File Name : cli.c                                        //
// Date : 2024/05/08                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #2-2 (client)      //
// Description : open client socket and connect with server //
//               send string to server                      //
//               receive string from server                 //
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

#define BUF_SIZE 256
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
    char buff[BUF_SIZE];
    int n;
    int sockfd;
    struct sockaddr_in serv_addr;

    /********************* prepare client socket **********************/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    /*****************************************************************/

    while(1){
        memset(buff, 0, BUF_SIZE);
        write(STDOUT_FILENO, "> ", 2);
        if(read(STDIN_FILENO, buff, BUF_SIZE) < 0){ /* receive string from user*/
        /*************** read error handling ****************/
            close(sockfd);
            exit(0);
        /***************************************************/
        }
        buff[strlen(buff)] = '\0';
        if(write(sockfd, buff, BUF_SIZE) > 0){  /* send string to server */
            memset(buff, 0, BUF_SIZE);
            if((n = read(sockfd, buff, BUF_SIZE)) > 0) { /* receive string from server */
            
                buff[strlen(buff)] = '\0';
                printf("from server:%s", buff);
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