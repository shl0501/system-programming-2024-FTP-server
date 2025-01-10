//////////////////////////////////////////////////////////////
// File Name : srv.c                                        //
// Date : 2024/05/08                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #2-2 (server)      //
// Description : open server socket and connect with client //
//               get string from client                     //
//               send string to client from chile process   //
//               print information of client from parent    //
//////////////////////////////////////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/wait.h>
#include<signal.h>

#define BUF_SIZE 256

//////////////////////////////////////////////////////////////
// client_info                                              //
// ======================================================== //
// Input : struct sockaddr_in client_addr                   //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : print client port_num and client ip address    //
//////////////////////////////////////////////////////////////
int client_info(struct sockaddr_in client_addr){
    write(1, "==========Client info==========", 32);
    write(STDOUT_FILENO, "\n\n", 3);
    /********************** client IP address ************************/
    write(1, "client IP : ", 13);
    char*client_IP = inet_ntoa(client_addr.sin_addr);
    write(STDOUT_FILENO, client_IP, strlen(client_IP));
    write(STDOUT_FILENO, "\n\n\n", 4);
    //////////////////////////////////////////////////////////////////

    /********************** client port number ***********************/
    char client_port[100];
    sprintf(client_port, "%d", client_addr.sin_port);
    write(1, "client port : ", 15);
    write(STDOUT_FILENO, client_port, strlen(client_port));
    //////////////////////////////////////////////////////////////////

    write(STDOUT_FILENO, "\n\n", 3);
    write(1, "===============================", 32);
    write(STDOUT_FILENO, "\n\n", 3);
    return 1;
}
void sh_chld(int); //signal handler for sighld
void sh_alrm(int); //signal handler for sigalrm

//////////////////////////////////////////////////////////////
// main                                                     //
// ======================================================== //
// Input : int argc, char**argv                             //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : open server socket and connect to client       //
//           get string from client and send it to client   //
//           using fork and signal handling                 //
//////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    char buff[BUF_SIZE];
    int n;
    struct sockaddr_in server_addr, client_addr;
    int server_fd, client_fd;
    int len;
    int port;

    /********************* prepare server socket and connect with client socket **********************/
    server_fd = socket(PF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//set address
    server_addr.sin_port = htons(atoi(argv[1]));//set port
    
    
    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){  //bind socket
        printf("Server: Can't bind local address\n");
        return 0;
    }   

    listen(server_fd, 5);   //listen from client
    /*************************************************************************************************/

    while(1)
    {
        pid_t pid;
        len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        
        /************************** Parent process **********************/
        if((pid = fork())> 0) {
            //  print client information    
            if(client_info(client_addr) < 0) 
                write(STDERR_FILENO, "client_info() err!!\n", 21);
            int ret = waitpid(pid, 0, 0);//wait for child process to be terminated
            sh_chld(0);//signal handling
        }
        /****************************************************************/


        /************************** child process ***********************/
        else{                   
            printf("Child Process ID : %d\n", getpid());    //print child process ID
            while(1){
                memset(buff, 0, BUF_SIZE);
                n = read(client_fd, buff, BUF_SIZE);        //read string from client

                char temp[BUF_SIZE];
                memset(temp, 0, BUF_SIZE);
                strcpy(temp, buff);
                temp[strlen(buff) -1] = '\0';

                /********** if received string is QUIT *************/
                if(!strcmp(temp, "QUIT")){                  
                    sh_alrm(getpid());  //signal handling
                    break;
                }
                /***************************************************/
                

                write(client_fd, buff, strlen(buff));   //write(send) string to client 
            }
        }
        /****************************************************************/
    }
    return 0;
}
//////////////////////////////////////////////////////////////
// sh_chld                                                  //
// ======================================================== //
// Input : int signum                                       //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : print that status of chile process changed     //
//////////////////////////////////////////////////////////////
void sh_chld(int signum){
    printf("Status of child process was changed.\n");
    wait(NULL);
}
//////////////////////////////////////////////////////////////
// sh_alrm                                                  //
// ======================================================== //
// Input : int signum                                       //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : print the child process will be terminated     //
//////////////////////////////////////////////////////////////
void sh_alrm(int signum){
    printf("Child Process(PID : %d) will be terminated.\n", getpid());
    exit(1);
}