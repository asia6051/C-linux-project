#ifndef _PROPERTIES_H
#define _PROPERTIES_H

#define TRYB_STDIN 1  // tryb odczytu z stdin
#define TRYB_PLIK 2         // tryb odczytu z pliku

#define ROZMIAR_WEJSCIA 8000       // wielkość bufora danych
#define ROZMIAR_KOMUNIKATOW 64  // wielkosc bufora notyfikacji

#define LICZBA_SYGNALOW 64   // liczba wszystkich sygnałów

#define SIGNAL_STOP 20    // SIGTSTP
#define SIGNAL_RESUME 18  // SIGCONT
#define SIGNAL_END 15      // SIGTERM
#define SIGNAL_TO_DO 18  //  sygnał notyfikacji o rządaniu operatora

#define KOMUNIKAT_STOP "SIGTSTP"
#define KOMUNIKAT_CONT "SIGCONT"
#define KOMUNIKAT_TERM "SIGTERM"

#define SHM_SEM_LOCK 3

#endif