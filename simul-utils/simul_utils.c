#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFERSIZE 1024
#define COPYMORE 0644

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

void print_error_copy(const char *s1, const char *s2)
{
    fprintf(stderr, "Error: %s ", s1);
    perror(s2);
    exit(1);
}

int make_copy(const char *source_f, const char *destination_f)
{
    int in_fd, out_fd, n_chars;
    char buf[BUFFERSIZE];

    if( (in_fd=open(source_f, O_RDONLY)) == -1 )
    {
        print_error_copy("Cannot open ", source_f);
    }


    if( (out_fd=creat(destination_f, COPYMORE)) == -1 )
    {
        print_error_copy("Cannot creat ", destination_f);
    }

    while( (n_chars = read(in_fd, buf, BUFFERSIZE)) > 0 )
    {
        if( write(out_fd, buf, n_chars) != n_chars )
        {
            print_error_copy("Write error to ", destination_f);
        }


        if( n_chars == -1 )
        {
            print_error_copy("Read error from ", source_f);
        }
    }

    if( close(in_fd) == -1 || close(out_fd) == -1 )
    {
        print_error_copy("Error closing files", "");

    }


    return 1;
}
