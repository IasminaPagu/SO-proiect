/*
Proiectul vizează dezvoltarea unui sistem de monitorizare a modificărilor în directoare, 
oferind utilizatorului capacitatea de a crea instantanee (snapshots) 
pentru a urmări evoluția acestora în timp. Utilizatorii vor specifica directorul țintă prin linia de comandă,
iar programul va detecta și urmări schimbările survenite, inclusiv în subdirectoarele sale. 
Schimbările monitorizate includ redenumiri, ștergeri, modificări de dimensiune, variații în numărul de legături 
și ajustări de permisiuni. La fiecare execuție, programul va actualiza snapshot-ul directorului, 
salvând metadate relevante pentru fiecare fișier sau subdirector, cum ar fi 

numele, identificatorul inode, dimensiunea și data ultimei modificări. 

În cazul identificării unui snapshot anterior, programul va compara
datele actuale cu cele vechi, actualizând sau înlocuind snapshot-ul după necesitate. 
Abordarea exactă de salvare și gestionare a snapshot-urilor este lăsată la discreția dezvoltatorilor, 
fie că optează pentru stocarea unui snapshot în fiecare director monitorizat, 
fie pentru crearea unei ierarhii separate. Se încurajează furnizarea de exemple de input și output pentru a ghida dezvoltatorii,
menținând totodată un grad de flexibilitate în implementare pentru a evita soluții uniforme. 
Proiectul încurajează inovația și auto-gestionarea resurselor, oferind studenților libertatea de a 
explora diverse metode de realizare a sarcinilor specificate.
   */

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

//adaug data ultimei modificari step1


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
    sprintf(buffer,"%sDIR %s\t: Dimensiune %ld bytes , inode number %ld , Time of last modification %d\n",spatii,nume_director,info.st_size, info.st_ino,ctime(&info.st_mtime));
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

            sprintf(buffer,"%sLINK %s\t: Dimensiune %ld bytes ,  inode number %ld , Time of last modification %d\n",spatii,cale_relativa,info.st_size, info.st_ino,ctime(&info.st_mtime));
            scriere_snapchot(fd2,buffer);
        }
        else if(S_ISREG(info.st_mode)){
            sprintf(buffer,"%sFILE %s\t: Dimensiune %ld bytes ,  inode number %ld , Time of last modification %d\n",spatii,cale_relativa,info.st_size, info.st_ino,ctime(&info.st_mtime));
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