#include <stdio.h>
#include "../include/screen_util.h"

// Fonction pour imprimer le menu
void print_menu(const char *menu[], int size, int selected, int grid_rows, int grid_cols, const char *theme) {
    // Effacer l'écran
    clear_screen();
    printf("\033[1mBienvenu dans les mots mélés\033[0m\n");
    move_cursor(3, 1);
    for (int i = 0; i < size; i++) {
        move_cursor(i + 3, 1);
        if (i == selected) {
            printf("\033[6m- %s\033[0m", menu[i]);
        } else {
            printf("- %s", menu[i]);
        }
    }
    move_cursor(size + 4, 1);
    printf("\033[32mTaille de la grille : %d lignes, %d colonnes\033[0m\n", grid_rows, grid_cols);
    move_cursor(size + 5, 1);
    printf("\033[32mThème sélectionné : %s\033[0m\n", theme);
}
