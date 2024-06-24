#include <termios.h>
#include <unistd.h>
#include "../include/termios_util.h"

// Fonction pour activer ou désactiver le mode brut
void set_raw_mode(int enable) {
    static struct termios oldt, newt;
    if (enable) {
        // Sauvegarder les paramètres actuels du terminal
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        // Désactiver le mode canonique et l'écho
        newt.c_lflag &= ~(ICANON | ECHO);
        // Appliquer les nouveaux paramètres immédiatement
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        // Restaurer les paramètres précédents du terminal
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}
