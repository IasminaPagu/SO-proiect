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
#include <time.h>


int main(int argc, char *argv[]){

    //imi deschid directorul si vreau sa scriu in el ceva
    //gen sa pun continutul unui fisier.txt

    /*
    DIR *output = opendir(argv[3]);
    if(output == NULL){
        perror("eroare deschidere director output\n");
        exit(errno);
    }
    //intrb mea e 
    //inode -ul de la director este egal cu inode-ul de la
    int fd2 = open ("snapchot.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    //aici sa vad
    //dar trb sa il pun in argv[2]
    */
   char snapchot_name[100];
   sprintf(snapchot_name, "snapchot_%d",112487);
   printf("%s\n",snapchot_name);
   sprintf(snapchot_name,"000");
   printf("%s\n",snapchot_name);
    return 0 ;
}