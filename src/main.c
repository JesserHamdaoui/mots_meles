#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../include/termios_util.h"
#include "../include/screen_util.h"
#include "../include/menu_util.h"
#include "../include/theme_util.h"
#include "../include/grid_util.h"
#include "../include/game_util.h"

char *selected_theme = NULL;
int grid_rows = 10; // Taille de grille par défaut
int grid_cols = 10; // Taille de grille par défaut

int main() {
    // Initialiser le thème sélectionné à "Animaux"
    selected_theme = strdup("Animaux");

    const char *menu[] = {
        "Jouer",
        "Choisir un thème",
        "Choisir la taille de la grille",
        "Ajouter une nouvelle liste de mots",
        "Quitter le jeu (Q)"
    };
    int menu_size = sizeof(menu) / sizeof(menu[0]);
    int selected = 0;

    set_raw_mode(1);

    print_menu(menu, menu_size, selected, grid_rows, grid_cols, selected_theme);

    while (1) {
        char ch = getchar();
        if (ch == 27) { // Séquence d'échappement
            ch = getchar();
            if (ch == 91) { // Touches fléchées
                ch = getchar();
                if (ch == 'A') { // Flèche vers le haut
                    selected = (selected - 1 + menu_size) % menu_size;
                } else if (ch == 'B') { // Flèche vers le bas
                    selected = (selected + 1) % menu_size;
                }
                print_menu(menu, menu_size, selected, grid_rows, grid_cols, selected_theme);
            }
        } else if (ch == 17) { // Ctrl+Q
            clear_screen();
            print_menu(menu, menu_size, selected, grid_rows, grid_cols, selected_theme);
        } else if (ch == '\n') { // Touche Entrée
            if (selected == 0) { // Option pour démarrer le jeu
                start_game(selected_theme, grid_rows, grid_cols);
                print_menu(menu, menu_size, selected, grid_rows, grid_cols, selected_theme);
            } else if (selected == 1) { // Option pour choisir un thème
                char *new_theme = choose_theme();
                if (new_theme != NULL) {
                    free(selected_theme); // Libérer le thème précédent
                    selected_theme = new_theme; // Mettre à jour avec le nouveau thème
                    clear_screen();
                    printf("Vous avez sélectionné le thème : %s\n", selected_theme);
                }
                print_menu(menu, menu_size, selected, grid_rows, grid_cols, selected_theme);
            } else if (selected == 2) { // Option pour choisir la taille de la grille
                choose_grid_size(&grid_rows, &grid_cols);
                clear_screen();
                printf("Taille de la grille : %d lignes, %d colonnes\n", grid_rows, grid_cols); 
                print_menu(menu, menu_size, selected, grid_rows, grid_cols, selected_theme);
            } else if (selected == 3) { // Option pour ajouter un nouveau thème
                add_new_theme();
                print_menu(menu, menu_size, selected, grid_rows, grid_cols, selected_theme);
            } else {
                // Gérer d'autres options de menu ici
                move_cursor(menu_size + 7, 1);
                break;
            }
        }
    }

    set_raw_mode(0);
    free(selected_theme); // Libérer le thème sélectionné à la fin
    return 0;
}
