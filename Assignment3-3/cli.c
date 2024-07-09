//////////////////////////////////////////////////////////////
// File Name : cli.c                                        //
// Date : 2024/06/06                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #3-3 (client)      //
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
    else if(!strcmp("get", command)){   //case of get
        strcpy(cmd_buff, "RETR");
    }
    else if(!strcmp("put", command)){   //case of put
        strcpy(cmd_buff, "STOR");
    }
    else if(!strcmp("bin", command)){   //case of bin
        strcpy(cmd_buff, "TYPE I");
    }
    else if(!strcmp("ascii", command)) {    //case of ascii
        strcpy(cmd_buff, "TYPE A");
    } 
    else if(!strcmp("type", command)) { //case of type
        char optype[10];
        int opn = 0;
        for(int ch = 5; ch < strlen(buff); ch++) {  //get option
            optype[opn++] = buff[ch];
            optype[opn] = '\0';
        }

        if(!strcmp(optype, "binary")){  //case of TYPE I
            strcpy(cmd_buff, "TYPE I");
            cmd_buff[strlen(cmd_buff)] = '\0';
        }
        else if(!strcmp(optype, "ascii")){  //case of ascii
            cmd_buff[strlen(cmd_buff)-1] = '\0';
            strcpy(cmd_buff, "TYPE A");
        }
        else
            return -1;
        return 0;
    }
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
        strcat(addr, ",");
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
    char username[100], *passwd, buf[MAX_BUF];
    
    memset(buf, 0, MAX_BUF);
    memset(username, 0, 100);
    write(sockfd, ip_str, strlen(ip_str));  //send ip address to server

    /************** read result from server **************/
    sleep(0.1);
    n = read(sockfd, buf, MAX_BUF);
    buf[strlen(buf)] = '\0';
    /****************************************************/

    /****************** check result ********************/
    if(!strcmp(buf, "ACCEPTED")) {
        write(1, "Connected to sswlab.kw.ac.kr\n", 30);
        write(sockfd, "nothing but make order of read and write", MAX_BUF);
        sleep(0.5);
        memset(buf, 0, MAX_BUF);
        n = read(sockfd, buf, MAX_BUF);
        buf[strlen(buf)] = '\0';
        write(1, buf, strlen(buf));
    }

    if(!strcmp(buf, "REJECTION")){
        write(1, "** Connection refused **\n", 26);
        memset(buf, 0, MAX_BUF);
        write(sockfd, "nothing but make order of read and write", MAX_BUF);
        n = read(sockfd, buf, MAX_BUF);
        buf[strlen(buf)] = '\0';
        write(1, buf, strlen(buf));
        close(sockfd);
        exit(0);
    }
    /***************************************************/

    for(;;){
        /******************** send ID and username from user *****************/
        write(1, "Input Name : ", 14);
        memset(username, 0, 100);
        if(read(STDIN_FILENO, username, 100) < 0){ /* receive string from user*/
            close(sockfd);
            exit(0);
        }
        username[strlen(username)-1] = '\0';

        /*
        * send USER username
        */
        memset(buf, 0, MAX_BUF);
        sprintf(buf, "USER %s", username);
        //pass username to server
        write(sockfd, buf, MAX_BUF);

        sleep(0.5);
        memset(buf, 0, MAX_BUF);
        read(sockfd, buf, MAX_BUF);
        write(1, buf, strlen(buf));
        
        char temp_num[10];
        for(int i = 0; i<strlen(buf); i++){
            if(buf[i] != ' '){
                temp_num[i] = buf[i];
                temp_num[i+1] = '\0';
            }
            else
                break;
        }
        if(atoi(temp_num) == 430)
            continue;
        if(atoi(temp_num) == 530)
            break;
        
        //get password from user
        passwd = getpass("Input Password : ");
        
        //pass password to server
        memset(buf, 0, MAX_BUF);
        sprintf(buf, "PASS %s", passwd);
        write(sockfd, buf, MAX_BUF);
    /*************************************************************/
        

        memset(buf, 0, MAX_BUF);
        n = read(sockfd, buf, MAX_BUF);
        buf[strlen(buf)] = '\0';
        //write(1, buf, strlen(buf));

            /***************case of success***************/
        if(!strcmp(buf, "OK")){
            write(1, "** User '", 10);
            write(1, username, strlen(username));
            write(1, "' logged in **\n", 16);
            memset(buf, 0, MAX_BUF);
            n = read(sockfd, buf, MAX_BUF);
            buf[strlen(buf)] = '\0';
            write(1, buf, strlen(buf));
            break;
        }
        /********************************************/

        /*************Failed to log-in**************/
        if(!strcmp(buf, "FAIL")){   //fail->re-try
            write(1, "** Log-in failed **\n", 21);
            memset(buf, 0, MAX_BUF);
            n = read(sockfd, buf, MAX_BUF);
            buf[strlen(buf)] = '\0';
            write(1, buf, strlen(buf));
            continue;
        }
        if(!strcmp(buf, "DISCONNECTION")) { // buf is “DISCONNECTION”
            write(1, "** Connection closed **\n", 25);
            memset(buf, 0, MAX_BUF);
            n = read(sockfd, buf, MAX_BUF);
            buf[strlen(buf)] = '\0';
            write(1, buf, strlen(buf));

            exit(0);
            break;
        } 
        /*******************************************/
    }
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
    //pointer to dotted-decimal string//
    strcpy(ip_str,inet_ntoa(temp.sin_addr));
    temp.sin_port = htons(atoi(argv[2]));
    connect(control_sockfd, (struct sockaddr*) &temp, sizeof(temp));    //connect with server
    /*****************************************************************/
    
    
    log_in(control_sockfd); //log_in process





    /*
    *
    *       get Command from user and convert command
    *
    */
    for(;;){
        char cmd_buf[100];
        memset(cmd_buf, 0, 100);
        memset(buf, 0, MAX_BUF);
        read(STDIN_FILENO, buf, MAX_BUF); 
        buf[strlen(buf)-1] = '\0';
        
        if(conv_cmd(buf, cmd_buf) < 0){
            write(1, "NO command\n", 12);
        };

        //send FTP command

        if(!strcmp(buf, "quit"))        //type of quit
        {
            write(control_sockfd, cmd_buf, MAX_BUF);
            exit(0);
        }

        int get = 0;
        if(buf[0] == 'g' && buf[1] == 'e' && buf[2] == 't')     //type of get
            get = 1;
        int put = 0;
        if(buf[0] == 'p' && buf[1] == 'u' && buf[2] == 't')     //type of put
            put = 1;
        
        if(buf[0] == 'l' && buf[1] == 's' || buf[0] == 'g' && buf[1] == 'e' && buf[2] == 't' || put == 1) {
            sleep(0.5);
            write(control_sockfd, cmd_buf, MAX_BUF);
        
            /****************** make random port number ********************/
            int data_port = rand() %50001 + 10000;
            hostport = convert_addr_to_str(temp.sin_addr.s_addr, data_port);    //make port command
            /***************************************************************/

            memset(buf, 0, MAX_BUF);
            read(control_sockfd, buf, MAX_BUF);

            
            sleep(0.5);
            write(control_sockfd, hostport, MAX_BUF);   //send port command
            /*
            *
            *           Connect data connection
            * 
            */
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

            
            /********get message from control connection("200 PORT successful")********/
            memset(buf, 0, MAX_BUF);
            read(control_sockfd, buf, MAX_BUF);
            buf[strlen(buf)] = '\0';
            write(1, buf, strlen(buf));
            /*************************************************************************/

            sleep(0.01);
    
            /********get message from control connection("150 opening successful")********/
            memset(buf, 0, MAX_BUF);
            read(control_sockfd, buf, MAX_BUF);
            buf[strlen(buf)] = '\0';

            write(1, buf, strlen(buf));

            int total_bytes = strlen(buf);
            /****************** get result from data_connfd *********************/

            /*
            *       case of PUT
            */
            if(put) {
                char filename[50];
                int fi = 5;
                for(fi; fi<strlen(cmd_buf); fi++){
                    
                    filename[fi-5] = cmd_buf[fi];

                }
                filename[fi-1] = '\0';
                memset(buf, 0, MAX_BUF);

                //get filename
                char arg[MAX_BUF];
                snprintf(arg, sizeof(arg), "%s/%s", getcwd(NULL, 0), filename);

                //open file
                FILE*temp_fp;
                temp_fp = fopen(arg, "r");


                if(temp_fp == NULL){    //failed to open
                    write(1, "Failed to open", 15);
                    memset(buf, 0, MAX_BUF);
                    strcpy(buf, "NoF#\0");
                }
                else{           //succeeded to open
                    memset(buf, 0, MAX_BUF);
                    char tempstr[MAX_BUF];
                    while((fgets(tempstr, MAX_BUF, temp_fp) != NULL)){  //open file and get contents
                        tempstr[strlen(tempstr)] = '\0';
                        strcat(buf, tempstr);                   
                        memset(tempstr, 0, MAX_BUF);
                    }
                }

                sleep(0.5);
                write(data_connfd, buf, MAX_BUF);



            }
            else if(get) {      //case of get
                char filename[50];
                int fi = 5;
                for(fi; fi<strlen(cmd_buf); fi++){
                    
                    filename[fi-5] = cmd_buf[fi];

                }
                filename[fi-5] = '\0';
                memset(buf, 0, MAX_BUF);
                int bytes_read;
                bytes_read = read(data_connfd, buf, MAX_BUF);
                buf[strlen(buf)] = '\0';
                if(buf[0] == 'N'&& buf[1] == 'o' && buf[2] == 'F'&& buf[3] == '#'){ //failed to open file
                    write(1, "Failed to open\n", 16);            
                    memset(buf, 0, MAX_BUF);
                    memset(cmd_buf, 0, 100);
                    memset(filename, 0, 50);
                    exit(0);
                }
                FILE*temp_fp = fopen(filename, "w");
                fprintf(temp_fp, "%s", buf);
                fflush(temp_fp);
            }
            else{       //succeeded to open file
                memset(buf, 0, MAX_BUF);
                read(data_connfd, buf, MAX_BUF);
                buf[strlen(buf)] = '\0';
                write(1, buf, strlen(buf));
            }
            /********************************************************************/
            


            /************ receive message from control connection("226 Result successful") **************/
            write(control_sockfd, "dummy\n", MAX_BUF);

            memset(buf, 0, MAX_BUF);
            read(control_sockfd, buf, MAX_BUF);
            buf[strlen(buf)] = '\0';
            write(1, buf, strlen(buf));
            /********************************************************************************************/

            printf("OK. %d bytes is received\n", total_bytes);
            
            memset(buf, 0, MAX_BUF);
            memset(cmd_buf, 0, 100);
            close(data_connfd);
            continue;
        }



        //normal case of command -> get reply message
        sleep(0.5);
        write(control_sockfd, cmd_buf, MAX_BUF);
        
        memset(buf, 0, MAX_BUF);
        read(control_sockfd, buf, MAX_BUF);
        

        buf[strlen(buf)] = '\0';
        write(1, buf, strlen(buf));

        if(cmd_buf[0] == 'R' && cmd_buf[1] == 'N' && cmd_buf[2] == 'F' && cmd_buf[3] == 'R'){   //case of rnfr, it should get more buf
            memset(buf, 0, MAX_BUF);
            read(control_sockfd, buf, MAX_BUF);
        

            buf[strlen(buf)] = '\0';
            write(1, buf, strlen(buf));
        }
        memset(buf, 0, MAX_BUF);
        memset(cmd_buf, 0, 100);

    }
    
    return 0;


}
