#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>

void parcurge(char *nume_dir, int nivel, FILE *snapchot)
{
    //nivelul meu initial este 0
    //adica pe pe nivelul 0 am directorul curent si toate fisierele si directoarele din el
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    char cale[PATH_MAX], cale_link[PATH_MAX + 1], spatii[PATH_MAX];
    //cale este calea catre fisierul sau directorul curent
    //cale_link este calea catre linkul simbolic
    //spatii este un vector de caractere care contine spatii

    int n;
    
    memset(spatii,' ', 2*nivel);
    //adica pe fiecare nivel adaug 2 spatii

    if(!(dir = opendir(nume_dir)))
    {
        printf("%s: ", nume_dir); fflush(stdout); 
        fprintf(snapchot,"%s: ", nume_dir);
        perror("opendir");
        exit(1);
    }
    //printf("%sDIR %s : Dimeniune %ld bytes, Last access time %s\n", spatii, nume_dir,info.st_size, ctime(&info.st_atime));
    fprintf(snapchot,"%sDIR %s : \nDimeniune %ld bytes, \nLast access time %s\n", spatii, nume_dir,info.st_size, ctime(&info.st_atime));
    /*
    entry = readdir(dir);
    if(entry==NULL){
        printf("Directorul trimis ca si argument in linie de comanda este gol\n");
        exit(1);
    }
    else{
      printf("%s  %s Director parinte, Dimeniune %d bytes, Last access time %s", spatii, cale,entry->d_reclen, ctime(&info.st_atime));
    
    }
    printf("%s  %s Director parinte, Dimeniune %d bytes, Last access time %s", spatii, cale,entry->d_reclen, ctime(&info.st_atime));
    //ce am scris mai sus este pentru a afisa directorul curent
    */
    
    while((entry=readdir(dir)) != NULL)
    {
        //adica aici imi parcurg directorul curent
    
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..")==0)
           continue;
    
        snprintf(cale, sizeof(cale), "%s/%s", nume_dir, entry->d_name);
        //snprintf() is a safer version of sprintf() that ensures that the output is properly null-terminated.
        //sprinrf() is a function that writes data to string and snprintf() is a function that writes data to a string with a specified length.
        //snprintf() is a safer version of sprintf() that ensures that the output is properly null-terminated.
        if(lstat(cale, &info)<0) 
        {
          //printf("%s: ", cale);  
          fprintf(snapchot,"%s: ", cale);
          fflush(stdout);
          perror("eroare la lstat");
          exit(1);
        }

        if(S_ISDIR(info.st_mode)){
          //printf("%s  %s Director file, Dimeniune %d bytes, Last access time %s", spatii, cale,entry->d_reclen, ctime(&info.st_atime));
          fprintf(snapchot,"%s  %s Director file, \nDimeniune %d bytes, \nLast access time %s", spatii, cale,entry->d_reclen, ctime(&info.st_atime));
          parcurge(cale, nivel + 1,snapchot);
        }
        else
        if(S_ISLNK(info.st_mode))
        {
          //printf("%s  %s Link simbolic, Dimeniune %d bytes, Last access time %s", spatii, cale,entry->d_reclen, ctime(&info.st_atime));
          fprintf(snapchot,"%s  %s Link simbolic, \nDimeniune %d bytes, \nLast access time %s", spatii, cale,entry->d_reclen, ctime(&info.st_atime));
          n = readlink(cale, cale_link, sizeof(cale_link));
          cale_link[n]='\0';
          //printf("%s  %s -> %s\n", spatii, cale, cale_link);
          fprintf(snapchot,"%s  %s -> %s\n", spatii, cale, cale_link);
        }
        else
        {
          //printf("%s  %s Regular file, Dimeniune %d bytes, Last access time %s", spatii, cale,entry->d_reclen, ctime(&info.st_atime));
          fprintf(snapchot,"%s  %s Regular file, \nDimeniune %d bytes, \nLast access time %s", spatii, cale,entry->d_reclen, ctime(&info.st_atime));
          
          if(info.st_mode & S_IXUSR || info.st_mode & S_IXGRP || info.st_mode & S_IXOTH)
          //S_IXUSR - execute/search permission, owner
          //S_IXGRP - execute/search permission, group
          //S_IXOTH - execute/search permission, others
          //printf("*");
          //printf("\n");
          fprintf(snapchot,"*");
          fprintf(snapchot,"\n");
        }
        //entry=readdir(dir);
    }
    closedir(dir);
}

int main(int argc, char *argv[])
{
  FILE *snapchot;
  snapchot = fopen("snapchot.txt", "w");
  if(snapchot == NULL){
    printf("Eroare la deschiderea fisierului\n");
    exit(1);
  }
    if(argc != 2)
    {
      //printf("Mod de utilizare: %s director\n", argv[0]);
      fprintf(snapchot,"Mod de utilizare: %s director\n", argv[0]);
      exit(1);
    }
    
    parcurge(argv[1], 0,snapchot);
    fclose(snapchot);
    
    return 0;
}