#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "Fifo.c"
#include "Pids.c"
#include "Properties.h"
#include "Semaphore.c"
#include "SharedMemory.c"

int aktualny_proces;        // nr porządkowy obecnego procesu
int tryb = 0;        // tryb odczytu danych
int flaga = 0;  // flaga wstrzymania działania

char data[ROZMIAR_WEJSCIA];             // bufor danych
char liczba_znakow[ROZMIAR_WEJSCIA];  // zmienna przechuwująca ilość znaków

char otrzymany_sygnal[ROZMIAR_KOMUNIKATOW];
char signalToSend[ROZMIAR_KOMUNIKATOW];

void cleanUp() {
    removePidsFile();         // usuniecie pliku z pidami
    removeSemaphores();       // usunięcie semaforów
    removeFifo();             // usunięcie fifo
    removeSharedMemory();     // usunięcie kolejki
    kill(getpid(), SIGKILL);  // samobójstwo procesu
}

void mainHandler(int signal) {  //obsluga sygnalow procesu macierzystego
    if (signal == SIGNAL_END) {
        printf("\nTu proces macierzysty: %d, Koncze dzialanie\n", getpid());

        semlock(SHM_SEM_LOCK);  // zajęcie pamięci współdzielonej
        strcpy(signalToSend, KOMUNIKAT_TERM);
        writeToSharedMemory(signalToSend);  // zapisanie sygnału do pamięci współdzielonej
        kill(getSavedPid(0), SIGNAL_TO_DO);  // SIGKILL konczy pracę procesu
        semunlock(SHM_SEM_LOCK);  // odblokowanie pamięci współdzielo

        cleanUp();  // usunięcie używanych zasobów przez proces macierzysty
    }else if ((signal == SIGNAL_RESUME && flaga) || (signal == SIGNAL_STOP && !flaga)) {
        semlock(SHM_SEM_LOCK);  // zajęcie pamięci współdzielonej
        strcpy(signalToSend, signal == SIGNAL_STOP ? KOMUNIKAT_STOP : KOMUNIKAT_CONT);
        writeToSharedMemory(signalToSend);  // zapisanie sygnału do pamięci współdzielonej
        kill(getSavedPid(0), SIGNAL_TO_DO);  // SIGKILL konczy pracę procesu

        flaga = (signal == SIGNAL_STOP);  // ustawienie flagi wstrzymania
        if (signal == SIGNAL_STOP) printf("Proces macierzysty wstrzymuje prace!\n");
        if (signal == SIGNAL_RESUME) printf("Proces macierzysty wznawiam prace!\n");

        semunlock(SHM_SEM_LOCK);  // odblokowanie pamięci współdzielonej
    }

}

void childHandler(int signal) {
    if ((aktualny_proces == 1) && (signal != SIGNAL_TO_DO)) {  // obsluga sygnalu od procesu macierzystego
        printf("\nTu proces potomny: %d, otrzymał sygnal: %d\n", aktualny_proces, signal);
        kill(getppid(), signal);
    } 
    if (signal == SIGNAL_TO_DO) {
        semlock(SHM_SEM_LOCK);

        readFromSharedMemory(otrzymany_sygnal);

        printf("\nProces %d otrzymal sygnal: %s\n",aktualny_proces+1, otrzymany_sygnal);

        if (!strcmp(otrzymany_sygnal, KOMUNIKAT_TERM))
        {
            writeToSharedMemory(otrzymany_sygnal);  // zapisanie sygnału do pamięci współdzielonej
            kill(getSavedPid(next()), SIGNAL_TO_DO);      // powiadomienie następnego procesu o otrzymaniu sygnału
            semunlock(SHM_SEM_LOCK);
            kill(getpid(), SIGKILL);  // samobójstwo procesu      
        }

        flaga = !strcmp(otrzymany_sygnal, KOMUNIKAT_STOP);  // ustawienie flagi wstrzymania (strcmp zwraca 0, jesli takie same)
        
        if (aktualny_proces != 2)
        {
            writeToSharedMemory(otrzymany_sygnal);  // zapisanie sygnału do pamięci współdzielonej
            kill(getSavedPid(next()), SIGNAL_TO_DO);      // powiadomienie następnego procesu o otrzymaniu sygnału     
        }   
        semunlock(SHM_SEM_LOCK);
    }
    return;
}

void menu() {
    fprintf(stderr,"Wybierz skad odbierac dane:\n");
    fprintf(stderr,"1. Z klawiatury (pojedyncza kropka konczy wczytywanie)\n");
    fprintf(stderr,"2. Z pliku tekstowego\n");
    char trybjaki = fgetc(stdin);
    tryb = trybjaki - '0';
    if(tryb != TRYB_STDIN && tryb != TRYB_PLIK) {
        printf("Podano bledna wrtosc\n");
        menu();
    }
}
void czytaj_z_stdin(char *data) {
    if (tryb == TRYB_STDIN) {
        fprintf(stderr, "\nPodaj dane:\n");
    }

    while (!fgets(data, ROZMIAR_WEJSCIA + 1, stdin)) {  // wczytanie strumienia wejścia do bufora
        while (1) pause();
    }
    data[strcspn(data, "\r\n")] = '\0';  // usunięcie entera
    if(strcmp(data, ".") == 0) {
        menu();
        return;
    }
}


void czytaj_z_pliku(char *data) {
    fprintf(stderr, "Podaj nazwe pliku z rozszerzeniem:\n");
    FILE *plikWe;
    char nazwa_pliku[30];
    fscanf(stdin, "%s", nazwa_pliku);
    if(plikWe = fopen(nazwa_pliku, "r")) {
        //fprintf(stderr, "Otworzylem plik\n");
        while(fgets(data, ROZMIAR_WEJSCIA, plikWe) != NULL) //odczyt z pliku
        {
            data[strcspn(data, "\r\n")] = '\0';
            semunlock(next());
            writeToFifo(data);
            semlock(aktualny_proces);
        }
        fclose(plikWe);
        //fprintf(stderr, "Zamknalem plik\n");
    } else {
        fprintf(stderr, "Nie mozna owtorzyc pliku");
        kill(getpid(), SIGNAL_END);
    }
    menu();
}

void P1() {
    while (1) {
        semlock(aktualny_proces);
        //printf("\nTu proces: %d, Czytam dane\n", getpid());
        if(tryb == TRYB_STDIN) czytaj_z_stdin(data);        // wywołanie funkcji odczytu
        else if(tryb == TRYB_PLIK) czytaj_z_pliku(data);
        while (flaga) pause();  // wstrzymanie pracy
        //printf("[P1:%d] Dane: %s\n", getpid(), data);
        semunlock(next());
        writeToFifo(data);
    }
}

void P2() {
    while (1) {
        semlock(aktualny_proces);
        while (flaga) pause();
        readFromFifo(data);
        sprintf(liczba_znakow, "%d", (int)strlen(data));  // strlen - liczba bajtów danych
        //printf("[P2:%d] Dane: %s\n", getpid(), data);
        //printf("[P2:%d] Ilosc znakow: %s\n", getpid(), liczba_znakow);
        data[0] = '\0';  // wyczyszczenie bufora danych
        semunlock(next());
        writeToFifo(liczba_znakow);
    }
}

void P3() {
    while (1) {
        semlock(aktualny_proces);
        while (flaga) pause();
        readFromFifo(liczba_znakow);
        //printf("[P3:%d] Dane: %s\n", getpid(), liczba_znakow);
        printf("%s\n", liczba_znakow);
        fflush(stdout);
        semunlock(next());
    }
}

void createProcesy() {
    void (*procesy[3])(void) = {P1, P2, P3};  // wskaźniki funckji procesów
    int index_procesu;
    for (index_procesu = 0; index_procesu < 3; index_procesu++) {
        // tutaj tworzony jest proces potomny (funkcja fork())
        savePid(index_procesu, fork());         // zapisanie pidu do tablicy pidów
        if (getSavedPid(index_procesu) == 0) {  // pid == 0 oznacza, że jest to proces potomny
            // zapisanie numeru porządkowego
            aktualny_proces = index_procesu;

            // wyswietlenie informacji o procesie potomnym
            printf("Proces %d PID:%d PPID:%d\n", aktualny_proces + 1, getpid(), getppid());

            // obsługa wszystkich sygnałów w podprocesach
            int i;
            for (i = 1; i <= LICZBA_SYGNALOW; i++) signal(i, childHandler);

            // pobranie PID'ów pozostałych procesów z pliku
            semlock(aktualny_proces);      // wstrzymanie do czasu odblokowania
            getPids();          // odczytanie pidów z pliku
            semunlock(next());  // umożliwienie odczytu kolejnemu procesowi

            procesy[aktualny_proces]();  //wywołanie odpowiedniej funkcji dla procesu
        }
    }
}

int main() {
    printf("Proces macierzysty PID:%d PPID:%d\n", getpid(), getppid());
    createFifo(ROZMIAR_WEJSCIA);                 // stworzenie kolejki komunikatów
    createSharedMemory(ROZMIAR_KOMUNIKATOW);  // stworzenie pamięci współdzielonej
    createSemaphores();                     // utworzenie semaforów
    semunlock(SHM_SEM_LOCK);                // odblokowanie semafora pamięci współdzielonej

    int i;
    for (i = 1; i <= LICZBA_SYGNALOW; i++) signal(i, mainHandler);  // obsługa wszystkich sygnałów

    menu();
    createProcesy();  // stworzenie procesów
    exportPids();       // wyeksportowanie pidów do pliku
    semunlock(0);       // odblokowanie pierwszego procesu

    while (1) pause();  // wsytrzymanie pracy procesu macierzystego
    return 0;
}