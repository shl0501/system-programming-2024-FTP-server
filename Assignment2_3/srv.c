//////////////////////////////////////////////////////////////
// File Name : srv.c                                        //
// Date : 2024/05/16                                        //  
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

#define BUF_SIZE 4096
#define MAX_CLIENTS 10

typedef struct {
    int pid;
    int port;
    time_t start_time;
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
int clients_cnt = 0;

int server_fd, client_fd;

//////////////////////////////////////////////////////////////
// error_handling                                           //
// ======================================================== //
// Input : int num                                          //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : handle error                                   //
//////////////////////////////////////////////////////////////
void error_handling(int num){
    /////////////   start checking error type   /////////////
    if(num == 0)    //case of invalid option
    {   
        write(1, "Error : invalid option\n\0", 25);
    }
    else if(num == 1)   //case of invalid argument 
    {   
        write(1, "Error : argument is not required\n\0", 35);

    }
    else if(num == 2){  //case of no argument
        write(1, "Error : argument is required\n\0", 31);
    }
    else if(num == 3){  //case of lack argument
        write(1, "Error : two arguments are required\n\0", 37);
    }
    else if(num == 4){  //case of much too many arguments
        write(1, "Error : only one argument can be processed\n\0", 45);
    }
    ////////////// end checking argument /////////////////////
    exit(0);
}
//////////////////////////////////////////////////////////////
// GetFiletype                                              //
// ======================================================== //
// Input : struct stat file                                 //
//         char *filenames                                  //
//        (Input parameter Description)                     //
// Output : char                                            //
//        (Out parameter Description)                       //
// Purpose : return filetype                                //
//////////////////////////////////////////////////////////////
char GetFiletype(struct stat file, char *filename)
{
    ////////////////// get file type ////////////////////////
    stat(filename, &file);
    if (S_ISBLK(file.st_mode))  //type of b
        return 'b';
    else if (S_ISCHR(file.st_mode)) //type of c
        return 'c';
    else if (S_ISDIR(file.st_mode)) //type of directory
        return 'd';
    else if (S_ISFIFO(file.st_mode))    //type of FiFO
        return 'p';
    else if (S_ISLNK(file.st_mode)) //type of link
        return 'l';
    else if (S_ISREG(file.st_mode)) //type of file
        return '-';
    else if (S_ISSOCK(file.st_mode))    //type of l
        return 'l';
    return 'N'; // not exist file
}

//////////////////////////////////////////////////////////////
// Getpermission                                            //
// ======================================================== //
// Input : struct stat file, char*filename, char*permission //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : return the permission of file                  //
//////////////////////////////////////////////////////////////
void GetPermission(struct stat file, char *filename, char*permission)
{
//////////////////////// Get permission ////////////////////////////////////////
    stat(filename, &file);
    if (file.st_mode & S_IRUSR)         //user READ permission
        strcat(permission, "r");
    else
        strcat(permission, "-");
    if (file.st_mode & S_IWUSR)         //user WRITE permission
        strcat(permission, "w");
    else
        strcat(permission, "-");
    if (file.st_mode & S_IXUSR)         //user EXECUTE permission
        strcat(permission, "x");
    else
        strcat(permission, "-");
    if (file.st_mode & S_IRGRP)         //group READ permission
        strcat(permission, "r");
    else
        strcat(permission, "-");
    if (file.st_mode & S_IWGRP)         //group WRITE permission
        strcat(permission, "w");
    else
        strcat(permission, "-");
    if (file.st_mode & S_IXGRP)         //group EXECUTE permission
        strcat(permission, "x");
    else
        strcat(permission, "-");
    if (file.st_mode & S_IROTH)         //other READ permission
        strcat(permission, "r");
    else
        strcat(permission, "-");
    if (file.st_mode & S_IWOTH)         //other WRITE permission
        strcat(permission, "w");
    else
        strcat(permission, "-");
    if (file.st_mode & S_IXOTH)         //other EXECUTE permission
        strcat(permission, "x");
    else
        strcat(permission, "-");
//////////////////////// End of Get permission ////////////////////////////////////////
}

//////////////////////////////////////////////////////////////
// correct_argument                                         //
// ======================================================== //
// Input : char *argument,                   //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : check if the argument trying to open is valid  //
//////////////////////////////////////////////////////////////
void correct_argument(char* argument)
{
    char *BUF = (char*)malloc(sizeof(char)*BUF_SIZE);
    DIR *dirp;
    struct dirent *dir;
    //open argument
    if(!(dirp = opendir(argument))){
        struct stat file;
        stat(argument, &file);
        char *permission = (char*)malloc(sizeof(char) *100);
        GetPermission(file, argument, permission);//get permission
        char filetype = GetFiletype(file, argument);   //get filetype
        if(filetype != 'N'){    //if the path is existing
            if(permission[0] == '-'){       //if not able to access file
                write(1, "Error : cannot access\n", 23);
            }   //end of if
        }
        else{   //if there is no permission => not existing directory
            write(1, "Error : No such file or directory\n", 35);
        }   //end of else
        free(permission);
        exit(0);
    }
    free(BUF);
}

//////////////////////////////////////////////////////////////
// option_l                                                 //
// ======================================================== //
// Input : char *argument                                   //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : return the file information in case of option l//
//////////////////////////////////////////////////////////////
void option_l(char *argument, char* result_buff){



    
    char*buf = (char*)malloc(sizeof(char) * BUF_SIZE);
    struct stat file;

    //////////////filetype//////////////////////
    char filetype = GetFiletype(file, argument);    //get filetype
    char *temp = (char*)malloc(sizeof(char)*BUF_SIZE);
    sprintf(temp, "%c", filetype);
    strcat(result_buff, temp);  //write file type
    stat(argument, &file);
    
    //////// permission //////////////////
    char *permission = (char*)malloc(sizeof(char)*BUF_SIZE);
    GetPermission(file, argument, permission);    //get permission
    strcat(result_buff, permission);    //write permission
    strcat(result_buff, " ");

    //////// nlink //////////////////////
    int filelink = file.st_nlink;           //get nlink
    sprintf(temp, "%d", filelink);
    strcat(result_buff, temp);           //write nlink
    strcat(result_buff, " ");
    memset(temp, 0, sizeof(temp));

    //////////////pw_name////////////////////////////
    uid_t owner_id;
    owner_id = file.st_uid;             //get owner of file
    struct passwd *pwd;
    pwd = getpwuid(owner_id);
    strcpy(temp, pwd->pw_name);
    strcat(result_buff, temp);       //write owner
    strcat(result_buff, " ");
    memset(temp, 0, sizeof(temp));

    /////////////// group id /////////////////////////
    struct group *grp = getgrgid(getgid()); //get group id
    strcat(result_buff, grp->gr_name);   //write group id
    strcat(result_buff, " ");

    /////////////// filesize/////////////////////////////
    long long filesize = file.st_size;  //get file size
    sprintf(temp, "%lld", filesize);
    strcat(result_buff, temp);   //write filesize
    strcat(result_buff, " ");
    memset(temp, 0, sizeof(temp));
    
    /////////////////// file time //////////////////////////
    time_t t = file.st_mtime;       //get file time
    struct tm *time = localtime(&t);

    char time_data[100][100];
    int td_num = 0;

    ////////////////// start parsing time information ////////////
    char *t_ptr = strtok(asctime(time), " ");
    while(t_ptr != NULL){
        if(td_num == 1){
            strcat(result_buff, t_ptr); //write month
            strcat(result_buff, " ");
            
        }
        else if(td_num == 2){
            strcat(result_buff, t_ptr); //write day
            strcat(result_buff, " ");
            
        }
        if(td_num == 3){
            for(int i = 0; i<5; i++)
                time_data[td_num][i] = t_ptr[i];
            strcat(result_buff, time_data[td_num]);
            strcat(result_buff, time_data[td_num]); //write time
            strcat(result_buff, " ");
            
        }
        td_num++;
        t_ptr = strtok(NULL, " ");
    }
    //////////// end of parsing time information ///////////////////

    strcat(result_buff, argument);   //write file name

    if(filetype == 'd')
    {   
        strcat(result_buff, "/"); //if it is directory, write /
    }
    strcat(result_buff, "\n");
}

//////////////////////////////////////////////////////////////
// ArrangeFilenames                                         //
// ======================================================== //
// Input : char **filenames, char**temp_filenames           //
//         int start, int end                               //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : arrange the list of file                       //
//////////////////////////////////////////////////////////////
void ArrangeFilenames(char **filenames, char **temp_filenames, int start, int end){
    if(start >= end)
        return;
    int pivot = start;
    int i = pivot + 1;
    int j = end;
    int temp;

    /////////////////////// start comparing pivot //////////////////////////////////
    while(i <= j){
        while(i <= end && strcasecmp(temp_filenames[i], temp_filenames[pivot]) <= 0)
            i++;
        while(j > start && strcasecmp(temp_filenames[j], temp_filenames[pivot]) >= 0)
            j--;
        
        char temp[4096];
        ///////////////// find the pivot place
        if(i > j){      // if find the pivot place
            strcpy(temp, temp_filenames[j]);
            strcpy(temp_filenames[j], temp_filenames[pivot]);
            strcpy(temp_filenames[pivot], temp);

            strcpy(temp, filenames[j]);
            strcpy(filenames[j], filenames[pivot]);
            strcpy(filenames[pivot], temp);
        }
        else{       //cross the start and end argument
            strcpy(temp, temp_filenames[i]);
            strcpy(temp_filenames[i], temp_filenames[j]);
            strcpy(temp_filenames[j], temp);

            strcpy(temp, filenames[i]);
            strcpy(filenames[i], filenames[j]);
            strcpy(filenames[j], temp);
        }
    }
    //////////////// recursive arrange  //////////////////////
    ArrangeFilenames(filenames, temp_filenames, start, j - 1);
    ArrangeFilenames(filenames, temp_filenames, j + 1, end);
}
//////////////////////////////////////////////////////////////
// pre_arrange                                              //
// ======================================================== //
// Input : char **filenames, char**temp_filenames           //
//         int start, int end                               //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : arrange the list of file                       //
//////////////////////////////////////////////////////////////
void pre_arrange(char** filenames, char**temp_filenames, int filecnt){
    for(int i = 0; i<filecnt; i++){
        {
            for (int j = 0; j < strlen(filenames[i]); j++)
            {
                temp_filenames[i][j] = filenames[i][j];
            }
        }
    }
    ArrangeFilenames(filenames, temp_filenames, 0, filecnt - 1);
}
//////////////////////////////////////////////////////////////
// cmd_process                                              //
// ======================================================== //
// Input : char *buff char* result_buff                     //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : command processign                             //
//////////////////////////////////////////////////////////////
int cmd_process(char*buff, char*result_buff){

    DIR *dirp;
    struct dirent *dir;

    char current_directory[200];
    getcwd(current_directory, sizeof(current_directory));
    
    memset(result_buff, 0, sizeof(result_buff));
    
    char command[30];
    memset(command, 0, sizeof(command));

    char ** argument = (char**)malloc(sizeof(char*)*BUF_SIZE);
    for(int i = 0; i<BUF_SIZE; i++){
        argument[i] = (char*)malloc(sizeof(char)*BUF_SIZE);
    }
    int argument_cnt = 0;

    char directory[BUF_SIZE] = ".";
    int option = 0;
    int aflag = 0;
    int lflag = 0;
    int dir_cnt = 0;
    int option_flag = 0;
    int invalid = 0;
    char*cur_dir = (char*)malloc(sizeof(char) * BUF_SIZE);
    /*                  seperate command, options, directory from buffer            */
    char *t_ptr = strtok(buff, " ");
    int td_num = 0;
    while(t_ptr != NULL){
        /*                   get command                      */
        if(td_num == 0){
            strcpy(command, t_ptr);
            td_num++; 
        }
        //////////////  finish getting command  ////////////////


        else if(td_num == 1){
            /*                  get options                         */
            if(t_ptr[0] == '-'){
                for(int s = 1; s<strlen(t_ptr); s++){
                    if(t_ptr[s] == 'a')
                        aflag++;
                    else if(t_ptr[s] == 'l')
                        lflag++;
                    else{
                        invalid = 1;
                        error_handling(0);
                        return -1;
                    }
                }
                option_flag = 1;
                td_num++;
            }
            //////////////          finish getting options      ///////

            /*                  get directory                         */
            else{
                strcpy(directory, t_ptr);
                strcpy(argument[argument_cnt++], t_ptr);
                dir_cnt++;
                td_num++;
            }
            //////////////          finish getting directory      ///////
        }
        else{
            if(dir_cnt == 1){
                dir_cnt++;
                strcpy(argument[argument_cnt++], t_ptr);
                //error_handling(4);      //too many directory
                //return -1;
            }
            //strcpy(directory, t_ptr);   //get directory 
            //strcpy(argument[argument_cnt++], t_ptr);
            td_num++;
        }
        t_ptr = strtok(NULL, " ");
    }
    //////////////////////          finish seperating from command       //////////////////////////
    char temp_pid[100];
    sprintf(temp_pid, "%d", getpid());

    if(!strcmp(command, "QUIT")){       //case of QUIT
        
        write(1, "QUIT\t[", 7);
        write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);

        strcpy(result_buff, "QUIT");
        return 1;
    }
    else if(!strcmp(command, "PWD")){
        /////////// handling error /////////////////////////////
        if(option_flag || invalid) error_handling(0);
        else if(dir_cnt >= 1) error_handling(1);
        //////////////get current working directory and print ///////////
        getcwd(cur_dir, BUF_SIZE);
        
        write(1, "PWD\t[", 6);
        write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);

        strcpy(result_buff, "\"");
        strcat(result_buff, cur_dir);
        strcat(result_buff, "\" is current directory\n\0");
        return 1;
    }
    else if(!strcmp(command, "CWD")){
        /****************** error check *********************/
        if(option_flag || invalid) error_handling(0);
        else if(dir_cnt == 0) error_handling(2);
        else{           //move to input directory
            correct_argument(directory);
            chdir(directory);
            getcwd(cur_dir, BUF_SIZE);
        }
        write(1, command, strlen(command));
        write(1, " ", strlen(" "));
        write(1, directory, strlen(directory));
        write(1, "\t[", 3);        
        write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);

        strcpy(result_buff, "\"");
        strcat(result_buff, cur_dir);
        strcat(result_buff, "\" is current directory\n\0");
        return 1;
    }
    else if(!strcmp(command, "CDUP")){
        if(option_flag || invalid) error_handling(0);
        chdir("..");//change directory to ..
        
        write(1, command, strlen(command));
        write(1, "\t[", 3);
        write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);
        
        getcwd(cur_dir, BUF_SIZE);
        strcpy(result_buff, "\"");
        strcat(result_buff, cur_dir);
        strcat(result_buff, "\" is current directory\n\0");
        return 0;
    }
    else if(!strcmp(command, "MKD")){
        /////////////// error handling ////////////////////////
        if(option_flag || invalid) error_handling(0);
        if(argument_cnt == 0) error_handling(2);
        ////////////////// make directory //////////////////////
        write(1, command, strlen(command));
        write(1, "\t[", 3);
        write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);

        for(int i = 0; i<dir_cnt; i++){
            if(!(dirp = opendir(argument[i]))){
                mkdir(argument[i], 0777);
                strcat(result_buff, "MKD ");
                strcat(result_buff, argument[i]);
                strcat(result_buff, "\n\0");
            }
            else {  //failed to make directory
                strcat(result_buff, "Error : cannot create directory '");
                strcat(result_buff, argument[i]);
                strcat(result_buff, "': File exists\n\0");
            }
        }
        return 1;
    }
    else if(!strcmp(command, "DELE")){
        ///////////////error handling ///////////////////
        if(option_flag || invalid) error_handling(0);
        if(argument_cnt == 0) error_handling(2);

        write(1, command, strlen(command));
        write(1, "\t[", 3);
        write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);

        for(int i = 0; i<argument_cnt; i++){
            int un = unlink(argument[i]);
            if(un == -1){
                strcat(result_buff, "Error : failed to delete '"); //failed to unlink
                strcat(result_buff, argument[i]);
                strcat(result_buff, "'\n\0");
            }
            if (un == 0){   //succeed in unlinking
                strcat(result_buff, "DELE ");
                strcat(result_buff, argument[i]);
                strcat(result_buff, "\n\0");
            }
        }
        return 1;
    }
    else if(!strcmp(command, "RMD")){
        if(option_flag || invalid) error_handling(0);
        if(argument_cnt == 0) error_handling(2);
        
        write(1, command, strlen(command));
        write(1, "\t[", 3);
        write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);
        
        for(int i = 0; i<argument_cnt; i++){
            int rm = rmdir(argument[i]);
            if(rm == -1){   //failed to remove
                strcat(result_buff, "Error : failed to remove '");
                strcat(result_buff, argument[i]);
                strcat(result_buff, "'\n\0");
            }
            if (rm == 0){   //succeeded in removing
                strcat(result_buff, "RMD ");
                strcat(result_buff, argument[i]);
                strcat(result_buff, "\n\0");
            }
        }
        return 1;
    }
    else if(!strcmp(command, "RNFR")){
        if(option_flag || invalid) error_handling(0);
        if(argument_cnt >= 3) error_handling(3);
        
        write(1, command, strlen(command));
        write(1, "\t[", 3);
        write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);

        if(access(argument[1], F_OK)!= -1){ //if there exists name already
            write(1, "Error : name to change already exists\n\0", 40);
            exit(0);
        }
        struct stat sttemp;
        if(stat(argument[1], &sttemp) == 0 && S_ISDIR(sttemp.st_mode)){ //failed to rename
            write(1, "Error : name to change already exists\n\0", 40);
            exit(0);
        }
        int rname = rename(argument[0], argument[1]);
        
        if(rname == -1){    //failed to rename
            write(1, "Error : name to change already exists\n\0", 40);
            exit(0);
        }
        if(rname == 0){ //succeeded in renaming
            strcat(result_buff, "RNFR ");
            strcat(result_buff, argument[0]);
            strcat(result_buff,"\n");
            strcat(result_buff, "RNTO ");
            strcat(result_buff, argument[1]);
            strcat(result_buff, "\n\0");
            return 1;
        }
    }
    else if(!strcmp(command, "NLST") || !strcmp(command, "LIST")){       //case of NLST or list
        if(dir_cnt > 1) error_handling(4);      //too many directory
        struct stat file;
         ////////////////// if the argument is file //////////////////////////
        if(access(directory, F_OK) == -1){
            correct_argument(directory);//check if i can open the directory
        }

        /*               case of no read permission             */
        char per[100];
        GetPermission(file, directory, per);
        if(per[0] == '-')
        {    
            write(1, "cannot access\n\0", 16);
            exit(0);
        }
        
        char filetype = GetFiletype(file, directory); //get filetype
        /*                      If directory is file                 */
        if(filetype == '-' ){
            if(!lflag)
                exit(0);
            write(1, "NLST -l", 8);
            write(1, "\t[", 3);
            write(1, temp_pid, strlen(temp_pid));
            write(1, "]\n", 3);

            option_l(directory, result_buff);
            chdir(current_directory);
            return 1;
        }

        char**filenames = (char**)malloc(sizeof(char*)*BUF_SIZE);
        for(int i = 0; i<BUF_SIZE; i++){
            filenames[i] = (char*)malloc(sizeof(char)*BUF_SIZE);
        }
        int filecnt = 0;
        /*              get the files of directory          */        
        dirp = opendir(directory);
        while((dir=readdir(dirp))!= NULL){
            if(aflag)                        //if a option
                strcpy(filenames[filecnt++], dir->d_name);
            else{
                if(dir->d_name[0] != '.')   //if not a option
                    strcpy(filenames[filecnt++], dir->d_name);
            }
        }
        closedir(dirp);

        /*          save current working directory and change directory             */
        chdir(directory);
        //////////////////////////////////////////////////////////////////////////////

        char**temp_filenames = (char**)malloc(sizeof(char*)*BUF_SIZE);
        for(int i = 0; i<BUF_SIZE; i++){
            temp_filenames[i] = (char*)malloc(sizeof(char)*BUF_SIZE);
            strcpy(temp_filenames[i], filenames[i]);
        }
        
        ArrangeFilenames(filenames, temp_filenames, 0, filecnt-1);//            arrange files 

        /************************** start nlst -al **************************/
        if(aflag && lflag){
            write(1, "NLST -al", 9);
            write(1, "\t[", 3);
            write(1, temp_pid, strlen(temp_pid));
            write(1, "]\n", 3);
            
            for(int i = 0; i<filecnt; i++){
                option_l(filenames[i], result_buff);
            }
            chdir(current_directory);
            return 1;
        }
        /////////////////////////////////////////////////////////////////////

        /************************** start nlst -a **************************/
        if(aflag){
            write(1, "NLST -a", 8);
            write(1, "\t[", 3);
            write(1, temp_pid, strlen(temp_pid));
            write(1, "]\n", 3);
            
            int c_num = 0;
            for(int i= 0; i<filecnt; i++){
                if(c_num == 5){
                    c_num = 0;
                    strcat(result_buff, "\n");
                }
                char filetype = GetFiletype(file, filenames[i]);
                if(filetype == 'd'){        //file type d
                    strcat(result_buff, filenames[i]);
                    strcat(result_buff, "/\t\0");
                }
                else{
                    strcat(result_buff, filenames[i]);
                    strcat(result_buff, "\t\0");
                }
                c_num++;
            }
            chdir(current_directory);
            strcat(result_buff, "\n\0");
            return 1;
        }
        ///////////////////////////////////////////////////////////////////

        /************************* start nlst -l**************************/
        if(lflag){
            write(1, "NLST -l", 8);
            write(1, "\t[", 3);
            write(1, temp_pid, strlen(temp_pid));
            write(1, "]\n", 3);

            for(int i= 0; i<filecnt; i++){
                option_l(filenames[i], result_buff);    //get file information
            }

            if(filecnt == 0){
                strcpy(result_buff, "\n\0");
            }
            chdir(current_directory);
            return 1;
        }
        /////////////////////////////////////////////////////////////////


        if(!strcmp(command,"LIST")){
            write(1, "LIST", 5);
            write(1, "\t[", 3);
            write(1, temp_pid, strlen(temp_pid));
            write(1, "]\n", 3);

            for(int i= 0; i<filecnt; i++){
                option_l(filenames[i], result_buff);    //get file information
            }
            if(filecnt == 0){
                strcpy(result_buff, "\n\0");
            }
            chdir(current_directory);
            return 1;
        }
        /*                      start nlst                          */
        else{
            write(1, "NLST", 5);
            write(1, "\t[", 3);
            write(1, temp_pid, strlen(temp_pid));
            write(1, "]\n", 3);
            int c_num = 0;
            for(int i =0; i<filecnt; i++){
                if(c_num == 5){
                    c_num = 0;
                    strcat(result_buff, "\n");
                }
                char filetype = GetFiletype(file, filenames[i]);
                if(filetype == 'd'){            //filetype d
                    strcat(result_buff, filenames[i]);
                    strcat(result_buff, "/\t\0");
                }
                else{
                    strcat(result_buff, filenames[i]);
                    strcat(result_buff, "\t\0");
                }
                c_num++;
            }
            if(filecnt == 0){
                strcpy(result_buff, "\n\0");
            }
            strcat(result_buff, "\n\0");
            chdir(current_directory);
            return 1;
        }
        //////////               finish nlst               /////////
        return 1;
    }
    return -1;
}


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


//////////////////////////////////////////////////////////////
// timer_handler                                            //
// ======================================================== //
// Input : int sig                                          //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : print information of clients every 10seconds   //
//////////////////////////////////////////////////////////////
void timer_handler(int sig) {
    write(1, "Current number of client: ", 27);  //print number of clients
    char temp[10];
    sprintf(temp, "%d", clients_cnt);
    write(1, temp, strlen(temp));
    write(1, "\n", 2);
    /********************** print current clients *****************************/
    write(1, "PID\tPORT\tTIME\t\n", 16);
    for (int i = 0; i < clients_cnt; i++) {
        time_t current_time = time(NULL);
        int process_time = (int)(current_time - clients[i].start_time) + 1;
        
        sprintf(temp, "%d", clients[i].pid);
        write(1, temp, strlen(temp));
        write(1, "\t", 2);
        sprintf(temp, "%d", clients[i].port);
        write(1, temp, strlen(temp));
        write(1, "\t", 2);
        sprintf(temp, "%d", process_time);
        write(1, temp, strlen(temp));
        write(1, "\t", 2);
        write(1, "\n", 2);
    }
    /**************************************************************************/
    alarm(10);//set 10seconds
}

//////////////////////////////////////////////////////////////
// sigint_handler                                           //
// ======================================================== //
// Input : int sig                                          //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : signal handling in case of terminate server    //
//////////////////////////////////////////////////////////////
void sigint_handler(int sig) {
    /******************end every clients and child processes************************/
    for (int i = 0; i < clients_cnt; i++) {
        kill(clients[i].pid, SIGTERM);
    }
    close(client_fd);
    close(server_fd);
    exit(0);
    /*******************************************************************************/
}

//////////////////////////////////////////////////////////////
// remove_client                                            //
// ======================================================== //
// Input : int sig                                          //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : kill terminated child process                  //
//////////////////////////////////////////////////////////////
void remove_client(int sig){
    int status;
    pid_t p = wait(&status);    //get pid of terminated child process
    for (int i = 0; i < clients_cnt; i++) {
        if (clients[i].pid == p) {
            kill(p, SIGTERM);       //kill child process
            printf("Client(%d)'s Release\n", p);
            /*************** erase process information ****************/
            for (int j = i; j < clients_cnt; j++) {
                clients[j] = clients[j+1];
            }
            clients_cnt--;
            break;
            /*********************************************************/
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
// Purpose : make server socket and communicate with clients//
//////////////////////////////////////////////////////////////
int main(int argc, char*argv[]) {
    char buff[BUF_SIZE];
    int n;
    struct sockaddr_in server_addr, client_addr;
    int len;
    int port;
    
    
    pid_t pid;

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

    /**************************** register signal handling  ********************************/
    signal(SIGALRM, timer_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, remove_client);
    alarm(10); // set 10 seconds alarm
    /***************************************************************************************/

    int num = 0;

    while (1) {
        pid_t pid;
        len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);   //connect
        if(client_info(client_addr) < 0) 
            write(STDERR_FILENO, "client_info() err!!\n", 21);

        if ((pid = fork()) == 0) { // child process
            close(server_fd); // close server socket which is used in parent process
            /****************************   communicate between sockets     *********************************/
            while(1){
                n = read(client_fd, buff, BUF_SIZE);        //read string from client
                char result_buff[BUF_SIZE];
                cmd_process(buff, result_buff);
                //if(!strcmp(result_buff, "QUIT")){
                    //printf("Client(%d)'s Release\n", getpid());
                    
                    //kill(getppid(), SIGUSR1);
                    //close(client_fd);
                    //exit(0);                
                //}
                if(!strcmp(result_buff, "QUIT")){
                    close(client_fd);
                    exit(0);   
                }
                write(client_fd, result_buff, strlen(result_buff));
                
            }
            /************************************************************************************************/

            close(client_fd);
            exit(0); // terminate chile process
        } else if (pid > 0) { // parent process
            clients[clients_cnt].pid = pid;
            clients[clients_cnt].port = client_addr.sin_port;
            clients[clients_cnt].start_time = time(NULL);
            clients_cnt++;
            timer_handler(1);
            alarm(10); // set 10 seconds alarm
            
            close(client_fd); // close client socket which is used in child process
            
        } else { // fork failed
            perror("fork");
            exit(1);
        }
    }

    close(server_fd);
    return 0;
}

