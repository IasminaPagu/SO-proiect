#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>

int main(){
    DIR *dir;
    struct dirent *entry;

    dir = opendir(".");
    if(dir==NULL){
        printf("Error\n");
        return 1;
    }
    while((entry=readdir(dir))!=NULL){
        if(entry->d_type == DT_REG){
            printf("File: %s\n",entry->d_name);
        }
        else if(entry->d_type == DT_DIR){
            printf("Dir: %s\n",entry->d_name);
        }
    }
    if(closedir(dir)==1){
        printf("Error closing directory\n");
        return 1;
    }
    return 0;
}