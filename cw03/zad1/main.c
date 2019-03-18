#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

time_t date;                        // for functions to compare date
int sign;

int printStats(const char* path, const struct stat* stats, int flag, struct FTW* ftwbuf){
    
    int diff = (int) difftime(date, stats -> st_mtime)/(60*60*24);  // division to ignore hour
    if(sign == -1 && diff <= 0) return 0;
    if(sign == 0 && diff != 0) return 0;
    if(sign == 1 && diff >= 0) return 0;

    if(S_ISREG(stats->st_mode)){                        // only regular files

        if(stats->st_mode & S_IFDIR) printf("d");       // is directory - SHOULD be always '-'
        else printf("-");
        if(stats->st_mode & S_IRUSR) printf("r");       // access rights
        else printf("-");
        if(stats->st_mode & S_IWUSR) printf("w");
        else printf("-");
        if(stats->st_mode & S_IXUSR) printf("x");
        else printf("-");
        if(stats->st_mode & S_IRGRP) printf("r");
        else printf("-");
        if(stats->st_mode & S_IWGRP) printf("w");
        else printf("-");
        if(stats->st_mode & S_IXGRP) printf("x");
        else printf("-");
        if(stats->st_mode & S_IROTH) printf("r");
        else printf("-");
        if(stats->st_mode & S_IWOTH) printf("w");
        else printf("-");
        if(stats->st_mode & S_IXOTH) printf("x");
        else printf("-");
        printf("\t ");

        printf("%10ld\t ", stats->st_size);              // size

        char buffor[20];                                // last modification time
        struct tm* timeinfo = localtime(&(stats -> st_mtime));
        strftime(buffor, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
        printf("%s\t ",buffor);

        printf("%d\t",(int)getpid());

        printf("%s\n", realpath(path, NULL));
    }
    return 0;
}

void manualFTW(char *basePath) {
    DIR *directory = opendir(basePath);
    if (directory == NULL) {
        printf("No such directory was found\n");
        return; 
    }

    struct dirent *dirEntry;        // ino_t d_ino – file i-node number;   char d_name[] - file name
    struct stat stats;
    char *path = malloc(PATH_MAX * sizeof(char));
    strcpy(path, basePath);

    while ((dirEntry = readdir(directory)) != NULL) {           // walk through files until end
        strcpy(path, basePath);
        strcat(path, "/");                                      // adding to path '/'
        strcat(path, dirEntry->d_name);                         // adding next file name

        if ((strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)) continue;

        if(lstat(path, &stats) >= 0) {
            if (S_ISDIR(stats.st_mode)) {                       // if catalog
                pid_t pid = fork();
                if(pid == 0){
                    manualFTW(path);
                    exit(0);
                    }

            } else if(S_ISREG(stats.st_mode)) {                 // if regular file
                printStats(path, &stats, FTW_F, NULL);
            }
        }
    }

    closedir(directory);
}

int main(int argc, char** argv){

    if(argc != 5){
        printf("Wrong number of parameters!\n");
        return 1;
    }

    char* path = argv[1];

    
    if(strcmp(argv[2],"<") != 0 && strcmp(argv[2],">") != 0 && strcmp(argv[2],"=") != 0){
        printf("Pass: '>', '<' or '=' as the second argument!\n");
        return 1;
    }
    char* comparator = argv[2];
    sign = (int) comparator[0] - 61;                                // 60 - '<'; 61 - '='; 62 - '>';

    struct tm* time = malloc(sizeof(struct tm));                    // format: YYYY-MM-DD
    if(strptime(argv[3], "%Y-%m-%d", time) == NULL){
        printf("Date format must be YYYY-MM-DD !\n");
        return 1;
    }
    date = mktime(time);
    
    if(strcmp(argv[4],"stat") != 0 && strcmp(argv[4],"nftw") != 0){
        printf("Pass 'stat' or 'nftw' as the forth argument for mode!\n");
        return 1;
    }
    char* version = argv[4];


    if(strcmp(version,"nftw") == 0){
        nftw(realpath(path, NULL), printStats, 10, FTW_PHYS);    // realpath - absolute pathname   
    }                                                           // FTW_PHYS - no symbolic links
    else manualFTW(path);

    return 0;
}