#define _GNU_SOURCE

#define _XOPEN_SOURCE 700
#include <ftw.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pwd.h>
#include <grp.h>
#include <stdint.h>
#include <libgen.h>
#include <fcntl.h>
#include <getopt.h>
#include "simul_utils.h"


#ifndef USE_FDS
#define USE_FDS 15
#endif


int i_lower_flag = 0;
int r_upper_flag = 0;
int s_lower_flag = 0;
int u_lower_flag = 0;
int v_lower_flag = 0;
char *source;
char *dest;

char buffer_read[1000];

/*
Nomes: Jéssica Genta dos Santos - DRE: 111031073
       Juan Augusto Santos de Paula - DRE: 111222844
       Oliver Sartori - DRE: 113020549 
*/

int print_copied_file(const char* filepath, const char* aux){
    if(v_lower_flag)
    {
        printf("\"%s\" -> \"%s\" \n",filepath, aux);
    }

    return 0;
}


int configure_print(const struct stat *info, const int typeflag, const char *filepath)
{

    char *part;

    char *aux = malloc(1024*sizeof(char));

    strcpy (aux,dest);


    part = repl_str(filepath, source, "" );

    strcat(aux, "/");
    strcat(aux, part);

    struct stat st;
    struct stat st2;

    if(typeflag==FTW_D && r_upper_flag)
    {
        if (stat(aux, &st) == -1)
        {
            mkdir(aux, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

    }





    char overwrite;
    if(!(typeflag == FTW_D))
    {
        aux[strlen(aux)-1] = '\0';


        int compare = 0; //If not u flag, compare equals to zero


        if (stat(aux, &st) == 0)
        {
            if(u_lower_flag)
            {
                struct timeval seconds_since_1970;
                TIMESPEC_TO_TIMEVAL(&seconds_since_1970, &st.st_mtim);
                if(stat(filepath, &st2) == 0)
                {
                    struct timeval seconds_since_1970_2;
                    TIMESPEC_TO_TIMEVAL(&seconds_since_1970_2, &st2.st_mtim);

                    if(timercmp(&seconds_since_1970_2, &seconds_since_1970, >))
                    {
                        compare = 1;
                    }
                }

                if(compare)
                {
                    if(i_lower_flag)
                    {
                        printf("Do you want to overwrite %s?\n", aux);
                        fgets(buffer_read, sizeof buffer_read, stdin);
                        sscanf(buffer_read, "%c", &overwrite);

                        if(tolower(overwrite)=='s' || tolower(overwrite)=='y')
                        {

                            if(s_lower_flag)
                            {
                                unlink(aux);
                                symlink(filepath, aux);

                            }
                            else
                            {
                                make_copy(filepath, aux);

                            }
                            print_copied_file(filepath, aux);

                        }
                    }
                    else
                    {

                        if(s_lower_flag)
                        {
                            unlink(aux);
                            symlink(filepath, aux);

                        }
                        else
                        {
                            make_copy(filepath, aux);

                        }
                        print_copied_file(filepath, aux);

                    }

                }

            }
            else
            {
                if(i_lower_flag)
                {
                    printf("Do you want to overwrite %s?\n", aux);
                    fgets(buffer_read, sizeof buffer_read, stdin);
                    sscanf(buffer_read, "%c", &overwrite);

                    if(tolower(overwrite)=='s' || tolower(overwrite)=='y')
                    {

                        if(s_lower_flag)
                        {
                            unlink(aux);
                            symlink(filepath, aux);

                        }
                        else
                        {
                            make_copy(filepath, aux);

                        }

                        print_copied_file(filepath, aux);


                    }
                }
                else
                {

                    if(s_lower_flag)
                    {
                        unlink(aux);
                        symlink(filepath, aux);

                    }
                    else
                    {
                        make_copy(filepath, aux);

                    }

                    print_copied_file(filepath, aux);

                }


            }

        }
        else
        {
            if(s_lower_flag)
            {
                symlink(filepath, aux);

            }
            else
            {
                make_copy(filepath, aux);

            }

            print_copied_file(filepath, aux);

        }
    }


    return 0;

}



int show_recursively_entry(const char *filepath, const struct stat *info,
                           const int typeflag, struct FTW *pathinfo)
{
    configure_print(info, typeflag, filepath + pathinfo->base );
    return FTW_CONTINUE;

}



int percurr_directory_tree()
{
    int result;

    if (source == NULL || *source == '\0')
        return errno = EINVAL;

    int flags = 0;

    flags |= FTW_ACTIONRETVAL;
    flags |= FTW_PHYS;

    result = nftw(source, show_recursively_entry, USE_FDS, flags);


    if (result >= 0)
        errno = result;

    return errno;
}





int main(int argc, char **argv)
{

    extern char *optarg;
    extern int optind,optopt;

    int c, index;

    static struct option long_options[] =
    {
        {"interactive",           no_argument, 0, 'i'},
        {"recursive",     no_argument, 0, 'R'},
        {"verbose",     no_argument, 0, 'v'},
        {"update",           no_argument, 0, 'u'},
        {"symbolic-link",     no_argument, 0, 's'},
        { NULL,            no_argument,        NULL,    0 }
    };

    while ((c = getopt_long(argc, argv, "iRsuv", long_options, &index)) != -1)
    {
        switch (c)
        {
        case 'i':
            i_lower_flag = 1;
            break;
        case 'R':
            r_upper_flag = 1;
            break;
        case 's':
            s_lower_flag = 1;
            break;
        case 'u':
            u_lower_flag = 1;
            break;
        case 'v':
            v_lower_flag = 1;
            break;
        case '?':
            exit(1);
            break;

        }
    }

    if(argc-optind<2)
    {
        printf("Less arguments were passed\n");
        exit(1);
    }
    else if(argc-optind>2)
    {
        printf("So much arguments were passed\n");
        exit(1);
    }

    source = argv[optind];
    dest = argv[optind+1];


    struct stat st;
    int result = lstat(source, &st);
    if(S_ISDIR(st.st_mode) && !r_upper_flag)
    {
        printf("Omitting directory %s .\n", source);
        exit(1);
    }
    else if(errno == ENOENT)
    {
        printf("Source %s does not exist!\n", source);
        exit(1);
    }
    else if( result == 0)
    {
        percurr_directory_tree();

    }


    return 0;
}

