//////////////////////////////////////////////////////////////
// File Name : srv.c                                        //
// Date : 2024/05/25                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #3-2 (server)      //
// Description : make control connection and data connection//
//               send result  using data connection         //
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
int cmd_process(char*buff, char*result_buff){

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
        
        write(1, "QUIT\t[", 7);
        //write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);

        strcpy(result_buff, "QUIT");
        return 1;
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
        
        write(1, "PWD\t[", 6);
        //write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);

        strcpy(result_buff, "\"");
        strcat(result_buff, cur_dir);
        strcat(result_buff, "\" is current directory\n\0");
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
            if(correct_argument(directory, result_buff)<0)
                return 0;
            chdir(directory);
            getcwd(cur_dir, MAX_BUF);
        }
        write(1, command, strlen(command));
        write(1, "\t[", 3);        
        //write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);

        strcpy(result_buff, "\"");
        strcat(result_buff, cur_dir);
        strcat(result_buff, "\" is current directory\n\0");
        return 1;
    }
    else if(!strcmp(command, "CDUP")){
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return 1;
        }
        chdir("..");//change directory to ..
        
        write(1, command, strlen(command));
        write(1, "\t[", 3);
        //write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);
        
        getcwd(cur_dir, MAX_BUF);
        strcpy(result_buff, "\"");
        strcat(result_buff, cur_dir);
        strcat(result_buff, "\" is current directory\n\0");
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
        write(1, "\t[", 3);
        //write(1, temp_pid, strlen(temp_pid));
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
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return 0;
        }
        if(argument_cnt == 0) {
            error_handling(2, result_buff);
            return 0;
        }

        write(1, command, strlen(command));
        write(1, "\t[", 3);
        //write(1, temp_pid, strlen(temp_pid));
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
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return 0;
        }
        if(argument_cnt == 0) {
            error_handling(2, result_buff);
            return 0;
        }
        
        write(1, command, strlen(command));
        write(1, "\t[", 3);
        //write(1, temp_pid, strlen(temp_pid));
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
        if(option_flag || invalid) {
            error_handling(0, result_buff);
            return 0;
        }
        if(argument_cnt >= 3) {
            error_handling(3, result_buff);
            return 0;
        }
        
        write(1, command, strlen(command));
        write(1, "\t[", 3);
        //write(1, temp_pid, strlen(temp_pid));
        write(1, "]\n", 3);

        if(access(argument[1], F_OK)!= -1){ //if there exists name already
            strcpy(result_buff, "Error : name to change already exists\n\0");
            return 0;
        }
        struct stat sttemp;
        if(stat(argument[1], &sttemp) == 0 && S_ISDIR(sttemp.st_mode)){ //failed to rename
            strcpy(result_buff, "Error : name to change already exists\n\0");
            return 0;
        }
        int rname = rename(argument[0], argument[1]);
        
        if(rname == -1){    //failed to rename
            strcpy(result_buff, "Error : name to change already exists\n\0");
            return 0;
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
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;

    /********************* prepare control connection **********************/
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
    /*************************************************************************/


    /*****************  receive random port number by control connection   ****************/
    char temp[MAX_BUF];
    char *host_ip = (char*)malloc(sizeof(char) * MAX_BUF);
    unsigned int port_num = 0;
    
    int n = read(connfd, temp, MAX_BUF);    //receive port command
    temp[strlen(temp)] = '\0';
    write(1, temp, MAX_BUF);
    write(1, "\n", 2);
    host_ip = convert_str_to_addr(temp, (unsigned int *) &port_num);    //convert port number and ip address
    /************************************************************************************/
    
    sleep(0.11);


    /********************* connect data connection *******************************/
    struct sockaddr_in data_temp;
    int data_sockfd;

    data_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&data_temp, 0, sizeof(data_temp));
    data_temp.sin_family = AF_INET;
    inet_pton(AF_INET, host_ip, &data_temp.sin_addr);
    data_temp.sin_port = htons(port_num);
    if(connect(data_sockfd, (struct sockaddr*) &data_temp, sizeof(data_temp)) < 0) printf("connection failed");    //connect with server
    /*****************************************************************************/
    

    /************* write message and send message ************/
    write(1, "200 Port command successful\n", 29);
    write(connfd, "200 Port command successful\n", MAX_BUF);
    /*********************************************************/

    sleep(0.01);

    /******** get command from control connection ********/
    read(connfd, buf, MAX_BUF);
    buf[strlen(buf)] = '\0';

    write(1, buf, strlen(buf));
    write(1, "\n", 2);
    /****************************************************/

    /************* write message and send message ************/
    write(1, "150 Opening data connection for directory list\n", 48);    
    write(connfd, "150 Opening data connection for directory list\n", 48);
    /*********************************************************/
    

    /********* send result of command *********/
    char result_buff[MAX_BUF];
    cmd_process(buf, result_buff);  //command processing
    write(data_sockfd, result_buff, MAX_BUF);   //send result
    /******************************************/

    close(data_sockfd); //close data connection

    /*********** write message and send message ("226 result successful") **********/
    write(1, "226 Result is sent successfully\n", 33);
    write(connfd, "226 Result is sent successfully\n", 33);
    /*******************************************************************************/

    
}