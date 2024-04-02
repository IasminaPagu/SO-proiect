#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

void list_directory(char *directory_path){
    DIR *dir=NULL;
    struct dirent *entry;
    if((dir=opendir(directory_path))==NULL){
        printf("Error\n");
        exit(1);
    }
    char full_path[PATH_MAX];
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0){
            continue;
        }
        snprintf(full_path,sizeof(full_path),"%s/%s/%d",directory_path,entry->d_name,entry->d_reclen);
        printf("%s\n",full_path);
        if(entry->d_type == DT_DIR){
            list_directory(full_path);
        }
    }

    /*
    while((entry=readdir(dir))!=NULL){
        if(entry->d_type == DT_REG){
            printf("File: %s\n",entry->d_name);
        }
        else if(entry->d_type == DT_DIR){
            printf("Dir: %s\n",entry->d_name);
        }
    }
    */
    if(closedir(dir)==1){
        printf("Error closing directory\n");
        exit(1);
    }
}
int main(int argc, char *argv[]){
    list_directory(argv[1]);
    return 0;
}