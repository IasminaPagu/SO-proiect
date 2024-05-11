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
#include <libgen.h>

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
   //are vreun efect chestia asta  ??-eu cred ca nu, deoarece am dat return inainte
}
void parcurgere_director(char *nume_director, int nivel, int *inode_number, int contor , char buffer_auxiliar[],int *nr_fisiere_malitioase, char *nume_director_fisiere_investate){
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
            parcurgere_director(cale_relativa, nivel + 3, inode_number, contor + 1, buffer_auxiliar,nr_fisiere_malitioase, nume_director_fisiere_investate);
        }else if(S_ISLNK(info.st_mode)){
            //sincera sa fiu, nu stiu daca trebuie sa verific si pentru link
            //daca are drepturile lipsa
            snprintf(buffer,sizeof(buffer)," %sLINK  %s\t: Dimensiune %ld bytes ,  inode number %ld , Time of last modification %s\n",spatii,cale_relativa,info.st_size, info.st_ino,ctime(&info.st_mtime));
            strcat(buffer_auxiliar,buffer);
        }
        else if(S_ISREG(info.st_mode)){ 
            
        
          if( (!( info.st_mode & S_IRWXU )) && (!( info.st_mode & S_IRWXG )) && (!( info.st_mode & S_IRWXO )) ){//daca fisierul meu n-are drepturi
                pid_t pid;
                int status;
                int pfd[2];
                FILE *stream;
                char mesaj_nepot[100];
                //fiul imi creeaza un pipe
                if(pipe(pfd)<0){
                    perror("Eroare la crearea pipe-ului");
                    exit(errno);
                }
                if((pid=fork())<0)
                {
                    perror("Eroare la fork nepot\n");
                    exit(errno);
                }
                if(pid==0) /* procesul fiu--adica nepot */
                {
                   close(pfd[0]);//inchid capatul de citire, deoarece fiul(nepotul) imi scrie
                   dup2(pfd[1],1);//imi redirectez iesirea standard spre pipe
                   
                   execl("/bin/bash","sh","verify_for_malicious.sh",cale_relativa,NULL);
                   perror("Eroare la exec\n"); 
                   exit(errno);
                    /* Daca execlp s-a intors, inseamna ca programul
                    nu a putut fi lansat in executie */
                }
                else /* procesul parinte--adica fiu */
                {
                    close(pfd[1]);//inchid capatul de scriere, pentru ca eu doar citesc
                    stream = fdopen(pfd[0],"r");
                    fscanf(stream,"%s",mesaj_nepot);

                    if(strcmp(mesaj_nepot,"SAFE") == 0){//daca fisierul e declarat ca fiind safe il scriu in snapchot
                    //daca nu, trebuie sa l mut in izolated_dir
                        snprintf(buffer,sizeof(buffer),"%s FILE %s\t: Dimensiune %ld bytes ,  inode number %ld , Time of last modification %s\n",spatii,cale_relativa,info.st_size, info.st_ino,ctime(&info.st_mtime));
                        strcat(buffer_auxiliar,buffer);
                    }
                    else{
                       char path[100];
                       sprintf(path, "%s/%s", nume_director_fisiere_investate,basename(cale_relativa)); //am folosit basename ca sa am doar numele fisierului,
                       //daca foloseam nume_fisier aveam de ecd xemplu /dir/b.c
                       int val_rename = rename(cale_relativa, path);

                       if(val_rename == -1){
                        perror("eroare rename\n");
                        exit(errno);
                       }
                       *nr_fisiere_malitioase = *nr_fisiere_malitioase + 1;

                    }

                    close(pfd[0]);
                    pid_t wpid = waitpid(pid,&status,0);
                    if (wpid == -1) {
                        perror("eroare waitpid fiu\n");
                        exit(errno);
                    }
                }
            }
            else{
                snprintf(buffer,sizeof(buffer),"%s FILE %s\t: Dimensiune %ld bytes ,  inode number %ld , Time of last modification %s\n",spatii,cale_relativa,info.st_size, info.st_ino,ctime(&info.st_mtime));
                strcat(buffer_auxiliar,buffer);
            }
            /*
            Ideea este urmatoarea, daca fisierul meu NU ARE drepturi , il verific cu scriptul
            daca scriptul zice SAFE - imi scriu in snapchot
                               NUME_FISIER - nu mai are rost sa il scriu si il mut din directorul curent in isolated_space_dir

                               daca fisierul meu ARE drepturi -- imi scriiu direct in snapchot
            */
        }
    }
    closedir(dir);
}
int verificare_director_argument_in_linia_de_comanda(char *nume_argument){
    //returneaza 1 daca argumentul dat ca si argument functiei este director
    //altfel returneaza 0
    struct stat info;
    int valoare_lstat;

    valoare_lstat = lstat(nume_argument,&info);
    if(valoare_lstat == -1){
        perror("Nu s-au putut afla atributele fisierului\n");
        exit(errno);
    }
    if( S_ISDIR(info.st_mode)){
        return 1;
    }
    else{
        return 0;
        }
}

int main(int argc, char *argv[]){

    char cale_director[256]="";
    char snapchot_name[100];
    int inode_number = 0;
    char buffer_auxiliar[BUFFER_SIZE];//este buffer-ul in care imi stochez parcurgerea_directorului

    pid_t pid ,wpid;
    int status;

    if(argc > 15){
        printf("Nu ati transmis numarul potrivit de argumente in linia de comanda\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 5 ; i < argc ; i++){

        inode_number = 0;
        strcpy(buffer_auxiliar,"");
        //verific daca argumentul este director, daca nu, trec peste
        if(verificare_director_argument_in_linia_de_comanda(argv[i]) == 1){//inseamna ca argumentul in linie de comanda este director
            pid = fork();
            if (pid == -1){
                printf("eroare fork\n");
                exit(1);
            }
            if(pid == 0){//cod fiu
                int nr_fisiere_malitioase=0;
                parcurgere_director(argv[i],0, &inode_number, 0,buffer_auxiliar,&nr_fisiere_malitioase,argv[4]);

                sprintf(snapchot_name, "snapchot_%d.txt",inode_number);
                strcpy(cale_director, argv[2]);
                strcat(cale_director, "/");
                strcat(cale_director,snapchot_name);
                
                if(verificare_exista_snapchot_anterior(argv[2],snapchot_name) == 0){//inseamna ca noul snapchot nu exista in directorul de output
                    //si atunci il creez
                    scriere_snapchot(cale_director,buffer_auxiliar);
                    printf("Snapchotul pentru directorul %s nu exista inainte de acest apel\n",argv[i]);
                }else{
                    //compar ce exista deja in snapchot_name cu buffer_auxiliar pe care l am obtinut prin apelarea functiei parcurgere_director(...)
                    if( (comparare_snapchot_anterior(cale_director,buffer_auxiliar) == 0) ){//inseamna ca nu s-a facut nicio modificare in snapchot
                        printf("nu s a produs nicio modificare fata de snapchotul anterior--pentru directorul %s\n",argv[i]);
                        //continue;
                    }
                    else{
                        scriere_snapchot(cale_director, buffer_auxiliar);
                        printf("a existat o modificare--pentru directorul %s\n",argv[i]);
                    }
                }
                exit(nr_fisiere_malitioase);
            }
        }
        else{
            printf("\nargumentul %s nu este un director !!\n",argv[i]);
            continue;
        }
    }

    //proces parinte
    int nr_copil=0;
    for(int i = 5 ; i < argc ; i++){
        nr_copil++;
        if(verificare_director_argument_in_linia_de_comanda(argv[i]) == 1){
            //doar daca argumentul este un director se creeaza un proces pentru el
            //daca nu puneam aceasta conditie aparea mesajul : "waitpid: No child processes "
            //dar facea totul corect in rest
            wpid = wait(&status);
            if (wpid == -1) {
                    perror("eroare waitpid parinte\n");
                    exit(errno);
            }
            if(WIFEXITED(status)){
            printf("Procesul Copil %d s-a incheiat cu PID-ul  %d  si cu %d fisiere potential periculoase\n",nr_copil,wpid,WEXITSTATUS(status));//WEXITSTATUS ne da codul de retur, gen exit(0)
            printf("\n");
            }
            else{
                printf("Child %d ended abnormally\n", wpid);
            }
            }

    }
        return 0;

}
    /*  IDEEA de lucru pt astazi este urmatoarea : 
    E1) parcurg directorul de output snapchot cu snapchot                                                       DONE 
    daca NU gasesc un snapchot cu numele identic 
         imi creez snapchotul folosind functia scriere_snapchot(...)                                            
                    altfel
         fac verificare de continut intre buffer_auxiliar si continutul snaphotului anterior   
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
    //E1) caut in directorul output fisierul cu inode-ul meu                                     DONE
    //E2)compar continutul cu strcmp()
    //strcmp(snaphot_anterior,buffer) == 0 => nu modific
    //strcmp(snapchot_anterior,buffer) !=0 => scriere_snapchot


    /*
    trb sa verific ca argumentul din linia de comanda este director                                    DONE

    cand lansez in executie cu exec , eu nu am drepturi asupra fisierului meu
    deci in script prima oara ii dau drepturi

    trb sa mut in codul meu din C fisierul din directorul transmis ca parametru in linia de comanda,
    in directorul de fisiere_malitioase
    cu rename
    si dupa il sterg din directorul meu cu remove

    inf circula prin pipe nepot-fiu
    inf circula prin cod de return fiu-parinte

    */

    //E1) trb sa verific daca fisierul are toate drepturile de access lipsa
    //gen chmod 000                                                                                     DONE
    

    /*
    PENTRU fisierele gasite fara niciun drept, trebuie sa le verific 
    folosind fisierul bash verify_for_malicious.sh
    --trb sa folosesc exec si asa mai departe
                                                                                                        DONE
    PENTRU fisierele descoperite ca fiind malitioase, trb sa le mut 
    intr-un director separat numit : "izolated_director" --- chestia asta face fiul

                        int rename(const char *oldpath, const char *newpath);
                        //On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.
                        rename(cale_relativa,cale_isolated_space_dir);
                        printf("%s FISIER investat \n",mesaj_nepot);

    */
   /*
    procesul parinte se ocupa doar cu codurile de retur, adica in fiu pun exec(nr_fisiere_malitioase)               DONE
    */





    //chestii de finete
    //1)ar trb sa vedem daca argumentele transmise ca parametrii sunt directoare                                DONE

    //2) ar trb sa verificam daca output este un director + sa verific daca directorul output exista
    //sau trb sa l creez eu

    //sa ma gandesc sa fisierul de output se poate sa nu fie in working directory-SO-proiect
    //ci se poate sa fie in /bin/home si atunci trb sa fac


    //16.04.2024
    //trb sa mi adaug cate un proces pentru fiecare arg in linie de comanda
    //recomandare prof : sa fac wait de n ori la final in proces parinte (adica cate snapchoturi imi creez atatea)
    //wait-uri trb sa am

    //cand pun un mesaj de eroare, neaparat trb sa se termine cu "\n", deoarece buffer-ul poate sa nu fie umplut
    //si asta inseamna ca se va afisa mesajul de eroare necorespunzator, cel mai probabil la final

    //folosesc de fiecare data wait ?       DA, DEOARECE EU ASTEPT DUPE FIECARE COPIL SA SE TERMINE
    //si dupa ce proces astept ?            PARALELISM
    //trb sa folosesc si  exec pentru ca eu nu am nevoie ca procesul fiu sa aiba si partea de verificare a ---- ASTA NU TREBUIE
    //a nr de argumente date in linia de comanda