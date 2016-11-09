
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
#include <libgen.h>

#include <fcntl.h>
#include <stddef.h>

#if (__STDC_VERSION__ >= 199901L)
#include <stdint.h>
#endif

#ifndef USE_FDS
#define USE_FDS 15
#endif

int cont_list = 0;

int f_lower_flag = 0;
int i_lower_flag = 0;
int r_upper_flag = 0;

char *source;
char *dest;

char **strarray = NULL;
int strcount;
int cont = 0;

#define BUFFERSIZE 1024
#define COPYMORE 0644
/*
Nomes: Jéssica Genta dos Santos - DRE: 111031073
       Juan Augusto Santos de Paula - DRE: 111222844
*/

void oops(const char *s1, const char *s2)
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
        oops("Cannot open ", source_f);
    }


    if( (out_fd=creat(destination_f, COPYMORE)) == -1 )
    {
        oops("Cannot creat ", destination_f);
    }


    /* copy files */
    while( (n_chars = read(in_fd, buf, BUFFERSIZE)) > 0 )
    {
        if( write(out_fd, buf, n_chars) != n_chars )
        {
            oops("Write error to ", destination_f);
        }


        if( n_chars == -1 )
        {
            oops("Read error from ", source_f);
        }
    }


    /* close files */
    if( close(in_fd) == -1 || close(out_fd) == -1 )
    {
        oops("Error closing files", "");

    }


    return 1;
}

char *repl_str(const char *str, const char *from, const char *to)
{

    /* Adjust each of the below values to suit your needs. */

    /* Increment positions cache size initially by this number. */
    size_t cache_sz_inc = 16;
    /* Thereafter, each time capacity needs to be increased,
     * multiply the increment by this factor. */
    const size_t cache_sz_inc_factor = 3;
    /* But never increment capacity by more than this number. */
    const size_t cache_sz_inc_max = 1048576;

    char *pret, *ret = NULL;
    const char *pstr2, *pstr = str;
    size_t i, count = 0;
#if (__STDC_VERSION__ >= 199901L)
    uintptr_t *pos_cache_tmp, *pos_cache = NULL;
#else
    ptrdiff_t *pos_cache_tmp, *pos_cache = NULL;
#endif
    size_t cache_sz = 0;
    size_t cpylen, orglen, retlen, tolen, fromlen = strlen(from);

    /* Find all matches and cache their positions. */
    while ((pstr2 = strstr(pstr, from)) != NULL)
    {
        count++;

        /* Increase the cache size when necessary. */
        if (cache_sz < count)
        {
            cache_sz += cache_sz_inc;
            pos_cache_tmp = realloc(pos_cache, sizeof(*pos_cache) * cache_sz);
            if (pos_cache_tmp == NULL)
            {
                goto end_repl_str;
            }
            else pos_cache = pos_cache_tmp;
            cache_sz_inc *= cache_sz_inc_factor;
            if (cache_sz_inc > cache_sz_inc_max)
            {
                cache_sz_inc = cache_sz_inc_max;
            }
        }

        pos_cache[count-1] = pstr2 - str;
        pstr = pstr2 + fromlen;
    }

    orglen = pstr - str + strlen(pstr);

    /* Allocate memory for the post-replacement string. */
    if (count > 0)
    {
        tolen = strlen(to);
        retlen = orglen + (tolen - fromlen) * count;
    }
    else	retlen = orglen;
    ret = malloc(retlen + 1);
    if (ret == NULL)
    {
        goto end_repl_str;
    }

    if (count == 0)
    {
        /* If no matches, then just duplicate the string. */
        strcpy(ret, str);
    }
    else
    {
        /* Otherwise, duplicate the string whilst performing
         * the replacements using the position cache. */
        pret = ret;
        memcpy(pret, str, pos_cache[0]);
        pret += pos_cache[0];
        for (i = 0; i < count; i++)
        {
            memcpy(pret, to, tolen);
            pret += tolen;
            pstr = str + pos_cache[i] + fromlen;
            cpylen = (i == count-1 ? orglen : pos_cache[i+1]) - pos_cache[i] - fromlen;
            memcpy(pret, pstr, cpylen);
            pret += cpylen;
        }
        ret[retlen] = '\0';
    }

end_repl_str:
    /* Free the cache and return the post-replacement string,
     * which will be NULL in the event of an error. */
    free(pos_cache);
    return ret;
}

int configure_print(const struct stat *info, const int typeflag, const char *filepath)
{
    cont_list++;

    char line[1024];





    if(!(typeflag == FTW_D && cont_list==1))  //It is a directory
    {


    }
    else if(!(typeflag == FTW_F && cont_list==1))  //It is a file
    {


    }

    struct stat st;

    if(typeflag==FTW_D && r_upper_flag)
    {

        if(cont==2){
            //printf("ook %s\n", filepath);

            strcpy(line, filepath);
            strarray = (char **)realloc(strarray, (strcount + 1) * sizeof(char *));
            strarray[strcount++] = strdup(line);


            //rmdir(filepath);
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

    int index;
    int c;

    // TODO Colocar as opções certas

    /*static struct option long_options[] =
    {
        {"all",           no_argument, 0, 'a'},
        {"almost-all",     no_argument, 0, 'A'},
        {"author",       no_argument, &author_flag, 1},
        {"directory",   no_argument, 0, 'd'},
        {"recursive",no_argument, 0, 'r'},
        {"inode",            no_argument, 0, 'i'},
        { NULL,            no_argument,        NULL,    0 }
    };*/

    while ((c = getopt (argc, argv, "fiR")) != -1)
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

    percurr_directory_tree();

    int i;
    if(strcount>0){

        for(i = strcount -1; i > -1; i--){
            rmdir(strarray[i]);
            printf("strarray[%d] == %s\n", i, strarray[i]);
        }

    }


    printf("cont %d\n",cont_list);

    return 0;
}

