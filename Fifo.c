#include <stdio.h>
#include <string.h>
#include "Properties.h"
#define FIFO "fifo"  // nazwa pliku fifo

FILE *fifo_plik;
int fifo_rozmiar;

void readFromFifo(char *data) {
    if (fifo_plik = fopen(FIFO, "r")) {
        fgets(data, fifo_rozmiar + 1, fifo_plik);  //odczyt z fifo
        fclose(fifo_plik);
    } else {
        fprintf(stderr, "Blad otwierania pliku fifo");
        kill(getppid(), SIGNAL_END);
    }
}

void writeToFifo(char *data) {
    if (fifo_plik = fopen(FIFO, "w")) {
        fputs(data, fifo_plik);
        fclose(fifo_plik);
    } else {
        fprintf(stderr, "Blad otwierania pliku fifo");
        kill(getppid(), SIGNAL_END);
    }
    data[0] = '\0';  // usunięcie zawartości bufora
}

void createFifo(int size) {
    fifo_rozmiar = size;  // zapisanie wielkości odczytu
    umask(0);              // nadanie uprawnien do plikow, jesli ich nie ma
    mkfifo(FIFO, 0666);    // utworzenie fifo
}

void removeFifo() {
    char command[64] = "rm ";  // rm - usunięcie pliku
    strcat(command, FIFO);     // dopisanie nazwy pliku do komendy
    system(command);           // wykonanie komendy w systemie
}