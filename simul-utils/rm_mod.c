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
#include "simul_utils.h"


#ifndef USE_FDS
#define USE_FDS 15
#endif

int cont_list = 0;

int f_lower_flag = 0;
int i_lower_flag = 0;
int r_upper_flag = 0;
int preserve_root_flag = 0;
int no_preserve_root_flag = 0;

char *source;
char *dest;

char **strarray = NULL;
int strcount;
int cont = 0;

#define BUFFERSIZE 1024
#define COPYMORE 0644


void erro(const char *s1, const char *s2)
{
    fprintf(stderr, "Error: %s ", s1);
    perror(s2);
    exit(1);
}

int make_copy(const char *source_f, const char *destination_f)
{
    int in_fd, out_fd, n_chars;
    char buf[BUFFERSIZE];


    /* open files */
    if( (in_fd=open(source_f, O_RDONLY)) == -1 )
    {
        erro("Cannot open ", source_f);
    }


    if( (out_fd=creat(destination_f, COPYMORE)) == -1 )
    {
        erro("Cannot creat ", destination_f);
    }


    /* copy files */
    while( (n_chars = read(in_fd, buf, BUFFERSIZE)) > 0 )
    {
        if( write(out_fd, buf, n_chars) != n_chars )
        {
            erro("Write error to ", destination_f);
        }


        if( n_chars == -1 )
        {
            erro("Read error from ", source_f);
        }
    }


    /* close files */
    if( close(in_fd) == -1 || close(out_fd) == -1 )
    {
        erro("Error closing files", "");

    }


    return 1;
}



int configure_print(const struct stat *info, const int typeflag, const char *filepath)
{
    cont_list++;

    char line[1024];

    if(typeflag==FTW_D && r_upper_flag)
    {

        if(cont==2){
            strcpy(line, filepath);
            strarray = (char **)realloc(strarray, (strcount + 1) * sizeof(char *));
            strarray[strcount++] = strdup(line);

        }


    }

    if(!(typeflag == FTW_D))
    {
        unlink(filepath);
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

int print_only_directory_entry(const char *filepath, const struct stat *info,
                               const int typeflag, struct FTW *pathinfo)
{
    configure_print(info, typeflag, filepath + pathinfo->base);
    return FTW_STOP;

}


int percurr_directory_tree()
{
    int result;

    if (source == NULL || *source == '\0')
        return errno = EINVAL;

    int flags = 0;

    flags |= FTW_ACTIONRETVAL;
    flags |= FTW_PHYS;

    if(r_upper_flag)
    {
        cont++;
        result = nftw(source, show_recursively_entry, USE_FDS, flags);
        cont++;
        result = nftw(source, show_recursively_entry, USE_FDS, flags);
    }
    else
    {
        cont++;
        result = nftw(source, show_not_recursively_entry, USE_FDS, flags);
        cont++;
        result = nftw(source, show_not_recursively_entry, USE_FDS, flags);


    }

    if (result >= 0)
        errno = result;

    return errno;
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
        {"preserve-root",       no_argument, &preserve_root_flag, 1},
        {"no-preserve-root",   no_argument, &no_preserve_root_flag, 1},
        { NULL,            no_argument,        NULL,    0 }
    };

    while ((c = getopt_long(argc, argv, "fiR", long_options, &index)) != -1)
    {
        switch (c)
        {
        case 'f':
            f_lower_flag = 1;
            break;
        case 'i':
            i_lower_flag = 1;
            break;
        case 'R':
            r_upper_flag = 1;
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

    if(strcmp(source,"/")==0 && no_preserve_root_flag==0){
        printf("It's dangerous to remove root folder!\n");
        exit(1);
    }


    percurr_directory_tree();

    int i;
    if(strcount>0){
        for(i = strcount -1; i > -1; i--){
            rmdir(strarray[i]);
        }

    }

    return 0;
}

