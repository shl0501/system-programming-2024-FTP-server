#include<unistd.h>
#include<stdio.h>

//////////////////////////////////////////////////////////////
// File Name : kw2019202032_opt.c                           //
// Date : 2024/03/29                                        //  
// OS : Ubuntu 16.04 LTS 64bits                             //
// Author : Lee Sang Hyeon                                  //
// Student ID : 2019202032                                  //
// -------------------------------------------------------- //
// Title : System Programming Assignment #1-1 (ftp server)  //
// Description : Use getopt() function to parameter parsing //
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// main                                                     //
// ======================================================== //
// Input : int argc                                         //
//         char **argv                                      //
//        (Input parameter Description)                     //
// Output : int 0                                           //
//        (Out parameter Description)                       //
// Purpose : Parameter parsing                              //
//////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    int aflag = 0, bflag = 0;
    char *cvalue = NULL;
    int index, c;
    opterr = 0;

//////////// parameter parsing ///////////////////////////////
    while((c = getopt(argc, argv, "abc:")) != -1)
    {
        switch(c)
        {
            case 'a' :              // case of option a
                aflag += 1;
                break;
            case 'b' :              // case of option b
                bflag += 1;
                break;
            case 'c' :              // case of option c
                cvalue = optarg;
                break;
        }
    }
///////// End of parameter parsing ///////////////////////////

    printf("aflag = %d, bflag = %d, cvalue = %s\n", aflag, bflag, cvalue);//print the result

//////// processing non-options argument ////////////////////   
    while(optind < argc){
        printf("Non-option argument %s\n", argv[optind++]);
    }
/////// End of processing non options argument //////////////

    return 0;
}