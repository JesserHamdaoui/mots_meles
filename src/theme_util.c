#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../include/screen_util.h"
#include "../include/theme_util.h"

// Fonction pour lister les fichiers de thèmes dans le dossier data
void list_themes(char *themes[], int *theme_count) {
    DIR *d;
    struct dirent *dir;
    d = opendir("data");
    if (d) {
        int i = 0;
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) { // Si c'est un fichier régulier
                // Obtenir le nom du fichier sans extension
                char *ext = strrchr(dir->d_name, '.');
                if (ext) {
                    size_t len = ext - dir->d_name;
                    themes[i] = malloc(len + 1);
                    strncpy(themes[i], dir->d_name, len);
                    themes[i][len] = '\0';
                    i++;
                }
            }
        }
        closedir(d);
        *theme_count = i;
    }
}

// Fonction pour choisir un thème
char* choose_theme() {
    char *themes[100];
    int theme_count = 0;
    list_themes(themes, &theme_count);

    int selected_theme = 0;
    while (1) {
        clear_screen();
        printf("\033[1mChoisir un thème\033[0m\n");
        move_cursor(3, 1);
        for (int i = 0; i < theme_count; i++) {
            move_cursor(i + 3, 1);
            if (i == selected_theme) {
                printf("\033[6m- %s\033[0m", themes[i]);
            } else {
                printf("- %s", themes[i]);
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
                    selected_theme = (selected_theme - 1 + theme_count) % theme_count;
                } else if (ch == 'B') { // Flèche vers le bas
                    selected_theme = (selected_theme + 1) % theme_count;
                }
            }
        } else if (ch == '\n') { // Touche Entrée
            // Gérer la sélection de thème ici
            clear_screen();
            char *selected = strdup(themes[selected_theme]);
            for (int i = 0; i < theme_count; i++) {
                free(themes[i]);
            }
            return selected;
        }
    }
    return NULL; // En cas de sortie du menu de sélection
}

// Fonction pour ajouter un nouveau thème
void add_new_theme() {
    char source_path[256];
    char dest_path[256];
    FILE *source_file, *dest_file;
    char ch;

    clear_screen();
    printf("Entrez le chemin du fichier texte à ajouter comme nouveau thème: ");
    scanf("%255s", source_path);
    getchar(); // Consommer le caractère de nouvelle ligne

    if (ch == 'q' || ch == 'Q') { // Quitter vers le menu principal
        clear_screen();
        return;
    }

    // Vérifier si le fichier source existe
    if (access(source_path, F_OK) == -1) {
        perror("Erreur : le fichier source n'existe pas");
        return;
    }

    // S'assurer que le répertoire data existe
    struct stat st = {0};
    if (stat("data", &st) == -1) {
        if (mkdir("data", 0700) == -1) {
            perror("Erreur lors de la création du répertoire data");
            return;
        }
    }

    // Obtenir le nom de fichier à partir du chemin source
    char *filename = strrchr(source_path, '/');
    if (filename) {
        filename++; // Passer le '/'
    } else {
        filename = source_path; // Aucun '/' trouvé, utiliser tout le chemin
    }

    // Créer le chemin de destination
    snprintf(dest_path, sizeof(dest_path), "data/%s", filename);

    // Ouvrir le fichier source
    source_file = fopen(source_path, "r");
    if (source_file == NULL) {
        perror("Erreur lors de l'ouverture du fichier source");
        return;
    }

    // Ouvrir le fichier de destination
    dest_file = fopen(dest_path, "w");
    if (dest_file == NULL) {
        perror("Erreur lors de la création du fichier destination");
        fclose(source_file);
        return;
    }

    // Copier le contenu du fichier source vers le fichier destination
    while ((ch = fgetc(source_file)) != EOF) {
        fputc(ch, dest_file);
    }

    printf("Thème ajouté avec succès !\n");
    printf("Chemin du fichier ajouté : %s\n", dest_path);

    // Fermer les fichiers
    fclose(source_file);
    fclose(dest_file);
}
