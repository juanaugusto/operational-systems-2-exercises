/*
Nomes: Jéssica Genta dos Santos - DRE: 111031073
       Juan Augusto Santos de Paula - DRE: 111222844
*/

#define _GNU_SOURCE

#define _XOPEN_SOURCE 700
#include <ftw.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdint.h>
#include <fcntl.h>
#include <getopt.h>
#include <ctype.h>
#include "simul_utils.h"


#ifndef USE_FDS
#define USE_FDS 15
#endif

int cont_list = 0;

int f_lower_flag = 0;
int i_lower_flag = 0;
int r_upper_flag = 0;
int v_lower_flag = 0;
int preserve_root_flag = 0;
int no_preserve_root_flag = 0;

char *source;
char *dest;

char **strdirs = NULL;
int strdirs_count = 0;

char **strfiles = NULL;

int strfiles_count = 0;
int cont = 0;

char buffer_read[1000];


#define BUFFERSIZE 1024
#define COPYMORE 0644

int configure_print(const struct stat *info, const int typeflag, const char *filepath)
{
    cont_list++;

    char line[1024];

    if(typeflag==FTW_D && r_upper_flag)
    {

        strcpy(line, filepath);
        strdirs = (char **)realloc(strdirs, (strdirs_count + 1) * sizeof(char *));
        strdirs[strdirs_count++] = strdup(line);

    }

    if(!(typeflag == FTW_D))
    {


        strcpy(line, filepath);
        strfiles = (char **)realloc(strfiles, (strfiles_count + 1) * sizeof(char *));
        strfiles[strfiles_count++] = strdup(line);

    }

    return 0;

}


int show_not_recursively_entry(const char *filepath, const struct stat *info,
                               const int typeflag, struct FTW *pathinfo)
{

    configure_print(info, typeflag, filepath );

    if(pathinfo->level!=1)
    {
        return FTW_CONTINUE;
    }
    return FTW_SKIP_SUBTREE;
}


int show_recursively_entry(const char *filepath, const struct stat *info,
                           const int typeflag, struct FTW *pathinfo)
{
    configure_print(info, typeflag, filepath );
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


int print_remove_message(const char* path, int err_number, int is_dir)
{


    if(is_dir)
    {

        if(err_number == ENOTEMPTY)
        {
            printf("Directory %s is not empty, therefore it can't be removed.\n", path);
        }
        else if(v_lower_flag)
        {
            printf("Removing directory %s\n", path);
        }
    }
    else if(v_lower_flag)
    {
        printf("Removing file %s\n", path);

    }
    return 0;
}





int main(int argc, char **argv)
{

    extern char *optarg;
    extern int optind,optopt;

    int c;
    int index;
    // TODO Colocar as opções certas

    static struct option long_options[] =
    {
        {"force",           no_argument, 0, 'f'},
        {"recursive",     no_argument, 0, 'R'},
        {"verbose",     no_argument, 0, 'v'},
        {"preserve-root",       no_argument, &preserve_root_flag, 1},
        {"no-preserve-root",   no_argument, &no_preserve_root_flag, 1},
        { NULL,            no_argument,        NULL,    0 }
    };

    while ((c = getopt_long(argc, argv, "fiRv", long_options, &index)) != -1)
    {
        switch (c)
        {
        case 'f':
            if(i_lower_flag)
            {
                i_lower_flag = 0;
            }
            f_lower_flag = 1;
            break;
        case 'i':

            if (f_lower_flag)
            {
                f_lower_flag = 0;
            }
            i_lower_flag = 1;
            break;
        case 'R':
            r_upper_flag = 1;
            break;
        case 'v':
            v_lower_flag = 1;
            break;
        case '?':
            exit(-1);
            break;

        }
    }


    if(argc-optind<1)
    {
        printf("Less arguments were passed\n");
        exit(1);
    }
    else if(argc-optind>1)
    {
        printf("So much arguments were passed\n");
        exit(1);
    }

    source = argv[optind];

    if(strcmp(source,"/")==0 && !no_preserve_root_flag && r_upper_flag)
    {
        printf("It's dangerous to remove root folder!\n");
        exit(1);
    }


    struct stat st;
    int result = lstat(source, &st);
    if(S_ISDIR(st.st_mode) && !r_upper_flag)
    {
        printf("Can't remove %s . It's a directory!\n", source);
        exit(1);
    }
    else if(errno == ENOENT && !f_lower_flag)
    {
        printf("Path %s does not exist!\n", source);
        exit(1);
    }
    else if( result == 0)
    {
        percurr_directory_tree();

    }


    int i;
    char overwrite;
    if(strfiles_count>0)
    {
        for(i = strfiles_count -1; i > -1; i--)
        {
            if(i_lower_flag)
            {
                printf("Do you want to remove file %s?\n", strfiles[i]);
                fgets(buffer_read, sizeof buffer_read, stdin);
                sscanf(buffer_read, "%c", &overwrite);


                if(tolower(overwrite)=='s' || tolower(overwrite)=='y')
                {
                    print_remove_message(strfiles[i],0,0);
                    unlink(strfiles[i]);

                }
            }
            else
            {
                print_remove_message(strfiles[i],0,0);
                unlink(strfiles[i]);

            }
        }
    }



    if(strdirs_count>0)
    {
        for(i = strdirs_count -1; i > -1; i--)
        {
            rmdir(strdirs[i]);
            print_remove_message(strdirs[i], errno, 1);

        }

    }

    return 0;
}

