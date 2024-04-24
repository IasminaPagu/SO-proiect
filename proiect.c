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
#include<sys/wait.h>

//adaug data ultimei modificari step1


#define BUFFER_SIZE 4096
void scriere_snapchot(char *cale_director, char buff2[]){
    int fd;
    fd = open (cale_director,  O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if((fd == -1)){
        perror("Eroare la crearea fisierului de iesire\n");
        exit(errno);
    }
    if(write(fd,buff2,strlen(buff2)) < 0 ){
    perror("eroare scriere in fisier\n");
    exit(errno);
   }
   close(fd);
}

int verificare_exista_snapchot_anterior(char *nume_dir_out, char *nume_snapchot){
    DIR *dir = opendir(nume_dir_out);
    if(dir == NULL){
        perror("Eroare deschidere director\n");
        exit(errno);
    } 
    struct dirent *intrare;
    while( (intrare = readdir(dir)) != NULL){
        if(strcmp(intrare->d_name,nume_snapchot) == 0 ){
            //inseamna ca snapchotul pe care vreau sa l generez pentru argumentul dat in linie
            //de comanda exista deja
            return 1;
        }
    }
    return 0;
}
int comparare_snapchot_anterior(const char *cale_relativa_snap_anterior, const char *buffer_actual){
    int fd=0;//file descriptor snapchot anterior
    int n;
    char buffer[BUFFER_SIZE]="";
    fd = open(cale_relativa_snap_anterior, O_RDONLY);
    if(fd == -1){
        perror("eroare deschidere snapchot anterior\n");
        exit(errno);
    }
    n = read(fd, buffer, BUFFER_SIZE-1);
    if( n == -1){
        perror("nu s a putut citi din snapchot\n");
        exit(errno);
    }
   if(strcmp(buffer,buffer_actual) == 0){//inseamna ca nu s a facut nicio modificare
    return 0;
   }
   else{
    
    return 1;//inseamna ca s a facut o modificare si vreau sa se inlocuiasca snapchotul anterior
   }
   close(fd);
   //are vreun efect chestia asta  ???
}
void parcurgere_director(char *nume_director, int nivel, int *inode_number, int contor , char buffer_auxiliar[]){
    DIR *dir = opendir(nume_director);
    if(dir == NULL){
        perror("Eroare deschidere director\n");
        exit(errno);
    } 
    struct stat info;
    char cale_relativa[512] = "",spatii[100] = "";
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
        }
        else if(S_ISREG(info.st_mode)){
            snprintf(buffer,sizeof(buffer),"%s FILE %s\t: Dimensiune %ld bytes ,  inode number %ld , Time of last modification %s\n",spatii,cale_relativa,info.st_size, info.st_ino,ctime(&info.st_mtime));
            strcat(buffer_auxiliar,buffer);
        }
    }
    closedir(dir);
    

}

int main(int argc, char *argv[]){

    char cale_director[256]="";
    char snapchot_name[100];

    int inode_number = 0;
    char buffer_auxiliar[BUFFER_SIZE];//este buffer-ul in care imi stochez parcurgerea_directorului

    pid_t pid ,wpid;
    int status;

    if(argc > 13){
        printf("Nu ati transmis numarul potrivit de argumente in linia de comanda\n");
        exit(EXIT_FAILURE);
    }
    
    for(int i = 3 ; i < argc ; i++){

        inode_number = 0;
        strcpy(buffer_auxiliar,"");
        parcurgere_director(argv[i],0, &inode_number, 0,buffer_auxiliar);

        sprintf(snapchot_name, "snapchot_%d.txt",inode_number);

        strcpy(cale_director, argv[2]);
        strcat(cale_director, "/");
        strcat(cale_director,snapchot_name);

        //am adaugat asta, deoarece cu abordarea trecuta, nu inteleg de ce
        //pentru acelasi fisier primeam 2 fd diferite 4 si 5
        pid = fork();
        if (pid == -1){
            printf("eroare fork\n");
            exit(1);
        }
        if(pid == 0){//cod fiu
            if(verificare_exista_snapchot_anterior(argv[2],snapchot_name) == 0){//inseamna ca noul snapchot nu exista in directorul de output
                //si atunci il creez
                scriere_snapchot(cale_director,buffer_auxiliar);
                printf("nu exista inainte de acest apel\n");
            }else{
                //compar ce exista deja in snapchot_name cu buffer_auxiliar pe care l am obtinut prin apelarea functiei parcurgere_director(...)
                if( (comparare_snapchot_anterior(cale_director,buffer_auxiliar) == 0) ){//inseamna ca nu s-a facut nicio modificare in snapchot
                    printf("nu s a produs nicio modificare fata de snapchotul anterior\n");
                    //continue;
                }
                else{
                    scriere_snapchot(cale_director, buffer_auxiliar);
                    printf("a existat o modificare\n");
                }
                printf("exista deja \n");
            }
            exit(0);
        }
    }

    //proces parinte
    for(int i = 3 ; i < argc ; i++){
        wpid = wait(&status);
        if (wpid == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
        }
        if(WIFEXITED(status)){
        printf("Child %d ended with code %d cu numele %s\n",wpid, WEXITSTATUS(status),snapchot_name);
        }
        else{
            printf("Child %d ended abnormally\n", wpid);
        }

    }
    /*
    afisarea din enunt trb sa fie 
    printf("Procesul cu PID %d s-a incheiat cu codul %d",wpid,WEXITSTATUS(status));
    */
        return 0;

}
            /*
     printf("\nChild ended with code %d\n", WEXITSTATUS(status));
  else
     printf("\nChild ended abnormally\n");
            if(WIFEXITED(status)){
                printf("\nChild %d ended with code %d cu numele %s\n\n", getpid(), WEXITSTATUS(status),snapchot_name);
            }else{
                printf("\nChild %d ended abnormally\n", getpid());
            }
            */

    /*  IDEEA de lucru pt astazi este urmatoarea : 
    E1) parcurg directorul de output snapchot cu snapchot                                                       DONE 
    daca NU gasesc un snapchot cu numele identic 
         imi creez snapchotul folosind functia scriere_snapchot(...)                                            
                    altfel
         fac verificare de continut intre buffer_auxiliar si continutul snaphotului anterior   
    */

    /*
    pid_t pid;
    int status;_
    for(int i = 3 ; i < argc ; i++){
        inode_number = 0;
        strcpy(buffer_auxiliar,"");

        if((pid = fork()) == -1){
            perror("nu s a creat cu succes un nou ");
            exit(1);
        }
        if(pid == 0){//cod fiu
            parcurgere_director(argv[i],0,&inode_number,0,buffer_auxiliar);
            sprintf(snapchot_name, "snapchot_%d.txt",inode_number);
            fd2 = openat (fd_out,snapchot_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            exit(0);

            if((fd2 == -1)){
                perror("Eroare la crearea fisierului de iesire\n");
                exit(errno);
            }

            scriere_snapchot(fd2,buffer_auxiliar);
            close(fd2);
        }else{
            wait(&status);
            if(WIFEXITED(status)){
                printf("Child ended with code %d\n",WEXITSTATUS(status));
            }
            else{
                printf("Child ended abnorminally\n");
            }
            exit(0);
    }
    }
    close(fd_out);
    */
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

    //first step
    //verific in directorul de output daca mai exista fisierul meu de snapchot
    //NU - scriere_snapchot(...)
    //DA- acum trb sa verific continutul
    //E1) caut in directorul output fisierul cu inode-ul meu                                     WORKING ON IT
    //E2)compar continutul cu strcmp()
    //strcmp(snaphot_anterior,buffer) == 0 => nu modific
    //strcmp(snapchot_anterior,buffer) !=0 => scriere_snapchot


    //chestii de finete
    //1)ar trb sa vedem daca argumentele transmise ca parametrii sunt directoare

    //2) ar trb sa verificam daca output este un director + sa verific daca directorul output exista
    //sau trb sa l creez eu

    //sa ma gandesc sa fisierul de output se poate sa nu fie in working directory-SO-proiect
    //ci se poate sa fie in /bin/home si atunci trb sa fac

    //strcpy()
    //strcpy


    //16.04.2024
    //trb sa mi adaug cate un proces pentru fiecare arg in linie de comanda
    //recomandare prof : sa fac wait de n ori la final in proces parinte (adica cate snapchoturi imi creez atatea)
    //wait-uri trb sa am

    //cand pun un mesaj de eroare, neaparat trb sa se termine cu "\n", deoarece buffer-ul poate sa nu fie umplut
    //si asta inseamna ca se va afisa mesajul de eroare necorespunzator, cel mai probabil la final

    //folosesc de fiecare data wait ? 
    //si dupa ce proces astept ?
    //trb sa folosesc si  exec pentru ca eu nu am nevoie ca procesul fiu sa aiba si partea de verificare a
    //a nr de argumente date in linia de comanda
/*
    cerinta pe sapt asta
    
*/