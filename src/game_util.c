#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/game_util.h"
#include "../include/screen_util.h"

// Charger les mots à partir du fichier de thème sélectionné
void load_words(const char *selected_theme, char **words, int *word_count, int max_words) {
    // Construire le chemin du fichier à partir du thème sélectionné
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "data/%s.txt", selected_theme);

    // Ouvrir le fichier en lecture
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de thème");
        return;
    }

    // Initialiser le compteur de mots à 0
    *word_count = 0;

    // Boucle pour lire les mots du fichier jusqu'à la fin du fichier ou max_words est atteint
    char buffer[MAX_WORD_LENGTH];
    while (fgets(buffer, sizeof(buffer), file) != NULL && *word_count < max_words) {
        // Retirer le caractère de nouvelle ligne de chaque mot
        buffer[strcspn(buffer, "\n")] = '\0';

        // Allouer de la mémoire pour chaque mot et le stocker dans le tableau de mots
        words[*word_count] = strdup(buffer);
        if (words[*word_count] == NULL) {
            perror("Erreur d'allocation de mémoire pour le mot");
            fclose(file);
            return;
        }
        (*word_count)++;
    }

    // Fermer le fichier
    fclose(file);

    // Appeler shuffle_words pour mélanger le tableau de mots
    shuffle_words(words, *word_count);
}

// Mélanger les mots
void shuffle_words(char **words, int word_count) {
    // Initialiser la graine aléatoire
    srand(time(NULL));
    
    // Boucle à travers le tableau de mots
    for (int i = 0; i < word_count; i++) {
        // Générer un index aléatoire dans la plage du tableau
        int rand_index = rand() % word_count;
        
        // Échanger le mot actuel avec le mot à l'index aléatoire
        char *temp = words[i];
        words[i] = words[rand_index];
        words[rand_index] = temp;
    }
}

// Générer la grille avec les mots placés dans des orientations aléatoires
void generate_grid(char **grid, int rows, int cols, char **words, int word_count, WordPosition *word_positions) {
    // Initialiser la grille avec des espaces
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            grid[i][j] = ' ';
        }
    }

    // Définir les directions possibles : droite, gauche, bas, haut, bas-droite, bas-gauche, haut-droite, haut-gauche
    int directions[8][2] = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    // Initialiser word_positions pour indiquer les mots non placés
    for (int i = 0; i < word_count; i++) {
        word_positions[i].row = -1;
        word_positions[i].col = -1;
        word_positions[i].direction = -1;
    }

    // Boucle à travers chaque mot pour le placer dans la grille
    for (int w = 0; w < word_count; w++) {
        int len = strlen(words[w]); // Obtenir la longueur du mot actuel
        int placed = 0; // Initialiser un drapeau pour vérifier si le mot a été placé
        int tries = 0; // Compter le nombre de tentatives pour placer le mot

        // Essayer de placer le mot jusqu'à ce qu'il s'adapte ou que nous abandonnions
        while (!placed && tries < 100) {
            tries++;
            // Sélectionner aléatoirement une position de départ (ligne et colonne)
            int row = rand() % rows;
            int col = rand() % cols;

            // Sélectionner aléatoirement une direction parmi les directions possibles
            int dir_index = rand() % 8;
            int row_dir = directions[dir_index][0];
            int col_dir = directions[dir_index][1];

            // Vérifier si le mot s'adapte dans la direction sélectionnée sans dépasser les limites
            int fits = 1;
            for (int i = 0; i < len; i++) {
                int new_row = row + i * row_dir;
                int new_col = col + i * col_dir;

                if (new_row < 0 || new_row >= rows || new_col < 0 || new_col >= cols) {
                    fits = 0;
                    break;
                }
            }
            if (!fits) continue;

            // Vérifier si le mot chevauche des mots existants dans la grille
            for (int i = 0; i < len; i++) {
                int new_row = row + i * row_dir;
                int new_col = col + i * col_dir;

                if (grid[new_row][new_col] != ' ' && grid[new_row][new_col] != words[w][i]) {
                    fits = 0;
                    break;
                }
            }
            if (!fits) continue;

            // Si le mot s'adapte dans la position et la direction sélectionnées, le placer dans la grille
            for (int i = 0; i < len; i++) {
                int new_row = row + i * row_dir;
                int new_col = col + i * col_dir;
                grid[new_row][new_col] = words[w][i];
            }

            // Stocker la position et la direction du mot dans le tableau word_positions
            word_positions[w].row = row;
            word_positions[w].col = col;
            word_positions[w].direction = dir_index;

            // Définir le drapeau placed à true
            placed = 1;
        }
    }

    // Remplir les espaces vides restants dans la grille avec des lettres aléatoires
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (grid[i][j] == ' ') {
                grid[i][j] = 'A' + rand() % 26;
            }
        }
    }
}

// Vérifier si le mot sélectionné est dans la liste et le marquer comme trouvé
int check_word(const char *selected_word, char **words, int word_count, int *found) {
    for (int i = 0; i < word_count; i++) {
        if (!found[i] && strcmp(selected_word, words[i]) == 0) {
            found[i] = 1;
            return 1;
        }
    }
    return 0;
}

int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// Afficher la grille
void display_grid(char **grid, int rows, int cols, char **words, int word_count, int *found, WordPosition *word_positions, int start_row, int start_col, int end_row, int end_col, int selecting, int blink, int score) {
    // Effacer l'écran
    clear_screen();

    // Buffer pour stocker le mot sélectionné
    char selected_word[MAX_WORD_LENGTH] = {0};

    // Boucle à travers chaque cellule de la grille
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int found_flag = 0;
            int selecting_flag = 0;

            // Vérifier si la cellule fait partie d'un mot trouvé
            for (int w = 0; w < word_count; w++) {
                if (found[w]) {
                    int len = strlen(words[w]);
                    for (int k = 0; k < len; k++) {
                        int r = word_positions[w].row, c = word_positions[w].col;
                        switch (word_positions[w].direction) {
                            case 0: r = word_positions[w].row + k; break; // Bas
                            case 1: r = word_positions[w].row - k; break; // Haut
                            case 2: c = word_positions[w].col + k; break; // Droite
                            case 3: c = word_positions[w].col - k; break; // Gauche
                            case 4: r = word_positions[w].row + k; c = word_positions[w].col + k; break; // Bas Droite
                            case 5: r = word_positions[w].row + k; c = word_positions[w].col - k; break; // Bas Gauche
                            case 6: r = word_positions[w].row - k; c = word_positions[w].col + k; break; // Haut Droite
                            case 7: r = word_positions[w].row - k; c = word_positions[w].col - k; break; // Haut Gauche
                        }
                        if (r == i && c == j) {
                            found_flag = 1;
                        }
                    }
                }
            }

            // Vérifier si la cellule fait partie du mot en cours de sélection
            if (selecting) {
                int row_step = (end_row > start_row) - (end_row < start_row); // 1 si déplacement vers le bas, -1 si vers le haut, 0 si même ligne
                int col_step = (end_col > start_col) - (end_col < start_col); // 1 si déplacement vers la droite, -1 si vers la gauche, 0 si même colonne
                int len = abs(start_row - end_row) > abs(start_col - end_col) ? abs(start_row - end_row) : abs(start_col - end_col);

                for (int k = 0; k <= len; k++) {
                    int r = start_row + k * row_step;
                    int c = start_col + k * col_step;

                    if (r == i && c == j) {
                        selecting_flag = 1;
                        strncat(selected_word, &grid[r][c], 1);
                    }
                }
            }

            if (selecting_flag && blink) {
                printf("\033[43;30m%c\033[0m ", grid[i][j]); // Fond jaune pour les lettres sélectionnées
            } else if (i == end_row && j == end_col && blink) {
                printf("\033[5;47;30m%c\033[0m ", grid[i][j]); // Fond blanc clignotant
            } else if (found_flag) {
                printf("\033[44;30m%c\033[0m ", grid[i][j]); // Fond bleu pour les lettres trouvées
            } else {
                printf("\033[47;30m%c\033[0m ", grid[i][j]); // Fond blanc pour les lettres normales
            }
        }
        printf("\n");
    }

    // Afficher les listes de mots trouvés et non trouvés
    printf("\nMots trouvés: ");
    for (int i = 0; i < word_count; i++) {
        if (found[i]) {
            printf("%s ", words[i]);
        }
    }
    printf("\nMots à trouver: ");
    for (int i = 0; i < word_count; i++) {
        // Afficher uniquement les mots qui ont été placés sur la grille
        if (!found[i] && word_positions[i].row != -1 && word_positions[i].col != -1) {
            printf("%s ", words[i]);
        }
    }
    printf("\n");

    // Afficher le mot actuellement sélectionné
    if (selecting) {
        printf("\nMot en cours: %s\n", selected_word);
    }

    // Afficher le score actuel
    printf("\nScore: %d\n", score);
}

// Démarrer le jeu
void start_game(const char *selected_theme, int grid_rows, int grid_cols) {
    // Allouer de la mémoire pour la grille
    char **grid = malloc(grid_rows * sizeof(char *));
    for (int i = 0; i < grid_rows; i++) {
        grid[i] = malloc(grid_cols * sizeof(char));
    }

    // Allouer de la mémoire pour le tableau de mots
    char *words[MAX_WORDS];
    int word_count = 0;

    // Allouer de la mémoire pour le tableau des mots trouvés et l'initialiser à 0
    int *found = malloc(MAX_WORDS * sizeof(int));
    memset(found, 0, MAX_WORDS * sizeof(int));

    // Allouer de la mémoire pour le tableau des positions des mots
    WordPosition word_positions[MAX_WORDS];

    // Charger les mots à partir du thème sélectionné
    load_words(selected_theme, words, &word_count, MAX_WORDS);

    // Générer la grille avec les mots chargés
    generate_grid(grid, grid_rows, grid_cols, words, word_count, word_positions);

    // Initialiser les positions de départ et de fin pour la sélection des mots
    int start_row = 0, start_col = 0, end_row = 0, end_col = 0;
    int selecting = 0;
    int blink = 0;
    int score = 0;

    while (1) {
        // Afficher la grille avec la sélection actuelle, l'état de clignotement et le score
        display_grid(grid, grid_rows, grid_cols, words, word_count, found, word_positions, start_row, start_col, end_row, end_col, selecting, blink, score);
        usleep(500000); // Délai de 500 ms pour l'effet de clignotement
        blink = !blink;

        // Vérifier si une touche a été pressée
        if (kbhit()) {
            char ch = getchar();

            // Gérer l'entrée utilisateur pour la navigation et la sélection
            if (ch == 'q' || ch == 'Q') { // Quitter vers le menu principal
                clear_screen();
                break;
            } else if (ch == '\n') { // Touche Entrée pour commencer ou terminer la sélection
                if (selecting == 0) {
                    start_row = end_row;
                    start_col = end_col;
                    selecting = 1;
                } else {
                    // Former le mot sélectionné
                    char selected_word[MAX_WORD_LENGTH] = {0};
                    int row_step = (end_row > start_row) - (end_row < start_row); // 1 si déplacement vers le bas, -1 si vers le haut, 0 si même ligne
                    int col_step = (end_col > start_col) - (end_col < start_col); // 1 si déplacement vers la droite, -1 si vers la gauche, 0 si même colonne
                    int len = abs(start_row - end_row) > abs(start_col - end_col) ? abs(start_row - end_row) : abs(start_col - end_col);

                    for (int i = 0; i <= len; i++) {
                        int r = start_row + i * row_step;
                        int c = start_col + i * col_step;
                        strncat(selected_word, &grid[r][c], 1);
                    }

                    if (check_word(selected_word, words, word_count, found)) {
                        // Mot trouvé, augmenter le score
                        score += 10; // Augmenter le score de 10 pour chaque mot trouvé
                        // Mettre à jour l'affichage
                        display_grid(grid, grid_rows, grid_cols, words, word_count, found, word_positions, start_row, start_col, end_row, end_col, selecting, blink, score);
                    }
                    selecting = 0;

                    // Vérifier si tous les mots sont trouvés
                    int all_found = 1;
                    for (int i = 0; i < word_count; i++) {
                        if (!found[i]) {
                            all_found = 0;
                            break;
                        }
                    }
                    if (all_found) {
                        clear_screen();
                        printf("Félicitations ! Vous avez trouvé tous les mots.\n");
                        printf("Score final : %d\n", score);
                        printf("Appuyez sur 'q' pour retourner au menu principal.\n");
                        while (1) {
                            if (kbhit()) {
                                char ch = getchar();
                                if (ch == 'q' || ch == 'Q') {
                                    clear_screen();
                                    return;
                                }
                            }
                        }
                    }
                }
            } else if (ch == '8') { // Déplacer vers le haut
                end_row = (end_row - 1 + grid_rows) % grid_rows;
            } else if (ch == '2') { // Déplacer vers le bas
                end_row = (end_row + 1) % grid_rows;
            } else if (ch == '6') { // Déplacer vers la droite
                end_col = (end_col + 1) % grid_cols;
            } else if (ch == '4') { // Déplacer vers la gauche
                end_col = (end_col - 1 + grid_cols) % grid_cols;
            } else if (ch == '7') { // Déplacer vers le haut-gauche
                end_row = (end_row - 1 + grid_rows) % grid_rows;
                end_col = (end_col - 1 + grid_cols) % grid_cols;
            } else if (ch == '3') { // Déplacer vers le bas-droite
                end_row = (end_row + 1) % grid_rows;
                end_col = (end_col + 1) % grid_cols;
            } else if (ch == '9') { // Déplacer vers le haut-droite
                end_row = (end_row - 1 + grid_rows) % grid_rows;
                end_col = (end_col + 1) % grid_cols;
            } else if (ch == '1') { // Déplacer vers le bas-gauche
                end_row = (end_row + 1) % grid_rows;
                end_col = (end_col - 1 + grid_cols) % grid_cols;
            }
        }
    }

    // Libérer la mémoire allouée
    for (int i = 0; i < grid_rows; i++) {
        free(grid[i]);
    }
    free(grid);

    for (int i = 0; i < word_count; i++) {
        free(words[i]);
    }

    free(found);
}
