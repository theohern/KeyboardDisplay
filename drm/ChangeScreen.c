#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <errno.h>

#define WIDTH  1920 // Largeur de l'écran
#define HEIGHT 1080 // Hauteur de l'écran

// Fonction pour afficher un carré rouge de 32x32
void afficher_carre_rouge(char *framebuffer, int largeur, int hauteur, int offset_x, int offset_y) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            int x = offset_x + j;
            int y = offset_y + i;
            if (x < largeur && y < hauteur) {
                int pos = (y * largeur + x) * 4; // 4 octets par pixel pour RGBA

                // Colorier le pixel en rouge (R=255, G=0, B=0, A=255)
                framebuffer[pos] = 0x00;     // Bleu
                framebuffer[pos + 1] = 0x00; // Vert
                framebuffer[pos + 2] = 0xFF; // Rouge
                framebuffer[pos + 3] = 0xFF; // Alpha (opaque)
            }
        }
    }
}

// Fonction pour sauvegarder l'état initial de l'écran
void sauvegarder_ecran(char *framebuffer, char *backup, int largeur, int hauteur) {
    memcpy(backup, framebuffer, largeur * hauteur * 4); // Sauvegarder les pixels dans le tampon
}

// Fonction pour restaurer l'état initial de l'écran
void restaurer_ecran(char *framebuffer, char *backup, int largeur, int hauteur) {
    memcpy(framebuffer, backup, largeur * hauteur * 4); // Restaurer les pixels depuis le tampon
}

int main() {
    int fb = open("/dev/fb0", O_RDWR);
    if (fb == -1) {
        perror("Impossible d'ouvrir le framebuffer");
        return 1;
    }

    struct fb_var_screeninfo vinfo;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Erreur de récupération des informations sur l'écran");
        close(fb);
        return 1;
    }

    int largeur = vinfo.xres;
    int hauteur = vinfo.yres;

    char *framebuffer = mmap(NULL, largeur * hauteur * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (framebuffer == MAP_FAILED) {
        perror("Impossible de mapper la mémoire du framebuffer");
        close(fb);
        return 1;
    }

    // Sauvegarder l'écran avant de dessiner quoi que ce soit
    char *backup = malloc(largeur * hauteur * 4);
    if (backup == NULL) {
        perror("Erreur d'allocation mémoire pour sauvegarde");
        munmap(framebuffer, largeur * hauteur * 4);
        close(fb);
        return 1;
    }

    sauvegarder_ecran(framebuffer, backup, largeur, hauteur);

    // Calculer les coordonnées pour centrer le carré rouge 32x32
    int offset_x = (largeur - 32) / 2;  // Centrer horizontalement
    int offset_y = (hauteur - 32) / 2 + 100;  // Descendre le carré de 100 pixels vers le bas

    // Afficher le carré rouge
    afficher_carre_rouge(framebuffer, largeur, hauteur, offset_x, offset_y);
    sleep(1); // Attendre une seconde pour que l'utilisateur voie l'affichage

    // Restaurer l'écran à son état initial
    restaurer_ecran(framebuffer, backup, largeur, hauteur);

    // Libérer la mémoire tampon
    free(backup);

    // Dé-mapper le framebuffer et fermer
    munmap(framebuffer, largeur * hauteur * 4);
    close(fb);

    return 0;
}

