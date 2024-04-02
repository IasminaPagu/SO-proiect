#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
void listFiles(const char *dirname){
    DIR *dir;
    dir = opendir(dirname);
    if(dir == NULL){
        printf("Eroare\n");
        exit(1);
    }
    printf("Directorul curent este %s\n",dirname);

    struct dirent *entry;
    while((entry=readdir(dir))!=NULL){
        //adica atata timp cat nu ajungem
        //la finalul directorului
        printf("%s\n",entry->d_name);
        if(entry->d_type == DT_DIR && (strcmp(entry->d_name,".") != 0 ) && (strcmp(entry->d_name,"..") != 0)){
            //daca fisierul este un director
            //atunci afisam numele directorlui si apelam recursiv functia listFiles
            char path[1024] = {0};
            strcat(path,dirname);
            strcat(path,"/");
            strcat(path,entry->d_name);
            listFiles(path);
        }
    }
    closedir(dir);
}
int main(int argc, char *argv[]){
    listFiles(argv[1]);
    printf("Hello world\n");
    return 0;
}