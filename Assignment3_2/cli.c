//////////////////////////////////////////////////////////////
// File Name : cli.c                                        //
// Date : 2024/05/25                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #3-2 (client)      //
// Description : make control connection and data connection//
//               send result  using data connection         //
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
#include<time.h>
#include<math.h>

#define MAX_BUF 4096

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

//////////////////////////////////////////////////////////////
// convert_addr_to_str                                      //
// ======================================================== //
// Input : unsigned long ip_addr, unsigned int port         //
//        (Input parameter Description)                     //
// Output : char*                                           //
//        (Out parameter Description)                       //
// Purpose : make PORT command using converted port number  //
//           and ip address                                 //
//////////////////////////////////////////////////////////////
char* convert_addr_to_str(unsigned long ip_addr, unsigned int port)
{
    struct in_addr ip_struct;
    ip_struct.s_addr = ip_addr;
    char*ip = inet_ntoa(ip_struct);

    char *addr = (char*)malloc(sizeof(char) *MAX_BUF);
    strcpy(addr, "PORT ");
    
    int ip_num = 0;
    char *ptr = strtok(ip, ".");
    /*************** start slicing **************/
    while(ptr != NULL) {
        strcat(addr, ptr);//strcat sliced string
        addr[strlen(addr)] = ',';
        ptr = strtok(NULL, ".");    //put ip address to port command
    }
    /*******************************************/

    /*********  convert to 16bit binary **********/
    char* binary = (char*)malloc(17);
    binary[16] = 0;
    for(int i = 15; i>= 0; i--) {
        binary[i] = (port&1) ? '1' : '0';
        port >>= 1;
    }
    /**************************************/

    /******** convert to MSB binary to decimal **********/
    int decimal1 = 0;
    int two = 1;
    for(int i = 7; i>=0; i--){
        decimal1 += two * (binary[i] - '0');
        two *= 2;
    }
    /**************************************/

    char temp[10];
    sprintf(temp, "%d", decimal1);

    strcat(addr, temp);
    addr[strlen(addr)] = ',';

    /********* convert to LSB binary to decimal **********/
    int decimal2 = 0;
    two = 1;
    for(int i = 15; i>=8; i--){
        decimal2 += two * (binary[i] - '0');
        two *= 2;
    }
    /****************************************************/

    memset(temp, 0, 10);
    sprintf(temp, "%d", decimal2);

    strcat(addr, temp);
    addr[strlen(addr)] = '\0';


    return addr;        //return ip address
}
//////////////////////////////////////////////////////////////
// main                                                     //
// ======================================================== //
// Input : int argc, char **argv                            //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : send and get messages by data connetion        //
//           and control connection                         //
//////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    srand(time(NULL));

    char buf[MAX_BUF];

    char *hostport;
    struct sockaddr_in temp;
    int control_sockfd;

    /********************* prepare control connection **********************/
    control_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&temp, 0, sizeof(temp));
    temp.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &temp.sin_addr);
    temp.sin_port = htons(atoi(argv[2]));
    connect(control_sockfd, (struct sockaddr*) &temp, sizeof(temp));    //connect with server
    /*****************************************************************/

    /****************** make random port number ********************/
    int data_port = rand() %20000 + 10001;
    hostport = convert_addr_to_str(temp.sin_addr.s_addr, data_port);    //make port command
    /***************************************************************/


    write(control_sockfd, hostport, MAX_BUF);   //send port command


    /**************************************** prepare data connection **********************************************/
    int data_listenfd, data_connfd;
    struct sockaddr_in data_servaddr, data_cliaddr;

    data_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(data_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&data_servaddr, 0, sizeof(data_servaddr));
    data_servaddr.sin_family = AF_INET;
    data_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//set address
    data_servaddr.sin_port = htons(data_port);//set port
        
    if(bind(data_listenfd, (struct sockaddr *)&data_servaddr, sizeof(data_servaddr)) < 0){  //bind socket
        printf("Server: Can't bind local address\n");
        return 0;
    }   

    listen(data_listenfd, 5);   //listen from client

    int clilen= sizeof(data_cliaddr);

    data_connfd = accept(data_listenfd, (struct sockaddr *) &data_cliaddr, &clilen);
    /*************************************************************************************************************/


    /*********receive command and convert command *******/
    char cmd_buf[100];
    read(STDIN_FILENO, buf, MAX_BUF); 
    buf[strlen(buf)-1] = '\0';
        
    if(conv_cmd(buf, cmd_buf) < 0){
        write(1, "NO command\n", 12);
    };
    /**************************************************/

    sleep(0.01);

    /********get message from control connection("200 PORT successful")********/
    memset(buf, 0, MAX_BUF);
    read(control_sockfd, buf, MAX_BUF);
    buf[strlen(buf)] = '\0';
    write(1, buf, strlen(buf));
    /*************************************************************************/


    /**************send command***************/
    write(control_sockfd, cmd_buf, strlen(cmd_buf));
    /*****************************************/

    sleep(0.01);
    
    /********get message from control connection("150 opening successful")********/
    memset(buf, 0, MAX_BUF);
    read(control_sockfd, buf, MAX_BUF);
    buf[strlen(buf)] = '\0';

    write(1, buf, strlen(buf));
    /**************************************************************************/

    sleep(0.01);

    /****************** get result from data_connfd *********************/
    memset(buf, 0, MAX_BUF);
    read(data_connfd, buf, MAX_BUF);
    buf[strlen(buf)] = '\0';
    write(1, buf, strlen(buf));
    /********************************************************************/

    close(data_connfd);                 //close data_connection socket


    sleep(0.01);

    /************ receive message from control connection("226 Result successful") **************/
    memset(buf, 0, MAX_BUF);
    read(control_sockfd, buf, MAX_BUF);
    buf[strlen(buf)] = '\0';
    write(1, buf, strlen(buf));
    /********************************************************************************************/

    printf("%ld bytes is received\n", strlen(buf));

}
