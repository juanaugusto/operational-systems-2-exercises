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

#ifndef USE_FDS
#define USE_FDS 15
#endif

int cont_list = 0;

int a_lower_flag = 0;
int a_upper_flag = 0;
int author_flag = 0;
int d_lower_flag = 0;
int r_upper_flag = 0;
int i_lower_flag = 0;
int l_lower_flag = 0;
int default_flag = 0;

/*
Nomes: Jéssica Genta dos Santos - DRE: 111031073
       Juan Augusto Santos de Paula - DRE: 111222844
*/

int print_inode(unsigned long ino)
{
    if(i_lower_flag)
    {
        printf("%lu ", ino);
    }
    return 0;
}

int print_parent_folder(int typeflag){
    if(r_upper_flag && FTW_D==typeflag){
        printf("\n");
    }
    return 0;

}

int print_basic_info(const struct stat *fileStat, const int typeflag, const char *filepath)
{
    print_parent_folder(typeflag);

    print_inode(fileStat->st_ino);

    printf("%s\n", filepath);

    return 0;
}

int print_detailed_info(const struct stat *fileStat, const int typeflag, const char *filepath)
{

    print_parent_folder(typeflag);

    print_inode(fileStat->st_ino);

    if(S_ISDIR(fileStat->st_mode))
    {
        printf("d");
    }
    else if(S_ISLNK(fileStat->st_mode))
    {
        printf("l");
    }
    else if(S_ISBLK(fileStat->st_mode))
    {
        printf("b");
    }
    else if(S_ISCHR(fileStat->st_mode))
    {
        printf("c");
    }
    else
    {
        printf("-");
    }

    printf( (fileStat->st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat->st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat->st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat->st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat->st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat->st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat->st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat->st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat->st_mode & S_IXOTH) ? "x" : "-");

    printf(" %lu ",fileStat->st_nlink);

    struct group *grp;
    struct passwd *pwd;

    grp = getgrgid(fileStat->st_gid);
    printf(" %s ", grp->gr_name);

    pwd = getpwuid(fileStat->st_uid);
    printf(" %s ", pwd->pw_name);

    printf(" %lu ",fileStat->st_size);

    struct tm mtime;

    localtime_r(&(fileStat->st_mtime), &mtime);

    printf("%04d-%02d-%02d %02d:%02d:%02d",
           mtime.tm_year+1900, mtime.tm_mon+1, mtime.tm_mday,
           mtime.tm_hour, mtime.tm_min, mtime.tm_sec);


    if (typeflag == FTW_SL)
    {
        char   *target;
        size_t  maxlen = 1023;
        ssize_t len;

        while (1)
        {

            target = malloc(maxlen + 1);
            if (target == NULL)
                return ENOMEM;

            len = readlink(filepath, target, maxlen);
            if (len == (ssize_t)-1)
            {
                const int saved_errno = errno;
                free(target);
                return saved_errno;
            }
            if (len >= (ssize_t)maxlen)
            {
                free(target);
                maxlen += 1024;
                continue;
            }

            target[len] = '\0';
            break;
        }

        printf(" %s -> %s\n", filepath, target);
        free(target);

    }
    else if (typeflag == FTW_SLN)
        printf(" %s (dangling symlink)\n", filepath);
    else if (typeflag == FTW_F)
        printf(" %s\n", filepath);
    else if (typeflag == FTW_D || typeflag == FTW_DP)
        printf(" %s/\n", filepath);
    else if (typeflag == FTW_DNR)
        printf(" %s/ (unreadable)\n", filepath);
    else
        printf(" %s (unknown)\n", filepath);

    return 0;

}


int configure_print(const struct stat *info, const int typeflag, const char *filepath)
{
    cont_list++;

    if(cont_list>1){

        if(a_upper_flag)
        {

            if(strlen(filepath)>=2){

                if((filepath[0]=='.' && filepath[1]!='.') || filepath[0]!='.'){

                    if(!l_lower_flag)
                    {

                        print_basic_info(info, typeflag, filepath);
                    }
                    else
                    {
                        print_detailed_info(info, typeflag, filepath);
                    }
                }
            }
        }
        else if(!a_upper_flag)
        {
            if((default_flag &&  filepath[0]!='.') || !default_flag){

                if(!l_lower_flag)
                {

                    print_basic_info(info, typeflag, filepath);
                }
                else
                {
                    print_detailed_info(info, typeflag, filepath);
                }
            }
        }
    }

    return 0;

}


int print_not_recursively_entry(const char *filepath, const struct stat *info,
                                const int typeflag, struct FTW *pathinfo)
{

    configure_print(info, typeflag, filepath + pathinfo->base);


    if(pathinfo->level!=1)
    {
        return FTW_CONTINUE;
    }

    return FTW_SKIP_SUBTREE;

}


int print_recursively_entry(const char *filepath, const struct stat *info,
                            const int typeflag, struct FTW *pathinfo)
{
    configure_print(info, typeflag, filepath + pathinfo->base);
    return FTW_CONTINUE;

}

int print_only_directory_entry(const char *filepath, const struct stat *info,
                               const int typeflag, struct FTW *pathinfo)
{
    configure_print(info, typeflag, filepath + pathinfo->base);
    return FTW_STOP;

}


int print_directory_tree(const char *dirpath)
{
    int result;

    if (dirpath == NULL || *dirpath == '\0')
        return errno = EINVAL;

    int flags = 0;

    flags |= FTW_ACTIONRETVAL;
    flags |= FTW_PHYS;



    if(d_lower_flag)
    {
        result = nftw(dirpath, print_only_directory_entry, USE_FDS, flags);
        return 0;

    }

    if(r_upper_flag)
    {
        result = nftw(dirpath, print_recursively_entry, USE_FDS, flags);
    }
    else
    {
        result = nftw(dirpath, print_not_recursively_entry, USE_FDS, flags);
    }

    struct stat file_info_um;
    struct stat file_info_dois;

    char *dummy  = strdup( dirpath );
    char *dname = dirname( dummy );

    if(a_lower_flag){
        if(stat(dirpath, &file_info_um)!=0){
           printf("Erro no stat!");
           exit(-1);

        }
        if(stat(dname, &file_info_dois)!=0){
           printf("Erro no stat!");
           exit(-1);

        }
        configure_print(&file_info_um, FTW_D, ".");

        configure_print(&file_info_dois, FTW_D, "..");
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

    while ((c = getopt (argc, argv, "aARidl")) != -1)
    {
        switch (c)
        {
        case 'a':
            a_lower_flag = 1;
            a_upper_flag = 0;
            break;
        case 'A':
            a_upper_flag = 1;
            if(a_lower_flag)
            {
                a_upper_flag = 0;
            }
            break;
        case 'R':
            r_upper_flag = 1;
            break;
        case 'i':
            i_lower_flag = 1;
            break;
        case 'd':
            d_lower_flag = 1;
            break;
        case 'l':
            l_lower_flag = 1;
            break;
        case '?':
            exit(-1);
            break;

        }
    }

    //Setting default
    if(!a_lower_flag && !a_upper_flag)
    {
        default_flag = 1;
    }


    for (index = optind; index < argc; index++)
    {
        printf ("Listing contents of directory %s\n", argv[index]);
        print_directory_tree(argv[index]);
        printf("\n\n");

    }

    if(optind==argc)
    {
        print_directory_tree(".");  // Directories were not passed into command line
    }




    return 0;
}
