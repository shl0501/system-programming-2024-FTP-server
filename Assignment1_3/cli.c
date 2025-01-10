#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<dirent.h>
#include<stdlib.h>

//////////////////////////////////////////////////////////////
// File Name : cli.c                                        //
// Date : 2024/04/17                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #1-3 (ftp server)  //
// Description : convert command to FTP command in client   //
//                  and give it to server                   //
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// Convert command                                          //
// ======================================================== //
// Input : int argc                                         //
//         char **argv                                      //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : convert command to FTP command for server      //
//////////////////////////////////////////////////////////////
#define MAX_SIZE 4096
int main(int argc, char **argv) {
    char buffer[MAX_SIZE];
    int cmd_num = 0;
    ////////////////    command check start     /////////////
    if(!strcmp("ls", argv[1]))      // case of ls
    {   
        strcpy(buffer, "NLST");
    }
    else if(!strcmp("dir", argv[1]))    //case of dir
        strcpy(buffer, "LIST");
    else if(!strcmp("pwd", argv[1]))    //case of pwd
        strcpy(buffer, "PWD");
    else if(!strcmp("cd", argv[1]))     //case of cd
    {   if(argc >= 3 && !(strcmp(argv[2], "..")))   //convert to cdup
            strcpy(buffer, "CDUP");
        else strcpy(buffer, "CWD");                 //conver to cwd
    }    
    else if(!strcmp("mkdir", argv[1]))  //case of mkdir
        strcpy(buffer, "MKD");
    else if(!strcmp("delete", argv[1])) //case of delete
        strcpy(buffer, "DELE");
    else if(!strcmp("rmdir", argv[1]))  //case of rmdir
        strcpy(buffer, "RMD");
    else if(!strcmp("rename", argv[1])){    //case of rename
        strcpy(buffer, "RNFR");
    }
    else if(!strcmp("quit", argv[1]))   //case of quit
        strcpy(buffer, "QUIT");
    else 
        return 0;
    /////////////////   end of checking command ////////////////

    ////////////////    add argument to buffer  ////////////////
    for(int i = 2; i < argc; i++) {
        strcat(buffer, " ");
        strcat(buffer, argv[i]);
    }
    ////////////////    end of adding argument  ////////////////

    strcat(buffer, "\0");
    write(STDOUT_FILENO, buffer, strlen(buffer));   //send buffer to srv
    write(STDOUT_FILENO, "\0", 2);
}