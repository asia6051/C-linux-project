#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "Properties.h"

FILE *PIDy_plik;            //wskaznik na plik z pidami
int pids[3];  // przechowuje PID'y procesów potomnych

int next() {  // zwraca numer porządkowy następnego procesu
    int pid = getpid();
    return (pid == pids[0]) + 2 * (int)(pid == pids[1]);
}
int previous() {  // zwraca numer porządkowy następnego procesu
    int pid = getpid();
    return (pid == pids[2]) + 2 * (int)(pid == pids[0]);
}
int getSavedPid(int index_procesu) {
    return pids[index_procesu];
}

void savePid(int index_procesu, int pid) {
    pids[index_procesu] = pid;
}

void exportPids() {  // zapisanie pidów do pliku PID_info.txt
    if (PIDy_plik = fopen("PID_info.txt", "w")) {
        int i;
        for (i = 0; i < 3; i++) fprintf(PIDy_plik, "%d ", pids[i]);
        fprintf(PIDy_plik, " Rodzic: %d", getpid());
        fclose(PIDy_plik);
    } else {
        fprintf(stderr, "Plik przechowujacy PIDy nie zostal utworzony!\n");
    }
}

void getPids() {  // odczytanie pidów z pliku PID_info.txt
    if (PIDy_plik = fopen("PID_info.txt", "r")) {
        int i;
        for (i = 0; i < 3; i++) fscanf(PIDy_plik, "%d", &pids[i]);
        fclose(PIDy_plik);
    } else {
        fprintf(stderr, "Uwaga! Nie znaleziono pliku PID_info.txt");
    }
}

void removePidsFile() {
    system("rm PID_info.txt");
}