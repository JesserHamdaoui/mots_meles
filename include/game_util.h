#ifndef GAME_UTIL_H
#define GAME_UTIL_H

#define MAX_WORDS 100
#define MAX_WORD_LENGTH 50

typedef struct {
    int row;
    int col;
    char direction;
} WordPosition;

void load_words(const char *selected_theme, char **words, int *word_count, int max_words);
void shuffle_words(char **words, int word_count);
void generate_grid(char **grid, int rows, int cols, char **words, int word_count, WordPosition *word_positions);
void display_grid(char **grid, int rows, int cols, char **words, int word_count, int *found, WordPosition *word_positions, int start_row, int start_col, int end_row, int end_col, int selecting, int blink, int score);
int check_word(const char *selected_word, char **words, int word_count, int *found);
int kbhit(void);
void start_game(const char *selected_theme, int grid_rows, int grid_cols);

#endif 