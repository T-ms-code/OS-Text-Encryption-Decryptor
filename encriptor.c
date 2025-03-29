#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>





//Consider ca un cuvant are maxim 100 de caractere.
int v1[1000], v2[1000], v3[1000], ok1=0,ok2=0, ok3=0, id1=0, id2=0, id3=0;



void genereaza_permutare(char* cuvant, int* permutare){
size_t lungime = strlen(cuvant);
for(size_t i = 0; i < lungime; i++){
        permutare[i] = i;
}
if(lungime <= 1) return;
for(int i = lungime-1; i >= 0; i--){//Nu mai folosesc size_t pentru i, caci poate deveni imens cand se decrementeaza 0. 
        size_t j = rand() % (i + 1);//Iau o pozitie random de la 0 la i.
        int aux = permutare[i];
        permutare[i] = permutare[j];
        permutare[j] = aux;
}

char aux[100];
strcpy(aux, cuvant);
for(size_t i = 0; i < lungime; i++){
        cuvant[i] = aux[permutare[i]];
}
}



void cripteaza(char *fis_mapat, size_t inceput, size_t sfarsit, int proces){
char cuvant[100];
size_t pozitie = inceput;

while(pozitie < sfarsit){
        size_t lungime_cuvant = 0;
        while(pozitie < sfarsit && (fis_mapat[pozitie] != ' ' && fis_mapat[pozitie] != '\n')){
                cuvant[lungime_cuvant] = fis_mapat[pozitie];
		lungime_cuvant++;
		pozitie++;
        }
        cuvant[lungime_cuvant] = '\0';
        if(lungime_cuvant > 0){
         	int permutare[100];
                genereaza_permutare(cuvant, permutare);
                int ok;
		switch (proces) {
                	case 0:
                		ok = ok1+lungime_cuvant;
                		for(int j = ok1; j < ok; j++)
                			v1[j] = permutare[j-ok1];
                		ok1 = ok;
                		break;
                	case 1:
                		ok = ok2+lungime_cuvant;
                		for(int j = ok2; j < ok; j++)
                			v2[j] = permutare[j-ok2];
                		ok2 = ok;
                		break;
                	case 2:
                		ok = ok3+lungime_cuvant;
                		for(int j = ok3; j < ok; j++)
                			v3[j] = permutare[j-ok3];
                		ok3 = ok;
                		break;
                }
                memcpy(fis_mapat + pozitie - lungime_cuvant, cuvant, lungime_cuvant);//Actualizam fisierul mapat.
        }
        while(pozitie < sfarsit && (fis_mapat[pozitie] == ' ' || fis_mapat[pozitie] == '\n')){
            pozitie++;
        }
}
}



void decripteaza(char *fis_mapat, size_t inceput, size_t sfarsit, FILE *fis_perm){
char cuvant[100];
size_t pozitie = inceput;

while(pozitie < sfarsit){
        size_t lungime_cuvant = 0;
        while(pozitie < sfarsit && (fis_mapat[pozitie] != ' ' && fis_mapat[pozitie] != '\n')){
                cuvant[lungime_cuvant] = fis_mapat[pozitie];
                lungime_cuvant++;
                pozitie++;
        }
        cuvant[lungime_cuvant] = '\0';
        if(lungime_cuvant > 0){
                int permutare[100];
                fread(permutare, sizeof(int), lungime_cuvant, fis_perm);
                for(size_t i = 0; i < lungime_cuvant; i++){
                    cuvant[permutare[i]] = fis_mapat[pozitie-lungime_cuvant + i];
                }
                memcpy(fis_mapat + pozitie - lungime_cuvant, cuvant, lungime_cuvant);//Actualizam fisierul mapat (in memorie).
        }
        while(pozitie < sfarsit && (fis_mapat[pozitie] == ' ' || fis_mapat[pozitie] == '\n')){
            pozitie++;
        }
}
}





int main(int argc, char* argv[]){

if(argc < 2 || argc>3){
fprintf(stderr, "Numarul de parametrii este gresit!");
return EXIT_FAILURE;
}

int utilizare;
char* fis_intrare = argv[1];
FILE *fis_perm;
if (argc == 3){
	utilizare = 2;//Decriptez
	char* fis_permutari = argv[2];
        fis_perm = fopen(fis_permutari, "rb");
	if(fis_perm == NULL){
                perror("Eroare la deschiderea fisierului cu  permutarile!");
                exit(EXIT_FAILURE);
        }
}
else
{
	utilizare = 1;//Encriptez
	fis_perm = fopen("permutari.bin", "wb");//Folosesc fisiere binare pentru precizie si pentru securitatea cheilor (in binar pot citi 10 in loc de 1 si apoi 0).
//Aleg fopen (fc. de nivel inalt) in loc de open, caci vreau o prelucrare mai complexa.
	if(fis_perm == NULL){
        	perror("Eroare la deschiderea fisierului cu  permutarile!");
        	exit(EXIT_FAILURE);
    	}
}

int fis_intr = open(fis_intrare, O_RDWR);
if(fis_intr == -1){
        perror("Eroare la deschiderea fisierului de cuvinte!");
        return 1;
}

struct stat informatii_fis;
if(fstat(fis_intr, &informatii_fis) < 0) {
        perror("Eroare la obtinerea informatiilor despre fisier!");
        close(fis_intr);
	fclose(fis_perm);
        exit(EXIT_FAILURE);
}
size_t dimensiune_fis = informatii_fis.st_size;

char *fis_mapat = mmap(NULL, dimensiune_fis, PROT_READ | PROT_WRITE, MAP_SHARED, fis_intr, 0);
if(fis_mapat == MAP_FAILED){
        perror("Eroare la maparea fisierului de intrare!");
        close(fis_intr);
        fclose(fis_perm);
        exit(EXIT_FAILURE);
}

///Pentru ca atunci cand criptez, procesele sa scrie cheile in ordinea dorita in fisierul de permutari
//, voi partaja in memorie o alta variabila, de ordine.
char *shm_name = "var_de_ordine";
size_t shm_size = sizeof(int);
int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
if(shm_fd == -1){
     perror("Eroare la shm_open!");
     exit(EXIT_FAILURE);
}
if(ftruncate(shm_fd, shm_size) == -1){
     perror("Eroare la ftruncate!");
     exit(EXIT_FAILURE);
}
int *var = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
if(var == MAP_FAILED){
     perror("Eroare la mmap!");
     exit(EXIT_FAILURE);
}
*var = 1;//Scrie primul procesul copil1.


pid_t pid=1;
if(utilizare == 1){//Voi folosi 3 procese copil in forma paralela care scriu intinial in 3 vectori,
//iar apoi se combina informatiile, pentru a se pastra
//corespondenta dintre locul cuvantului in fisierul de intrare si al
//permutarii in fisierul cu permutari.
size_t dimensiune_seg = dimensiune_fis / 3;
for(int i = 0; i <= 2 && pid!=0; i++){
        size_t inceput = i * dimensiune_seg;
        size_t sfarsit;
	if(i == 2){sfarsit = dimensiune_fis;}
	else sfarsit = (i + 1) * dimensiune_seg;
        pid = fork();
        if(pid == 0){
        	switch (i){
        	case 0: 
        		id1 = 1;
        		break;
        	case 1:
        		id2 = 1;
        		break;
        	case 2: 
        		id3 = 1;
        		break;
        	}
         	cripteaza(fis_mapat, inceput, sfarsit, i);
        }
    }
	
if(id1 == 1){//Copilul 1 scrie primul.
if(ok1 != 0){
printf("Proces_copil1--%d, proces_parinte--%d: ", getpid(), getppid());
for(int i = 0; i < ok1; i++) {
fwrite(&v1[i], sizeof(int), 1, fis_perm);
printf("%d ", v1[i]);
}
printf("\n");
}
*var = 2;//Pentru a se putea trece la urmatoarele scrieri.
}

if(id2 == 1){
while(*var != 2){//Copilul 2 sta aici pana ii vine randul.
}
if(ok2 != 0){
printf("Proces_copil2--%d, proces_parinte--%d: ", getpid(), getppid());
for(int i = 0; i < ok2; i++){
fwrite(&v2[i], sizeof(int), 1, fis_perm);
printf("%d ", v2[i]);
}
printf("\n");
}
*var = 3;
}

if(id3 == 1){
while(*var != 3){//Copilul 3 sta aici pana ii vine randul.
}
if(ok3 != 0){
printf("Proces_copil3--%d, proces_parinte--%d: ", getpid(), getppid());
for(int i = 0; i < ok3; i++){
fwrite(&v3[i], sizeof(int), 1, fis_perm);
printf("%d ", v3[i]);
}
printf("\n");
}
*var = 4;//Nu va mai urma nimic oricum.
}


if(id1 == 0 && id2 == 0 && id3 == 0){//sau: pid!=0, al doilea if nu mai e necesar.
while(*var != 4){//Asta se va intampla ultima oara.
}
if(pid != 0) {//Asteapta doar parintele/doar el are copii, copiii nu au copii.
for(int i = 0; i <= 2; i++)
	wait(NULL);
printf("Primul proces are id-ul %d\n", getpid());
}
}
	
}


//NU s-a specificat ca pentru decriptare ttrebuie utilizate procese.
//De asemenea, ea va trebui facuta tot in 3 parti, caci unele cuvinte au fost "sparte". Pot folosi 3 procese
//in ordine secventiala, dar este mai ineficient decat a nu folosi.
else{
decripteaza(fis_mapat, 0, dimensiune_fis/3, fis_perm);
decripteaza(fis_mapat, dimensiune_fis/3, 2*(dimensiune_fis/3), fis_perm);
decripteaza(fis_mapat, 2*(dimensiune_fis/3),  dimensiune_fis, fis_perm);
}


if(pid != 0){
if(msync(fis_mapat, dimensiune_fis, MS_SYNC) < 0) {
    perror("Eroare la sincronizarea fisierului de intrare!");
}//Face ca fisierul de pe disc/efectiv sa arate ca maparea din memorie.

if(munmap(fis_mapat, dimensiune_fis) < 0) {
     perror("Eroare la demaparea fisierului de intrare!");
}

close(fis_intr);
fclose(fis_perm);

if(munmap(var, shm_size) < 0){
	perror("Eroare la demaparea variabilei!");
}
if(shm_unlink(shm_name) == -1){
	perror("Eroare la delinkarea variabilei!");
}
}

}
