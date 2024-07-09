//////////////////////////////////////////////////////////////
// File Name : srv.c                                        //
// Date : 2024/06/06                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #3-3 (server)      //
// Description : make control connection and data connection//
//               send result  using data connection         //
//               make log file and reply message            //
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


#define MAX_BUF 4096
FILE*logFile;
void log_info(char logInfo[500], struct sockaddr_in client_addr, char*ip, char*user);
void get_time(char time_str[100]);

char cur_time[100];
//////////////////////////////////////////////////////////////
// error_handling                                           //
// ======================================================== //
// Input : int num                                          //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : handle error                                   //
//////////////////////////////////////////////////////////////
void error_handling(int num, char*result_buff){
    /////////////   start checking error type   /////////////
    if(num == 0)    //case of invalid option
    {   
        strcpy(result_buff, "Error : invalid option\n\0");
    }
    else if(num == 1)   //case of invalid argument 
    {   
        strcpy(result_buff, "Error : argument is not required\n\0");
    }
    else if(num == 2){  //case of no argument
        strcpy(result_buff, "Error : argument is required\n\0");
    }
    else if(num == 3){  //case of lack argument
        strcpy(result_buff, "Error : two arguments are required\n\0");
    }
    else if(num == 4){  //case of much too many arguments
        strcpy(result_buff, "Error : only one argument can be processed\n\0");
    }
    ////////////// end checking argument /////////////////////
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
int correct_argument(char* argument, char*result_buff)
{
    char *BUF = (char*)malloc(sizeof(char)*MAX_BUF);
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
                strcpy(result_buff, "Error : cannot access\n");
            }   //end of if
        }
        else{   //if there is no permission => not existing directory
            strcpy(result_buff, "Error : No such file or directory\n");
        }   //end of else
        free(permission);
        return -1;
    }
    free(BUF);
    return 0;
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



    
    char*buf = (char*)malloc(sizeof(char) * MAX_BUF);
    struct stat file;

    //////////////filetype//////////////////////
    char filetype = GetFiletype(file, argument);    //get filetype
    char *temp = (char*)malloc(sizeof(char)*MAX_BUF);
    sprintf(temp, "%c", filetype);
    strcat(result_buff, temp);  //write file type
    stat(argument, &file);
    
    //////// permission //////////////////
    char *permission = (char*)malloc(sizeof(char)*MAX_BUF);
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
int cmd_process(char*buff, char*result_buff, char logstr[100], struct sockaddr_in cliaddr, char *ip, char*user, int connfd){

    DIR *dirp;
    struct dirent *dir;

    char current_directory[200];
    getcwd(current_directory, sizeof(current_directory));
    
    memset(result_buff, 0, sizeof(result_buff));
    
    char command[30];
    memset(command, 0, sizeof(command));

    char ** argument = (char**)malloc(sizeof(char*)*MAX_BUF);
    for(int i = 0; i<MAX_BUF; i++){
        argument[i] = (char*)malloc(sizeof(char)*MAX_BUF);
    }
    int argument_cnt = 0;

    char directory[MAX_BUF] = ".";
    int option = 0;
    int aflag = 0;
    int lflag = 0;
    int dir_cnt = 0;
    int option_flag = 0;
    int invalid = 0;
    char*cur_dir = (char*)malloc(sizeof(char) * MAX_BUF);
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
                        error_handling(0, result_buff);
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
            dir_cnt++;
            strcpy(argument[argument_cnt++], t_ptr);
            strcpy(directory, t_ptr);
            td_num++;
        }
        t_ptr = strtok(NULL, " ");
    }
    //////////////////////          finish seperating from command       //////////////////////////
    char temp_pid[100];
    sprintf(temp_pid, "%d", getpid());

    if(!strcmp(command, "QUIT")){       //case of QUIT
        
        sprintf(result_buff, "221 Goodbye\n");
        write(1, result_buff, strlen(result_buff));
        return 5;
    }
    else if(!strcmp(command, "PWD")){
        /////////// handling error /////////////////////////////
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return -1;
        }
        else if(dir_cnt >= 1) {
            error_handling(1, result_buff);
            return -1;
        }
        //////////////get current working directory and print ///////////
        getcwd(cur_dir, MAX_BUF);
        
        write(1, "PWD\n", 5);
        sprintf(result_buff, "257 \"%s\" is current directory\n", cur_dir);
        write(1, result_buff, strlen(result_buff));        
        return 1;
    }
    else if(!strcmp(command, "CWD")){
        /****************** error check *********************/
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return -1;
        }
        else if(dir_cnt == 0) {
            error_handling(2, result_buff);
            return -1;
        }
        else{           //move to input directory
            if(correct_argument(directory, result_buff)<0){
                write(1, "CWD\n", 5);
                sprintf(result_buff, "550 Can't find such file or directory.\n");
                write(1, result_buff, strlen(result_buff));     
                return 0;
            }
            chdir(directory);
            getcwd(cur_dir, MAX_BUF);
        }
        write(1, "CWD\n", 5);
        sprintf(result_buff, "250 CWD command succeeds\n");
        write(1, result_buff, strlen(result_buff));        
        return 1;
    }
    else if(!strcmp(command, "CDUP")){
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return 1;
        }
        chdir("..");//change directory to ..
        
        getcwd(cur_dir, MAX_BUF);
        write(1, "CDUP\n", 5);
        sprintf(result_buff, "250 CWD command succeeds\n");
        write(1, result_buff, strlen(result_buff));    
        return 0;
    }
    else if(!strcmp(command, "MKD")){
        /////////////// error handling ////////////////////////
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return 0;
        }
        if(argument_cnt == 0) {
            error_handling(2, result_buff);
            return 0;
        }
        ////////////////// make directory //////////////////////
        write(1, command, strlen(command));
        write(1, "\n", 2);

        for(int i = 0; i<dir_cnt; i++){
            if(!(dirp = opendir(argument[i]))){
                mkdir(argument[i], 0777);       //make directory
                sprintf(result_buff, "250 MKD command performed successfully.\n");
                write(1, result_buff, strlen(result_buff));   
            }
            else {  //failed to make directory
                sprintf(result_buff, "550 %s: can't create directory.\n", argument[i]);
                write(1, result_buff, strlen(result_buff));   
            }
        }
        return 1;
    }
    else if(!strcmp(command, "DELE")){
        ///////////////error handling ///////////////////
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return 0;
        }
        if(argument_cnt == 0) {
            error_handling(2, result_buff);
            return 0;
        }

        write(1, command, strlen(command));
        write(1, "\n", 2);

        for(int i = 0; i<argument_cnt; i++){
            int un = unlink(argument[i]);       //delete directory
            if(un == -1){                       //failed to delete
                sprintf(result_buff, "550 %s : Can't find such file or directory.\n", argument[i]);
                write(1, result_buff, strlen(result_buff));      
            }
            if (un == 0){   //succeed in unlinking
                sprintf(result_buff, "250 DELE command performed successfully.\n");
                write(1, result_buff, strlen(result_buff));   
            }
        }
        return 1;
    }
    else if(!strcmp(command, "RMD")){       //remove directory
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return 0;
        }
        if(argument_cnt == 0) {
            error_handling(2, result_buff);
            return 0;
        }
        
        write(1, command, strlen(command));
        write(1, "\n", 2);
        
        for(int i = 0; i<argument_cnt; i++){
            int rm = rmdir(argument[i]);            //remove directory
            if(rm == -1){   //failed to remove
                sprintf(result_buff, "550 %s: can't remove directory.\n", argument[i]);
                write(1, result_buff, strlen(result_buff));  
            }
            if (rm == 0){   //succeeded in removing
                sprintf(result_buff, "250 RMD command performed successfully.\n");
                write(1, result_buff, strlen(result_buff));   
            }
        }
        return 1;
    }
    else if(!strcmp(command, "RNFR")){      //rename file
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return 0;
        }
        if(argument_cnt >= 3) {
            error_handling(3, result_buff);
            return 0;
        }

        int suc = 0;

        struct stat temp_buf;
            if(stat(argument[0], &temp_buf) == 0){  //if there is file to change
                sprintf(result_buff, "350 File exists, ready to rename.\n");
                write(1, result_buff, strlen(result_buff));

                sleep(0.5);
                write(connfd, result_buff, MAX_BUF);

                memset(logstr, 0, 500);
                log_info(logstr, cliaddr, ip, user);    //get log information
                fprintf(logFile, "%s %s", logstr, result_buff);
                fflush(logFile);
                memset(result_buff, 0, MAX_BUF);
            }
            else{               //if there is no file to change
                sprintf(result_buff, "550 %s: Can't find such file or directory\n", argument[0]);   //failed to change
                write(1, result_buff, strlen(result_buff));
                sleep(0.5);
                write(connfd, result_buff, MAX_BUF);

                memset(logstr, 0, 500);             //write to log file
                log_info(logstr, cliaddr, ip, user);
                fprintf(logFile, "%s %s", logstr, result_buff);
                fflush(logFile);
                suc = 1;
                memset(result_buff, 0, MAX_BUF);
            }
        
        if(suc == 1 || stat(argument[1], &temp_buf) == 0){      //if there already file name to change
            sprintf(result_buff, "550 %s : Can't be renamed.\n", argument[1]);  //failed to rename
            write(1, result_buff, strlen(result_buff));
            sleep(0.5);
            write(connfd, result_buff, MAX_BUF);
            //write log file
            memset(logstr, 0, 500);
            log_info(logstr, cliaddr, ip, user);
            fprintf(logFile, "%s %s", logstr, result_buff);
            fflush(logFile);
            memset(result_buff, 0, MAX_BUF);
            return 3;
        }
        else{                       //if there's no file name to change
            sprintf(result_buff, "250 RNTO command succeeds.\n");   //succeeded in changing filename
            write(1, result_buff, strlen(result_buff));
            rename(argument[0], argument[1]);
            sleep(0.5);
            write(connfd, result_buff, MAX_BUF);
            //write log file
            memset(logstr, 0, 500);
            log_info(logstr, cliaddr, ip, user);
            fprintf(logFile, "%s %s", logstr, result_buff);
            fflush(logFile);
            suc = 1;
            memset(result_buff, 0, MAX_BUF);
            return 3;
        }

    }
    else if(!strcmp(command, "RETR"))           //case of RETR
    {
        sleep(0.5);
        memset(result_buff, 0, MAX_BUF);
        sprintf(result_buff, "150 Opening binary/ascii mode data connection for %s\n", argument[0]);
        /*write log file*/

        memset(logstr, 0, 500);
        log_info(logstr, cliaddr, ip, user);
        fprintf(logFile, "%s %s", logstr, result_buff);
        fflush(logFile);
        write(1, result_buff, strlen(result_buff));
        write(connfd, result_buff, MAX_BUF);
        memset(logstr, 0, 500);
        
        //get filename
        char arg[MAX_BUF];
        snprintf(arg, sizeof(arg), "%s/%s", getcwd(NULL, 0), argument[0]);

        //open file
        FILE*temp_fp;
        temp_fp = fopen(arg, "r");

        if(temp_fp == NULL){    //failed to open
            write(1, "Failed to open", 15);
            strcpy(result_buff, "NoF#\0");
            return 7;
        }
        else{
            memset(result_buff, 0, MAX_BUF);
            char tempstr[MAX_BUF];
            while((fgets(tempstr, MAX_BUF, temp_fp) != NULL)){  //open file and get contents
                tempstr[strlen(tempstr)] = '\0';
                strcat(result_buff, tempstr);                   
                memset(tempstr, 0, MAX_BUF);
            }
        }
    }

    else if(!strcmp(command, "STOR"))           //case of STOR
    {

        int bytes_read;
        bytes_read = read(connfd, result_buff, MAX_BUF);
        
        result_buff[strlen(result_buff)] = '\0';
        if(result_buff[0] == 'N'&& result_buff[1] == 'o' && result_buff[2] == 'F'&& result_buff[3] == '#'){ //if there is no file
            write(1, "Failed to open\n", 16);            
            memset(result_buff, 0, MAX_BUF);
            return 10;
        }
        //open file
        FILE*temp_fp = fopen(argument[0], "w");
        fprintf(temp_fp, "%s", result_buff);
        fflush(temp_fp);
        return 1;
        
    }
    else if(!strcmp(command, "TYPE") && argument[0][0] == 'A') {   //case of TYPE A

        sprintf(result_buff, "201 Type set to A\n");
        write(1, result_buff, strlen(result_buff));  
        return 1;
    }
    else if(!strcmp(command, "TYPE") && argument[0][0] == 'I') {   //case of TYPE I


        sprintf(result_buff, "201 Type set to I\n");
        write(1, result_buff, strlen(result_buff));
        return 1;
    }
    else if(!strcmp(command, "NLST") || !strcmp(command, "LIST")){       //case of NLST or list
        if(dir_cnt > 1) {
            error_handling(4, result_buff);      //too many directory
            return 0;
        }
        struct stat file;
         ////////////////// if the argument is file //////////////////////////
        if(access(directory, F_OK) == -1){

            if(correct_argument(directory, result_buff) < 0) return 0;//check if i can open the directory
        }
        /*               case of no read permission             */
        char per[100];
        GetPermission(file, directory, per);
        if(per[0] == '-')
        {    
            strcpy(result_buff, "cannot access\n\0");
            return 1;
        }
        
        char filetype = GetFiletype(file, directory); //get filetype
        /*                      If directory is file                 */
        if(filetype == '-' ){        
            if(!lflag) {
                strcpy(result_buff, "need -l option");
                return 0;
            }
//            write(1, "NLST -l", 8);
//            write(1, "\t[", 3);
//            write(1, temp_pid, strlen(temp_pid));
//            write(1, "]\n", 3);

            option_l(directory, result_buff);
            chdir(current_directory);
            return 1;
        }
        
        char**filenames = (char**)malloc(sizeof(char*)*MAX_BUF);
        for(int i = 0; i<MAX_BUF; i++){
            filenames[i] = (char*)malloc(sizeof(char)*MAX_BUF);
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

        char**temp_filenames = (char**)malloc(sizeof(char*)*MAX_BUF);
        for(int i = 0; i<MAX_BUF; i++){
            temp_filenames[i] = (char*)malloc(sizeof(char)*MAX_BUF);
            strcpy(temp_filenames[i], filenames[i]);
        }
        
        ArrangeFilenames(filenames, temp_filenames, 0, filecnt-1);//            arrange files 

        /************************** start nlst -al **************************/
        if(aflag && lflag){
    //        write(1, "NLST -al", 9);
     //       write(1, "\t[", 3);
     //       write(1, temp_pid, strlen(temp_pid));
     //       write(1, "]\n", 3);
            
            for(int i = 0; i<filecnt; i++){
                option_l(filenames[i], result_buff);
            }
            chdir(current_directory);
            return 1;
        }
        /////////////////////////////////////////////////////////////////////

        /************************** start nlst -a **************************/
        if(aflag){
    //        write(1, "NLST -a", 8);
    //        write(1, "\t[", 3);
    //        write(1, temp_pid, strlen(temp_pid));
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
    //        write(1, "NLST -l", 8);
    //        write(1, "\t[", 3);
            //write(1, temp_pid, strlen(temp_pid));
    //        write(1, "]\n", 3);

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
    //        write(1, "LIST", 5);
    //        write(1, "\t[", 3);
            //write(1, temp_pid, strlen(temp_pid));
    //        write(1, "]\n", 3);

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
    //        write(1, "NLST", 5);
    //        write(1, "\t[", 3);
            //write(1, temp_pid, strlen(temp_pid));
    //        write(1, "]\n", 3);
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



/////////////////////////////////////////////////////////////////
// convert_str_to_addr                                         //
// ========================================================    //
// Input : char *str, unsigned int *port                       //
//        (Input parameter Description)                        //
// Output : char *                                             //
//        (Out parameter Description)                          //
// Purpose : get ip address, port number from port message     //
/////////////////////////////////////////////////////////////////
char * convert_str_to_addr(char *str, unsigned int *port)
{
    char *addr = (char*)malloc(sizeof(char) * MAX_BUF);
    memset(addr, 0, MAX_BUF);
    for(int i = 0; i<strlen(str); i++){
        if(str[i] == ' ')
            str[i] = ',';
    }
    char *ptr = strtok(str, ",");
    int num = 0;
    /*************** start slicing **************/
    while(ptr != NULL) {
        if(num == 6) {     //get port number
            int two = 1;
            int number = atoi(ptr); //convert to integer
            for(int i = 7; i >= 0; i--) {
                int left = number % 2;
                number = number /2; 
                *port += left * two;    //get port number
                two *= 2;
            }
        }
        else if(num == 5) { //get port number
            int two = 256;
            int number =atoi(ptr);  //convert to integer
            for(int i = 0; i<=7; i++){
                int left = number %2;
                number = number /2;     
                *port += left * two;    //get port number
                two*= 2;
            }
        }
        else if(num >= 1){  //get ip address
            strcat(addr, ptr);
            addr[strlen(addr)] = '.';
        }
        num++;
        ptr = strtok(NULL, ",");
    }
    /******************************************/
    addr[strlen(addr)] = '\0';
    return addr;    //return ip address
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
int client_info(struct sockaddr_in client_addr, char*ip){
    write(1, "** Client is trying to connect **\n", 35);
    /********************** client IP address ************************/
    write(1, " - IP : ", 9);
    write(1, ip, strlen(ip));
    write(STDOUT_FILENO, "\n", 2);
    //////////////////////////////////////////////////////////////////

    /********************** client port number ***********************/
    char client_port[100];
    sprintf(client_port, "%d", client_addr.sin_port);
    write(1, " - Port : ", 11);
    write(STDOUT_FILENO, client_port, strlen(client_port));
    write(STDOUT_FILENO, "\n", 2);
    //////////////////////////////////////////////////////////////////
    return 1;
}

//////////////////////////////////////////////////////////////
// user_match                                               //
// ======================================================== //
// Input : char*user, char*passwd                           //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : check if the username and password are correct //
//////////////////////////////////////////////////////////////
int user_match(char*user, char*passwd)
{   
    FILE *fp;
    struct passwd *pw;
    fp = fopen("passwd", "r");  //open passwd
    if(fp == NULL) {
        return 0;
    }
    /*********check if password and user name is efficient*********/
    while((pw = fgetpwent(fp))!= NULL) {
        if(!strcmp(user, pw->pw_name)) {    //if username if correct
            if(!strcmp(passwd, pw->pw_passwd)) {    //if password is correct
                return 1;
            }
        }
    }
    /**************************************************************/
    return 0;
}
//////////////////////////////////////////////////////////////
// get_time                                                 //
// ======================================================== //
// Input : char []                                          //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : return current time                            //
//////////////////////////////////////////////////////////////
void get_time(char time_str[100])
{
    memset(time_str, 0, 100);
    static char welcome_msg[150];
    time_t curr_time;
    struct tm* time_info;

    //get time information
    time(&curr_time);
    time_info = localtime(&curr_time);

    // strncpy year from asctime() return string
    strncpy(time_str, asctime(time_info), 24);
    time_str[strlen(time_str)] = '\0';
}

//////////////////////////////////////////////////////////////
// log_auth                                                 //
// ======================================================== //
// Input : int connfd                                       //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : check if user can log-in                       //
//////////////////////////////////////////////////////////////
int log_auth(int connfd, struct sockaddr_in client_addr, char*ip, char user[100])
{
    char passwd[MAX_BUF];
    int n, count = 1;

    while(1) {
        char temp[10];
        sprintf(temp, "%d", count);
        write(1, "** User is trying to log-in (", 30);
        write(1, temp, strlen(temp));
        write(1, "/3) **\n", 8);

        memset(user, 0, 100);
        memset(passwd, 0, MAX_BUF);
    /****************   read username and password form client *************/
        n = read(connfd, user, MAX_BUF);    //get username
        user[strlen(user)] = '\0';

        FILE *fp;
        struct passwd *pw;
        fp = fopen("passwd", "r");  //open passwd
        if(fp == NULL) {
            return 0;
        }
        
        int u = 5;
        for(u; u<strlen(user); u++) {
            user[u-5] = user[u];
        }
        user[u-5] = '\0';

        /*
        *check if name is valid
        */
        int find = 0;
        while((pw = fgetpwent(fp))!= NULL) {
            if(!strcmp(user, pw->pw_name)) {
                find = 1;
            }
        }
        
        if(find){          //331 : password is required
            char temp[MAX_BUF];
            memset(temp, 0, 100);
            char temp_log[500];
            log_info(temp_log, client_addr, ip, user);
            sprintf(temp, "%s 331 Password is required for %s\n", temp_log, user);
            fprintf(logFile, temp, strlen(temp));
            fflush(logFile);
            sprintf(temp, "331 Password is required for %s\n", user);
            write(1, temp, strlen(temp));
            write(connfd, temp, MAX_BUF);



            n = read(connfd, passwd, MAX_BUF);
            passwd[strlen(passwd)] = '\0';

            /*
            *       strtok passwd
            */
            int p = 5; 
            for(p; p<strlen(passwd); p++){
                passwd[p-5] = passwd[p];
            }
            passwd[p-5] = '\0';
            

            if(n = (user_match(user, passwd)) == 1) {   //if succeeded in usermatch

                sleep(0.5);
                write(connfd, "OK", MAX_BUF);

                sleep(0.5);
                char temp[MAX_BUF];
                memset(temp, 0, 100);
                char temp_log[500];     //get login information
                log_info(temp_log, client_addr, ip, user);
                sprintf(temp, "%s 230 User %s logged in\n", temp_log, user);
                fprintf(logFile, temp, strlen(temp));
                fflush(logFile);
                sprintf(temp, "230 User %s logged in\n", user);
                write(1, temp, strlen(temp));
                write(connfd, temp, MAX_BUF);

                return 1;
            }

            else if(n == 0) {       //case of no client information
                if(count >= 3) {    /* 3 time failed*/
                    sleep(0.5);
                    write(connfd, "DISCONNECTION", MAX_BUF); //send disconnection

                    write(1, "** Log-in failed **\n", 21);
                    char temp[MAX_BUF];
                    memset(temp, 0, 100);
                    char temp_log[500];
                    log_info(temp_log, client_addr, ip, user);
                    sprintf(temp, "%s 530 Failed to log-in\n", temp_log);
                    fprintf(logFile, temp, strlen(temp));
                    fflush(logFile);
                    sprintf(temp, "530 Failed to log-in\n");
                    write(1, temp, strlen(temp));
                    write(connfd, temp, MAX_BUF);
                    return 0;
                }
                /****************re-try*****************/
                write(1, "** Log-in failed **\n", 21);
                sleep(0.5);
                write(connfd, "FAIL", MAX_BUF);
                //write logfile
                char temp[MAX_BUF];
                memset(temp, 0, 100);
                char temp_log[500];
                log_info(temp_log, client_addr, ip, user);
                sprintf(temp, "%s 430 Invalid username of password\n", temp_log);
                fprintf(logFile, temp, strlen(temp));
                fflush(logFile);
                sprintf(temp, "430 Invalid username of password\n");
                write(1, temp, strlen(temp));
                write(connfd, temp, MAX_BUF);
                count++;
                continue;
                /**************************************/
            }

        }
        else {              //invalid username or password
            if(count >= 3) {    /* 3 time failed*/
                sleep(0.5);
                write(1, "** Log-in failed **\n", 21);
                char temp[MAX_BUF];
                memset(temp, 0, 100);
                char temp_log[500];
                log_info(temp_log, client_addr, ip, user);
                sprintf(temp, "%s 530 Failed to log-in\n", temp_log);
                fprintf(logFile, temp, strlen(temp));
                fflush(logFile);
                sprintf(temp, "530 Failed to log-in\n");
                write(1, temp, strlen(temp));
                write(connfd, temp, MAX_BUF);
                return 0;
            }  
            //case of failed to log in
            char temp[MAX_BUF];
            memset(temp, 0, 100);
            char temp_log[500];
            log_info(temp_log, client_addr, ip, user);
            sprintf(temp, "%s 430 Invalid username of password\n", temp_log);
            fprintf(logFile, temp, strlen(temp));
            fflush(logFile);
            sprintf(temp, "430 Invalid username of password\n");
            write(1, temp, strlen(temp));
            write(connfd, temp, MAX_BUF);
            count++;
            continue;
        }  
    }
    return 1;
}

//////////////////////////////////////////////////////////////
// log_info                                                 //
// ======================================================== //
// Input : struct sockaddr_in client_addr,char*user,char*ip //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : return log_information                         //
//////////////////////////////////////////////////////////////
void log_info(char logInfo[500], struct sockaddr_in client_addr, char*ip, char*user){
    char temp_time[100];
    get_time(temp_time);    //get time information

    char client_port[50];
    sprintf(client_port, "%d", client_addr.sin_port);   //get client information

    sprintf(logInfo, "%s [%s:%s] %s", temp_time, ip, client_port, user);    //return log file information
    logInfo[strlen(logInfo)] = '\0';

}
//////////////////////////////////////////////////////////////
// Ip_Slicing                                               //
// ======================================================== //
// Input : char *str, char**sliced                          //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : slice ip address by dot(.)                     //
//////////////////////////////////////////////////////////////
void Ip_Slicing(char *str, char**sliced) {
    int num = 0;
    char *ptr = strtok(str, ".");
    /*************** start slicing **************/
    while(ptr != NULL) {
        strcpy(sliced[num++], ptr);//strcat sliced string
        ptr = strtok(NULL, ".");
    }
}

/////////////////////////////////////////////////////////////////
// main                                                        //
// ========================================================    //
// Input : int argc, char **argv                               //
//        (Input parameter Description)                        //
// Output : int                                                //
//        (Out parameter Description)                          //
// Purpose : get message from control connection and send      //
//           message using data connection                     //
/////////////////////////////////////////////////////////////////
int main(int argc, char **argv) 
{

    char buf[MAX_BUF];
    /*
    *
    *       open logfile
    *
    */  
    logFile = fopen("logfile", "w");
    if(logFile == NULL){
        write(1, "failed to open", 15);
    }
    
    /*
    *
    *       open access.txt
    *
    */    
   
    FILE *fp_checkIP;   //FILE stream to check client's IP
    fp_checkIP = fopen("access.txt", "r");
    if(fp_checkIP == NULL) {
        printf("Error: cannot open access file\n");
        //close(connfd);
    }

    

    /*
    *
    *       prepare control connection
    *
    */

    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//set address
    servaddr.sin_port = htons(atoi(argv[1]));//set port
        
    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){  //bind socket
        printf("Server: Can't bind local address\n");
        return 0;
    }   

    listen(listenfd, 5);   //listen from client

    int clilen= sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);



    /*
    *
    *   log server is started
    *
    */    

    get_time(cur_time);
    fprintf(logFile, "%s Server is started\n", cur_time);
    fflush(logFile);


    /*
    *
    *       get ip address from client
    *
    */
    char client_ip[MAX_BUF];
    int n = read(connfd, client_ip, MAX_BUF);
    client_ip[n] = '\0';
        
    //print information of client//
    //client_info(cliaddr, client_ip);







    /*
    *
    *        check if client ip is acceptable
    * 
    */
    char IPs[MAX_BUF];
    int find = 0;
    while((fgets(IPs, MAX_BUF, fp_checkIP) != NULL))
    {
        find = 1;
        if(IPs[strlen(IPs)-1] == '\n')
            IPs[strlen(IPs)-1] = '\0';
        else
            IPs[strlen(IPs)] = '\0';

        /*
        *       allocate string array
        */
        char **access_IP = (char**)malloc(sizeof(char*) * 5);
        for(int i = 0; i<5; i++){
        access_IP[i] = (char*)malloc(sizeof(char) * 10);
        }

        char **input_IP = (char**)malloc(sizeof(char*) * 5);
        for(int i = 0; i<5; i++){
        input_IP[i] = (char*)malloc(sizeof(char) * 10);
        }

        /*
        *       Slicing IP address by dot(.)
        */
        Ip_Slicing(IPs, access_IP);
        Ip_Slicing(client_ip, input_IP);
        
        /*
        *       Check string
        */    
        for(int i = 0; i<4; i++){
            if(!strcmp(input_IP[i], "*"))   //wildcard
                continue;
            if(!strcmp(access_IP[i], "*"))  //wildcard
                continue;
            if(strcmp(access_IP[i], input_IP[i]))   //not equal string
                find = 0;
        }

        /*
        *       Free string array
        */
        for(int i = 0; i<5; i++){
            free(access_IP[i]);
        }
        free(access_IP);
        
        for(int i = 0; i<5; i++){
            free(input_IP[i]);
        }
        free(input_IP);

        /*
        *       Client connected
        */
        if(find) {
            write(1, "** Client is connected **\n", 27);
            write(connfd, "ACCEPTED", 9);
            char welcome_msg[200];
            get_time(cur_time);

            read(connfd, buf, MAX_BUF);//NOTHING

            sprintf(welcome_msg, "sswlab.kw.ac.kr FTP server (version myftp [1.0] %s) ready.\n", cur_time);
            write(1, welcome_msg, strlen(welcome_msg));
            fprintf(logFile, "%s", welcome_msg);
            fflush(logFile);
            write(connfd, welcome_msg, strlen(welcome_msg));
            break;
        }
    }

    /*
    *   Failed to connect
    */
    if(!find) {
        write(1, "** It is not authenticated client **\n", 38);
        write(connfd, "REJECTION", 10);
        memset(buf, 0, MAX_BUF);
        read(connfd, buf, MAX_BUF);

        char temp_info[500];

        write(1, "431 This client can't access. Close the session.\n", 50);
        fprintf(logFile, "431 This client can't access. Close the session.\n");
        fflush(logFile);
        write(connfd, "431 This client can't access. Close the session.\n", MAX_BUF);
        close(connfd);
        exit(0);
    }


    /*
    *        log-in failed
    */
    char user[100];
    if(log_auth(connfd, cliaddr, IPs, user) == 0) {     //log-in failed
        write(1, "** Fail to log-in **\n", 22);
        get_time(cur_time);
        fprintf(logFile, "%s Server is terminated\n", cur_time);
        exit(0);
        close(connfd);
    //    continue;
    }
    else {
        /******log-in success*******/
        write(1, "** Success to log-in **\n", 25);
        //close(connfd);
        /**************************/
    }
    


    /**
     * 
     * Get command from control connection
     * 
    */
   for(;;){
        memset(buf, 0, MAX_BUF);
        read(connfd, buf, MAX_BUF);
        buf[strlen(buf)] = '\0';
        char result_buff[MAX_BUF];
        memset(result_buff, 0, MAX_BUF);

        char logstr[500];


        int stor = 0;
        if(buf[0] == 'S' && buf[1] == 'T' && buf[2] == 'O' && buf[3] == 'R')
            stor = 1;
        if(buf[0] == 'N' && buf[1] == 'L' && buf[2] == 'S' && buf[3] == 'T' || buf[0] == 'R' && buf[1] == 'E' && buf[2] == 'T' && buf[3] == 'R' || stor == 1) {
        /*****************  receive random port number by control connection   ****************/
        char temp[MAX_BUF];
        char *host_ip = (char*)malloc(sizeof(char) * MAX_BUF);
        unsigned int port_num = 0;

        write(connfd, "dummy", MAX_BUF);
                
        n = read(connfd, temp, MAX_BUF);    //receive port command
        temp[strlen(temp)] = '\0';
        write(1, temp, MAX_BUF);
        write(1, "\n", 2);
        host_ip = convert_str_to_addr(temp, (unsigned int *) &port_num);    //convert port number and ip address
        /************************************************************************************/
                
        sleep(0.5);


        /********************* connect data connection *******************************/
        struct sockaddr_in data_temp;
        int data_sockfd;

        data_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        memset(&data_temp, 0, sizeof(data_temp));
        data_temp.sin_family = AF_INET;
        inet_pton(AF_INET, host_ip, &data_temp.sin_addr);
        data_temp.sin_port = htons(port_num);
        if(connect(data_sockfd, (struct sockaddr*) &data_temp, sizeof(data_temp)) < 0){
            write(1, "550 Failed to access\n", 2);
            write(connfd, "550 Failed to access\n", MAX_BUF);

            memset(logstr, 0, 500);
            log_info(logstr, cliaddr, IPs, user);
            fprintf(logFile, "%s 550 Failed to access\n", logstr);
            fflush(logFile);
            memset(logstr, 0, 500);
        }
        /*****************************************************************************/
        
        /************* write message and send message ************/
        sleep(0.5);
        write(1, "200 Port command preformed successfully\n", 41);
        write(connfd, "200 Port command preformed successfully\n", MAX_BUF);

        memset(logstr, 0, 500);
        log_info(logstr, cliaddr, IPs, user);
        fprintf(logFile, "%s 200 Port command preformed successfully\n", logstr);
        fflush(logFile);
        memset(logstr, 0, 500);
        /*********************************************************/


        /************* write message and send message ************/
        sleep(0.5);
        if(buf[0] == 'N' && buf[1] == 'L' && buf[2] == 'S' && buf[3] == 'T') {
            write(1, "150 Opening data connection for directory list\n", 48);    
            write(connfd, "150 Opening data connection for directory list\n", 48);
        }
        if(buf[0] == 'R' && buf[1] == 'E' && buf[2] == 'T' && buf[3] == 'R') {
            int cpn = cmd_process(buf, result_buff, logstr, cliaddr, IPs, user, connfd);    //get file
            write(data_sockfd, result_buff, MAX_BUF);   //send to client
            
            if(cpn == 7)    //faild to open
                continue;
            memset(buf, 0, MAX_BUF);
            read(connfd, buf, MAX_BUF);

            //send message to client
            sleep(0.5);
            write(1, "226 Complete transmission\n", 27);
            write(connfd, "226 Complete transmission\n", 27);

            //write log file
            memset(logstr, 0, 500);
            log_info(logstr, cliaddr, IPs, user);
            fprintf(logFile, "%s 226 Complete transmission\n", logstr);
            fflush(logFile);
            memset(logstr, 0, 500);
            /*******************************************************************************/

            close(data_sockfd);
            continue;
        }
        /*********************************************************/
        if(stor) {      //case of stor
            
            //get argumnet
            char arg[100];
            int fnn = 0;
            for(fnn = 5; fnn < strlen(buf); fnn++){
                arg[fnn-5] = buf[fnn];   
            }
            arg[fnn] = '\0';
            sleep(0.5);
            memset(result_buff, 0, MAX_BUF);
            sprintf(result_buff, "150 Opening binary/ascii mode data connection for %s\n", arg);

            /*write log file*/

            memset(logstr, 0, 500);
            log_info(logstr, cliaddr, IPs, user);
            fprintf(logFile, "%s %s", logstr, result_buff);
            fflush(logFile);
            write(1, result_buff, strlen(result_buff));
            write(connfd, result_buff, MAX_BUF);
            memset(logstr, 0, 500);

            int cpn = cmd_process(buf, result_buff, logstr, cliaddr, IPs, user, data_sockfd);    //get file
            if(cpn == 10)
                exit(0);
            memset(buf, 0, MAX_BUF);
            read(connfd, buf, MAX_BUF);

            //send message to client
            sleep(0.5);
            write(1, "226 Complete transmission\n", 27);
            write(connfd, "226 Complete transmission\n", 27);

            //write log file
            memset(logstr, 0, 500);
            log_info(logstr, cliaddr, IPs, user);
            fprintf(logFile, "%s 226 Complete transmission\n", logstr);
            fflush(logFile);
            memset(logstr, 0, 500);
            /*******************************************************************************/

            close(data_sockfd);
            continue;

        }
        /*write log file*/
        memset(logstr, 0, 500);
        log_info(logstr, cliaddr, IPs, user);
        fprintf(logFile, "%s 150 Opening data connection for directory list\n", logstr);
        fflush(logFile);
        memset(logstr, 0, 500);


        int cp = cmd_process(buf, result_buff, logstr, cliaddr, IPs, user, connfd);  //command processing
        int wn = write(data_sockfd, result_buff, MAX_BUF);   //send result
        if(wn < 0)      //if failed to send result
        {
            write(1, "550 Failed transmission\n", 25);
            write(connfd, "550 Failed transmission\n", 25);

            memset(logstr, 0, 500);
            log_info(logstr, cliaddr, IPs, user);
            fprintf(logFile, "%s 550 Failed transmission\n", logstr);
            fflush(logFile);
            memset(logstr, 0, 500);
        }

        /*********** write message and send message ("226 result successful") **********/
        memset(buf, 0, MAX_BUF);
        read(connfd, buf, MAX_BUF);

        sleep(0.5);
        write(1, "226 Complete transmission\n", 27);
        write(connfd, "226 Complete transmission\n", 27);

        memset(logstr, 0, 500);
        log_info(logstr, cliaddr, IPs, user);
        fprintf(logFile, "%s 226 Complete transmission\n", logstr);
        fflush(logFile);
        memset(logstr, 0, 500);
        /*******************************************************************************/

        close(data_sockfd);
        continue;
    }
    int cp = cmd_process(buf, result_buff, logstr, cliaddr, IPs, user, connfd);  //command processing
    if(cp == 3) {
        memset(logstr, 0, 500);
        continue;
    }
    if(cp == 5) {   //case of QUIT
        close(connfd);      //close socket
        memset(logstr, 0, 500);
        log_info(logstr, cliaddr, IPs, user);
        fprintf(logFile, "%s %s", logstr, result_buff);
        fflush(logFile);
        memset(logstr, 0, 500);   

        exit(0);
    }

    sleep(0.5);
    write(connfd, result_buff, MAX_BUF);    //write to socket
    //write log file
    memset(logstr, 0, 500);
    log_info(logstr, cliaddr, IPs, user);
    fprintf(logFile, "%s %s", logstr, result_buff);
    fflush(logFile);
    memset(logstr, 0, 500);
   }

    return 0;

}   
