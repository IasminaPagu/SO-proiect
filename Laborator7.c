#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096
void scriere_snapchot(int file_descriptor, char *buff2){
    if(write(file_descriptor,buff2,strlen(buff2)) < 0 ){
    perror("eroare scriere in fisier\n");
    exit(errno);
   }

}
void parcurgere_director(char *nume_director, int nivel, int fd2){
    //nume_director este argv[1]
    DIR *dir = opendir(nume_director);
    if(dir == NULL){
        perror("Eroare deschidere director\n");
        exit(errno);
    } 
    struct stat info;
    char cale_relativa[BUFFER_SIZE] = "",spatii[BUFFER_SIZE] = "";
    struct dirent *intrare;
    char buffer[BUFFER_SIZE]="";

    memset(spatii, ' ', 3*nivel);
    spatii[2*nivel]='\0';

    int val_lstat;
    val_lstat = lstat(nume_director,&info);
    if(val_lstat == -1){
        perror("Nu s-au putut afla atributele fisierului\n");
        exit(errno);
    }
    sprintf(buffer,"%sDIR %s\t: Dimensiune %ld bytes , inode number %ld\n",spatii,nume_director,info.st_size, info.st_ino);
    scriere_snapchot(fd2,buffer);

    while( (intrare = readdir(dir) ) != NULL){
        //printf("%s\n",intrare->d_name);
        //intrare->d_name reprezinta numele fisierului/directorlui

        if(strcmp(intrare->d_name,".") == 0 || (strcmp(intrare->d_name,"..") == 0)){
            continue;
            //adica daca avem .. sau . inseamna ca am dat de calea relativa sau absoluta
            //si nu vreau sa afisez asta
        }
        snprintf(cale_relativa, sizeof(cale_relativa), "%s/%s", nume_director, intrare->d_name);
        val_lstat = lstat(cale_relativa,&info);
        if(val_lstat == -1){
        perror("Nu s-au putut afla atributele fisierului\n");
        exit(errno);
        }
        if(S_ISDIR(info.st_mode)){
            parcurgere_director(cale_relativa,nivel+3,fd2);
        }else if(S_ISLNK(info.st_mode)){

            sprintf(buffer,"%sLINK %s\t",spatii,cale_relativa);
            scriere_snapchot(fd2,buffer);
        }
        else if(S_ISREG(info.st_mode)){
            sprintf(buffer,"%sFILE %s\t: Dimensiune %ld bytes ,  inode number %ld\n",spatii,cale_relativa,info.st_size, info.st_ino);
            scriere_snapchot(fd2,buffer);
        }
    }
    closedir(dir);
    

}

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Nu ati transmis numarul potrivit de argumente in linia de comanda\n");
        exit(EXIT_FAILURE);
    }
    int fd2 = open ("snapchot.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    if((fd2 == -1)){
    perror("Eroare la crearea fisierului de iesire\n");
    exit(errno);
    }
    parcurgere_director(argv[1],0,fd2);

    return 0;
}