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


/*pt fiecare argument primit in linia de comanda se va crea un proces separat 
care sa se ocupe de argumentul respectiv

numele snapchot-ul va contine i_node number

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
void scriere_snapchot(int file_descriptor, char buff2[]){
    if(write(file_descriptor,buff2,strlen(buff2)) < 0 ){
    perror("eroare scriere in fisier\n");
    exit(errno);
   }

}
void parcurgere_director(char *nume_director, int nivel, int *inode_number, int contor , char buffer_auxiliar[]){
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
    if(contor == 0){
        //inseamna ca asta este prima iteratie
        //deci acum vreau sa mi aflu inode-ul directorului
        *inode_number = info.st_ino;
    }
    snprintf(buffer,sizeof(buffer),"%s DIR %s\t: Dimensiune %ld bytes , inode number %ld , Time of last modification %s\n",spatii,nume_director,info.st_size, info.st_ino,ctime(&info.st_mtime));
    strcat(buffer_auxiliar,buffer);
    //scriere_snapchot(fd2,buffer);

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
            parcurgere_director(cale_relativa, nivel + 3, inode_number, contor + 1, buffer_auxiliar);
        }else if(S_ISLNK(info.st_mode)){

            snprintf(buffer,sizeof(buffer)," %sLINK  %s\t: Dimensiune %ld bytes ,  inode number %ld , Time of last modification %s\n",spatii,cale_relativa,info.st_size, info.st_ino,ctime(&info.st_mtime));
            strcat(buffer_auxiliar,buffer);
            //scriere_snapchot(fd2,buffer);
        }
        else if(S_ISREG(info.st_mode)){
            snprintf(buffer,sizeof(buffer),"%s FILE %s\t: Dimensiune %ld bytes ,  inode number %ld , Time of last modification %s\n",spatii,cale_relativa,info.st_size, info.st_ino,ctime(&info.st_mtime));
            strcat(buffer_auxiliar,buffer);
            //scriere_snapchot(fd2,buffer);
        }
    }
    closedir(dir);
    

}

int main(int argc, char *argv[]){

    if(argc > 13){
        printf("Nu ati transmis numarul potrivit de argumente in linia de comanda\n");
        exit(EXIT_FAILURE);
    }
    int fd_out = open(argv[2],__O_PATH);
    if(fd_out == -1){
        perror("nu am putut deschide directorul folosind functia open\n");
        exit(errno);
    }
    /*
    eu vreau snapchot.txt sa mi se genereze in directorul output
    care este argv[2]
    rezulta ca imi deschid directorul de output si ii iau file descriptorul
    pe care il folosesc in functia "openat"
    */

   /*
    int fd2 = openat (fd_out,"snapchot.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    if((fd2 == -1)){
    perror("Eroare la crearea fisierului de iesire\n");
    exit(errno);
    }
    */


    int fd2;
    char snapchot_name[100];

    int inode_number = 0;
    char buffer_auxiliar[BUFFER_SIZE];//este buffer-ul in care imi stochez parcurgerea_directorului

    //inode_number ul meu trebuie sa fie pentru directorul din lista de argumente in linia de comanda
    //=> in functia parcurgere_director trb sa imi retin 1 singura data inode_numberul
    //la prima iteratie gen
    for(int i = 3 ; i < argc ; i++){
        inode_number = 0;
        strcpy(buffer_auxiliar,"");
        parcurgere_director(argv[i],0, &inode_number, 0,buffer_auxiliar);

        sprintf(snapchot_name, "snapchot_%d.txt",inode_number);
        fd2 = openat (fd_out,snapchot_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

        //printf("\n\n bufferul auxiliar este %s\n\n",buffer_auxiliar);
        //afiseaza bine => problema nu este la buffer_auxiliar

        if((fd2 == -1)){
        perror("Eroare la crearea fisierului de iesire\n");
        exit(errno);
        }

        scriere_snapchot(fd2,buffer_auxiliar);
        close(fd2);
    }
    close(fd_out);


    return 0;
    //deci eu vreau sa imi creez un snapchot in output
    //pentru fiecare argument in linia de comanda
    //./p -o output dir1 dir2 dir3 dir4 dir5 dir6 dir7 dir8 dir9
    //output continut prima iteratie
    //snapchot_inode_dir1 snapchot_inode_dir2 snapchot_inode_dir3 etc

    //first step
    //cum imi pun pentru un singur argument in linia de comanda             DONE
    //fisierul snapchot in directorul transmis ca si parametru



    //second step
    //acum vreau sa imi generez un snapchot pentru fiecare argument in linia de comanda         DONE
    //snapchot caruia vreau sa ii dau numele "snapchot_inodeNumber.txt"


    //third step-efectiv nu stiu cum sa ma apuc
    //sa compar snapshoturile intre ele...


    //chestii de finete
    //1)ar trb sa vedem daca argumentele transmise ca parametrii sunt directoare
    //2) ar trb sa verificam daca output este un director

}