#include <stdio.h>
#include <stdlib.h>
#include "../include/screen_util.h"
#include "../include/grid_util.h"

// Fonction pour choisir la taille de la grille
void choose_grid_size(int *rows, int *cols) {
    const char *grid_menu[] = {
        "Changer la taille des lignes",
        "Changer la taille des colonnes",
        "Retourner au menu principal (Q)"
    };
    int menu_size = sizeof(grid_menu) / sizeof(grid_menu[0]);
    int selected = 0;

    while (1) {
        clear_screen();
        printf("\033[1mChoisir la taille de la grille\033[0m\n");
        move_cursor(3, 1);
        for (int i = 0; i < menu_size; i++) {
            move_cursor(i + 3, 1);
            if (i == selected) {
                printf("\033[6m- %s\033[0m", grid_menu[i]);
            } else {
                printf("- %s", grid_menu[i]);
            }
        }

        char ch = getchar();
        if (ch == 'q' || ch == 'Q') { // Quitter vers le menu principal
            clear_screen();
            break;
        } else if (ch == 27) { // Séquence d'échappement
            ch = getchar();
            if (ch == 91) { // Touches fléchées
                ch = getchar();
                if (ch == 'A') { // Flèche vers le haut
                    selected = (selected - 1 + menu_size) % menu_size;
                } else if (ch == 'B') { // Flèche vers le bas
                    selected = (selected + 1) % menu_size;
                }
            }
        } else if (ch == '\n') { // Touche Entrée
            if (selected == 0) { // Changer la taille des lignes
                clear_screen();
                printf("Entrer la nouvelle taille des lignes: ");
                scanf("%d", rows);
                getchar(); // Consommer le caractère de nouvelle ligne
            } else if (selected == 1) { // Changer la taille des colonnes
                clear_screen();
                printf("Entrer la nouvelle taille des colonnes: ");
                scanf("%d", cols);
                getchar(); // Consommer le caractère de nouvelle ligne
            } else if (selected == 2) { // Retourner au menu principal
                break;
            }
        }
    }
}
