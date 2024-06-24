#include <stdio.h>
#include "../include/screen_util.h"

// Fonction pour effacer l'écran
void clear_screen() {
    printf("\033[H\033[J");
}

// Fonction pour déplacer le curseur à une position donnée
void move_cursor(int row, int col) {
    printf("\033[%d;%dH", row, col);
}
