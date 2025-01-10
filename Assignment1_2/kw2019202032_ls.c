#include<sys/types.h>
#include<dirent.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>

//////////////////////////////////////////////////////////////
// File Name : kw2019202032_ls.c                            //
// Date : 2024/04/08                                        //  
// OS : Ubuntu 20.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #1-2 (ftp server)  //
// Description : open existing directory and print filenames//
//////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////
// GetFiletype                                              //
// ======================================================== //
// Input : struct stat file                                 //
//         char filenames                                   //
//         char filetype                                    //
//        (Input parameter Description)                     //
// Output : char filetype                                   //
//        (Out parameter Description)                       //
// Purpose : Get filetype                                   //
//////////////////////////////////////////////////////////////
char GetFiletype(struct stat file, char *filename, char filetype)
{
    stat(filename, &file);
    if (S_ISBLK(file.st_mode))
        return 'b';
    else if (S_ISCHR(file.st_mode))
        return 'c';
    else if (S_ISDIR(file.st_mode))
        return 'd';
    else if (S_ISFIFO(file.st_mode))
        return 'p';
    else if (S_ISLNK(file.st_mode))
        return 'l';
    else if (S_ISREG(file.st_mode))
        return '-';
    else if (S_ISSOCK(file.st_mode))
        return 'l';
    return 'N';
}
//////////////////////////////////////////////////////////////
// GetPermission                                            //
// ======================================================== //
// Input : struct stat file                                 //
//         char filenames                                   //
//         char permission                                  //
//        (Input parameter Description)                     //
// Output : void                                            //
//        (Out parameter Description)                       //
// Purpose : Get file permission                            //
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

//////////////////////////////////////////////////////////////////////////
// main                                                                 //
// ========================================================             //
// Input : int argc                                                     //
//         char **argv                                                  //
//        (Input parameter Description)                                 //
// Output : int 0                                                       //
//        (Out parameter Description)                                   //
// Purpose :                                                            //
//            - open existing directory and get filenames               //
//            - do not open not existing file and no access permission  //
//////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    DIR *dirp;
    struct dirent *dir;
    if(argc >= 3)   //if there are more than one directory path
        printf("only one directory path can be processed\n");
//////////////////// check path and open directory ////////////////////
    else if(argc == 1) { //if there is no path
        dirp = opendir(".");        //open current directory
        while((dir = readdir(dirp)) != NULL){
            printf("%s\n", dir->d_name);
        }
        closedir(dirp);
    }   //  end of if
    else if(!(dirp = opendir(argv[1]))) {   //If not able to open directory
        struct stat file;
        stat(argv[1], &file);
        char *permission = (char*)malloc(sizeof(char) *100);
        GetPermission(file, argv[1], permission);//get permission

        char filetype = GetFiletype(file, argv[1], filetype);   //get filetype
        if(filetype != 'N'){    //if the path is existing
            if(permission[0] == '-'){       //if not able to access file
                printf("cannot access '%s' : Access denied\n", argv[1]);
            }   //end of if
        }
        else{   //if there is no permission => not existing directory
            printf("cannot access '%s' : No such directory\n", argv[1]);
        }   //end of else
        free(permission);
    }   //end of else if
    else {
        char *changed_directory = (char*)malloc(sizeof(char) * 1024);
        chdir(argv[1]);     //change working directory input path
        getcwd(changed_directory, 1024);
        dirp = opendir(".");
        while((dir = readdir(dirp)) != NULL){   //read dir and print file name
            printf("%s\n", dir->d_name);
        }
        closedir(dirp);

        free(changed_directory);
    }   //end of else
}