#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>
#define MAX_SIZE 4096
#define DELIM_CHARS " \n"
//////////////////////////////////////////////////////////////
// File Name : srv.c                                        //
// Date : 2024/04/17                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #1-3 (ftp server)  //
// Description : print the result of command                //
//////////////////////////////////////////////////////////////



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
// option_check                                             //
// ======================================================== //
// Input : char *options                                    //
//         int *aflag, *lflag, *invalid                     //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : check the options of command                   //
//////////////////////////////////////////////////////////////
void option_check(char *options, int *aflag, int *lflag, int *invalid){
    //////////// check option start /////////////////
    for(int i = 1; i<strlen(options); i++){
        if(options[i] == 'a')
            *aflag = 1;                 //a option
        else if(options[i] == 'l')
            *lflag = 1;                 //l option
        else
            *invalid = 1;               //invalid option
    }
    ///////////// end of option check //////////////
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
// Input : char *argument, char*command                     //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : check if the argument trying to open is valid  //
//////////////////////////////////////////////////////////////
void correct_argument(char* argument, char*command)
{
    char *BUF = (char*)malloc(sizeof(char)*MAX_SIZE);
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
                strcpy(BUF, "Error : cannot access\n\0");
            }   //end of if
        }
        else{   //if there is no permission => not existing directory
            if(!strcmp(command, "NLST"))    //case of NLST
                strcpy(BUF, "Error : No such file or directory\n\0");
            if(!strcmp(command, "CWD"))     //case of CWD
                strcpy(BUF, "Error : directory not found\n\0");
            if(!strcmp(command, "LIST")){   //case of LIST
                strcpy(BUF, command);
                strcat(BUF, "\n");
                strcat(BUF, "Error : No such file or directory\n\0");
            }
        }   //end of else
        write(1, BUF, strlen(BUF));
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
void option_l(char *argument){
    char*buf = (char*)malloc(sizeof(char) * MAX_SIZE);
    struct stat file;

    //////////////filetype//////////////////////
    char filetype = GetFiletype(file, argument);    //get filetype
    char *temp = (char*)malloc(sizeof(char)*MAX_SIZE);
    sprintf(temp, "%c", filetype);
    write(1, temp, strlen(temp));   //print filetype
    stat(argument, &file);
    
    //////// permission //////////////////
    char *permission = (char*)malloc(sizeof(char)*MAX_SIZE);
    GetPermission(file, argument, permission);    //get permission
    write(1, permission, strlen(permission));     //write permission
    write(1, " ", 1);

    //////// nlink //////////////////////
    int filelink = file.st_nlink;           //get nlink
    sprintf(temp, "%d", filelink);
    write(1, temp, strlen(temp));           //write nlink
    write(1, " ", 1);
    strcpy(temp, "\0");

    //////////////pw_name////////////////////////////
    uid_t owner_id;
    owner_id = file.st_uid;             //get owner of file
    struct passwd *pwd;
    pwd = getpwuid(owner_id);
    strcpy(temp, pwd->pw_name);
    write(1, temp, strlen(temp));       //write owner
    write(1, " ", 1);
    strcpy(temp, "\0");

    /////////////// group id /////////////////////////
    struct group *grp = getgrgid(getgid()); //get group id
    write(1, grp->gr_name, strlen(grp->gr_name));   //write group id
    write(1, " ", 1);

    /////////////// filesize/////////////////////////////
    long long filesize = file.st_size;  //get file size
    sprintf(temp, "%lld", filesize);
    write(1, temp, strlen(temp));   //write filesize
    write(1, " ", 1);
    strcpy(temp, "\0");
    
    /////////////////// file time //////////////////////////
    time_t t = file.st_mtime;       //get file time
    struct tm *time = localtime(&t);

    char time_data[100][100];
    int td_num = 0;

    ////////////////// start parsing time information ////////////
    char *t_ptr = strtok(asctime(time), " ");
    while(t_ptr != NULL){
        if(td_num == 1){
            write(1, t_ptr, strlen(t_ptr)); //write month
            write(1, " ", 1);
            
        }
        else if(td_num == 2){
            write(1, t_ptr, strlen(t_ptr)); //write day
            write(1, " ", 1);
            
        }
        if(td_num == 3){
            for(int i = 0; i<5; i++)
                time_data[td_num][i] = t_ptr[i];
            write(1, time_data[td_num], strlen(time_data[td_num])); //write time
            write(1, " ", 1);
            
        }
        td_num++;
        t_ptr = strtok(NULL, " ");
    }
    //////////// end of parsing time information ///////////////////

    write(1, argument, strlen(argument));   //write file name

    if(filetype == 'd')
    {   
        write(1, "/", 1);   //if it is directory, write /
    }

    write(1, "\n", 1);
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
// main                                                     //
// ======================================================== //
// Input : int argc, char **argv                            //
//        (Input parameter Description)                     //
// Output : int                                             //
//        (Out parameter Description)                       //
// Purpose : option check                                   //
//           command check                                  //
//           argument check                                 //
//           print the result                               //
//////////////////////////////////////////////////////////////
int main(int argc, char **argv){
    char buffer[MAX_SIZE];
    ssize_t bytes_read = read(STDIN_FILENO, buffer, MAX_SIZE);

    char *command = (char*)malloc(sizeof(char)*MAX_SIZE);
    char *option = (char*)malloc(sizeof(char)*MAX_SIZE);
    int option_flag = 0;
    char ** argument = (char**)malloc(sizeof(char*)*MAX_SIZE);
    for(int i = 0; i<MAX_SIZE; i++){
        argument[i] = (char*)malloc(sizeof(char)*MAX_SIZE);
    }
    int arg_cnt = 0;
    
    ///////////// start input parsing ////////////////////////
    char *ptr;
    char *next_ptr;
    ptr = strtok_r(buffer, DELIM_CHARS, &next_ptr);
    int ptr_cnt = 0;
    while(ptr){
        if(ptr_cnt == 0)
            strcpy(command, ptr);   //get the command
        else if(ptr_cnt == 1){      // get the argument
            strcpy(argument[arg_cnt++], ptr);
            strcpy(argument[arg_cnt++], argument[0]);
        }
        else
            strcpy(argument[arg_cnt++], ptr);
        ptr_cnt += 1;
        ptr = strtok_r(NULL, DELIM_CHARS, &next_ptr);
    }
    /////////////// end parsing input ///////////////////////
    

    //////////////  start option check ///////////////////////
    int aflag =0 ;
    int lflag = 0;
    int invalid = 0;

    if(argument[0][0] == '-')
    {
        option_flag = 1;
        option_check(argument[0], &aflag, &lflag, &invalid);
    }
    /////////////   end option check ////////////////////////

    DIR *dirp;
    struct dirent *dir;


    char *cur_dir = (char*)malloc(sizeof(char)*MAX_SIZE);
    /////////////////////// start processing command ///////////////
    if(!strcmp(command, "NLST") || !strcmp(command, "LIST")){  //case of NLST and LIST
    /////////////// error ///////////////////////////
        if(invalid) error_handling(0);  
        if(arg_cnt >=3 && !option_flag) error_handling(4);
        strcpy(argument[0], ".");
    
    ////////////// check directory path ////////////////////
        int argument_starting_number = 0;
        if(arg_cnt == 0)
            argument_starting_number = 0;
        else if(option_flag && arg_cnt == 2)
            argument_starting_number = 0;
        else if(option_flag)
            argument_starting_number = 2;
        else
            argument_starting_number = 1;

    ////////////////// if the argument is file //////////////////////////
        if(access(argument[argument_starting_number], F_OK)== -1 ){
            correct_argument(argument[argument_starting_number], command);  //check if i can open the directory
            
        }

        ////////////////////// case of no read permission   ///////////////////
        struct stat file;
        char per[100];
        GetPermission(file, argument[argument_starting_number], per);
        if(per[0] == '-')
        {    
            write(1, "cannot access\n\0", 16);
            exit(0);
        }
        char filetype = GetFiletype(file, argument[argument_starting_number]);

        ////////////////// case of file /////////////////////////////
        if(filetype == '-' && arg_cnt != 0)
        {
            if(!lflag)
                exit(0);
            write(1, command, strlen(command));
            write(1, " -l", 5);
            write(1, "\0", 2);
            write(1, "\n", 2);
            option_l(argument[argument_starting_number]);
            exit(0);
        }
        


        char **filenames = (char**)malloc(sizeof(char*)*MAX_SIZE);
        for(int i = 0; i<MAX_SIZE; i++){
            filenames[i] = (char*)malloc(sizeof(char)*MAX_SIZE);
        }
        int filecnt = 0;

        /////////// get the files of directory ////////////////////
        dirp = opendir(argument[argument_starting_number]);
        while((dir = readdir(dirp)) != NULL){
            if(aflag)       //if a option
                strcpy(filenames[filecnt++], dir->d_name);
            else{
                if(dir->d_name[0] != '.')   //if not a option
                    strcpy(filenames[filecnt++], dir->d_name);
            }
        }
        closedir(dirp);

        chdir(argument[argument_starting_number]);  //change directory

        for(int i = 0; i<MAX_SIZE; i++){
            free(argument[i]);
        }
        free(argument);

        char**temp_filenames = (char**)malloc(sizeof(char*)*MAX_SIZE);
        for(int i = 0; i<MAX_SIZE; i++){
            temp_filenames[i] = (char*)malloc(sizeof(char)*MAX_SIZE);
            strcpy(temp_filenames[i], filenames[i]);
        }
        //////// arrange files /////////////////////////////////////////
        ArrangeFilenames(filenames, temp_filenames, 0, filecnt-1);
        write(1, command, strlen(command));
        if(!strcmp(command, "LIST")){   //case of list
            write(1, "\n\0", 3);
            for(int i = 0; i<filecnt; i++){
                option_l(filenames[i]);
            }
            exit(0);
        }    
        if(aflag != 0 && lflag != 0){       //case of al option
            write(1, " -al\n\0", 7);
            for(int i = 0; i<filecnt; i++){
                option_l(filenames[i]);
            }
            exit(0);
        }
        else if(aflag){
            write(1, " -a\n\0", 6);     //case of a option

            int c_num = 0;
            for(int i = 0; i<filecnt; i++){
                if(c_num == 5)
                {
                    c_num = 0;
                    write(1, "\n", 2);
                }
                char filetype = GetFiletype(file, filenames[i]);
                if(filetype == 'd'){        //if it is directory
                    write(1, filenames[i], strlen(filenames[i]));
                    write(1, "/ ", 2);
                }
                else{
                    write(1, filenames[i], strlen(filenames[i]));
                    write(1, " ", 2);
                }
                c_num++;
            }
            write(1, "\n\0", 3);
            exit(0);
        }
        else if(lflag){     //case of l option
            write(1, " -l\n\0", 6);
            for(int i = 0; i<filecnt; i++){
                option_l(filenames[i]);
            }
            exit(0);
        }
        else{               //case of no option
            write(1, "\n\0", 3);
            int c_num = 0;
            for(int i = 0; i<filecnt; i++){
                if(c_num == 5)
                {
                    c_num = 0;
                    write(1, "\n", 2);
                }
                char filetype = GetFiletype(file, filenames[i]);    //get filetype
                if(filetype == 'd'){    //if directoory path
                    write(1, filenames[i], strlen(filenames[i]));
                    write(1, "/ ", 3);
                }
                else{   //not directory path
                    write(1, filenames[i], strlen(filenames[i]));
                    write(1, " ", 2);
                }
                c_num++;
            }
            write(1, "\n\0", 3);
            exit(0);


        }

    }
    else if(!strcmp(command, "PWD")){   //print working directory
        /////////// handling error /////////////////////////////
        if(option_flag || invalid) error_handling(0);
        else if(arg_cnt >= 1) error_handling(1);
        char *BUF = (char*)malloc(sizeof(char)*MAX_SIZE);
        //////////////get current working directory and print ///////////
        getcwd(cur_dir, MAX_SIZE);
        strcpy(BUF, "\"");
        strcat(BUF, cur_dir);
        strcat(BUF, "\" is current directory\n\0");
        write(1, BUF, strlen(BUF));
        exit(0);
    }
    else if(!strcmp(command, "CWD")){   //change directory
        char *BUF = (char*)malloc(sizeof(char)*MAX_SIZE);
        ///////// error check/////////////////////////////
        if(option_flag || invalid) error_handling(0);
        else if(arg_cnt == 0) error_handling(2);
        else{           //move to input directory
            correct_argument(argument[1], command);
            chdir(argument[1]);
            getcwd(cur_dir, MAX_SIZE);
        }
        write(1, command, strlen(command));
        write(1, " ", strlen(" "));
        write(1, argument[1], strlen(argument[1]));
        write(1, "\n", strlen("\n"));
        strcpy(BUF, "\"");
        strcat(BUF, cur_dir);
        strcat(BUF, "\" is current directory\n\0");
        write(1, BUF, strlen(BUF));
        exit(0);
    }
    else if(!strcmp(command, "CDUP")){  //cd ..
        if(option_flag || invalid) error_handling(0);
        chdir("..");//change directory to ..
        write(1, command, strlen(command));
        write(1, "\n", 2);
        char *BUF = (char*)malloc(sizeof(char)*MAX_SIZE);
        getcwd(cur_dir, MAX_SIZE);
        strcpy(BUF, "\"");
        strcat(BUF, cur_dir);
        strcat(BUF, "\" is current directory\n\0");
        write(1, BUF, strlen(BUF));
        exit(0);
    }
    else if(!strcmp(command, "MKD")){   //make directory
    /////////////// error handling ////////////////////////
        if(option_flag || invalid) error_handling(0);
        if(arg_cnt == 0) error_handling(2);
    ////////////////// make directory //////////////////////
        for(int i = 1; i<arg_cnt; i++){
            char *BUF = (char*)malloc(sizeof(char)*MAX_SIZE);
            if(!(dirp = opendir(argument[i]))){
                mkdir(argument[i], 0777);
                strcat(BUF, "MKD ");
                strcat(BUF, argument[i]);
                strcat(BUF, "\n\0");
                write(1, BUF, strlen(BUF));
            }
            else {  //failed to make directory
                write(1, "Error : cannot create directory '", 34);
                write(1, argument[i], strlen(argument[i]));
                write(1, "': File exists\n\0", 17);
            }
        }
        exit(0);
    }
    else if(!strcmp(command, "DELE")){  //delete
    ///////////////error handling ///////////////////
        if(option_flag || invalid) error_handling(0);
        if(arg_cnt == 0) error_handling(2);
        for(int i = 1; i<arg_cnt; i++){
            int un = unlink(argument[i]);
            if(un == -1){
                write(1, "Error : failed to delete '", 27); //failed to unlink
                write(1, argument[i], strlen(argument[i]));
                write(1, "'\n\0", 4);
            }
            if (un == 0){   //succeed in unlinking
                write(1, "DELE ", 6);
                write(1, argument[i], strlen(argument[i]));
                write(1, "\n\0", 3);
            }
        }
        exit(0);
    }
    else if(!strcmp(command, "RMD")){   //remove directory
        if(option_flag || invalid) error_handling(0);
        if(arg_cnt == 0) error_handling(2);
        for(int i = 1; i<arg_cnt; i++){
            int rm = rmdir(argument[i]);
            if(rm == -1){   //failed to remove
                write(1, "Error : failed to remove '", 27);
                write(1, argument[i], strlen(argument[i]));
                write(1, "'\n\0", 4);
            }
            if (rm == 0){   //succeeded in removing
                write(1, "RMD ", 5);
                write(1, argument[i], strlen(argument[i]));
                write(1, "\n\0", 3);
            }
        }
        exit(0);
    }
    else if(!strcmp(command, "RNFR")){  //rename
        if(option_flag || invalid) error_handling(0);
        if(arg_cnt != 3) error_handling(3);
        char* buf = (char*)malloc(sizeof(char)*MAX_SIZE);
        strcat(buf, command);
        strcat(buf, " ");
        strcat(buf, argument[1]);
        strcat(buf, "\n");
        strcat(buf, "RNTO");
        strcat(buf, " ");
        strcat(buf, argument[2]);
        strcat(buf, "\n\0");
        if(access(argument[2], F_OK)!= -1){ //if there exists name already
            write(1, "Error : name to change already exists\n\0", 40);
            exit(0);
        }
        struct stat sttemp;
        if(stat(argument[2], &sttemp) == 0 && S_ISDIR(sttemp.st_mode)){ //failed to rename
            write(1, "Error : name to change already exists\n\0", 40);
            exit(0);
        }
        int rname = rename(argument[1], argument[2]);
        
        if(rname == -1){    //failed to rename
            write(1, "Error : name to change already exists\n\0", 40);
            exit(0);
        }
        if(rname == 0){ //succeeded in renaming
            write(1, buf, strlen(buf));
            exit(0);
        }
    }
    else if(!strcmp(command, "QUIT")){  //quit
        if(option_flag || invalid) error_handling(0);
        else if(arg_cnt >= 1) error_handling(1);
        write(1, "QUIT success\n\0", 15);   //quit the program
        exit(0);
    }
    else{
        write(1, "ERROR : NO COMMAND\n\0", 21);
    }
}