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
#define SQUARE_SIZE 64 // Taille du carré rouge
#define BUFFER_SIZE 20


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <pthread.h>

char buffer[BUFFER_SIZE];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;

void* getKey(void *arg) {
    const char *alphabet = "AZERTYUIOP0000QSDFGHJKLM0000WXCVBN";
    const char *device = "/dev/input/event0";
    int keyboard = open(device, O_RDONLY);
    if (keyboard == -1) {
        perror("Error: cannot open device");
        return NULL;
    }

    struct input_event ev;

    while (1) {
        ssize_t n = read(keyboard, &ev, sizeof(ev));
        if (n < (ssize_t)sizeof(ev)) {
            perror("Error: cannot read event");
            close(keyboard);
            return NULL;
        }

        if (ev.type == EV_KEY && ev.value == 1) {
	    int code = ev.code - 16;
	    pthread_mutex_lock(&mutex);
	    while (count == BUFFER_SIZE){
		    pthread_cond_wait(&cond_full, &mutex);
	    }
	    buffer[in] = alphabet[code];
	    in = (in + 1) % BUFFER_SIZE;
	    count ++;
            printf("Key pressed : %d and letter %c\n", ev.code, alphabet[code]);
	    pthread_cond_signal(&cond_empty);
	    pthread_mutex_unlock(&mutex);
        }
    }

    close(keyboard);
    return NULL;
}



void load_bin(char *filename, char *bitmap) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier glyphe");
        exit(1);
    }

    // Lire le contenu du bitmap
    fread(bitmap, sizeof(char), 64*64*4, file);
    fclose(file);
}


void afficher_lettre_A(char *framebuffer, char *bitmap, int largeur, int hauteur, int offset_x, int offset_y) {
    for (int i = 0; i < SQUARE_SIZE; i++) {
        for (int j = 0; j < SQUARE_SIZE; j++) {
            int x = offset_x + j;
            int y = offset_y + i;
            if (x < largeur && y < hauteur) {
                int pos = (y * largeur + x) * 4; // 4 octets par pixel pour RGBA
		int loc = (i*64 +j)*4;
                // Colorier le pixel en rouge (R=255, G=0, B=0, A=255)
                framebuffer[pos] = bitmap[loc];     // Bleu
                framebuffer[pos + 1] = bitmap[loc + 1]; // Vert
                framebuffer[pos + 2] = bitmap[loc + 2]; // Rouge
                framebuffer[pos + 3] = bitmap[loc + 3]; // Alpha (opaque)
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

// Fonction pour initialiser le framebuffer
int initialiser_framebuffer(const char *device, char **framebuffer, int *largeur, int *hauteur) {
    int fb = open(device, O_RDWR);
    if (fb == -1) {
        perror("Impossible d'ouvrir le framebuffer");
        return -1;
    }

    struct fb_var_screeninfo vinfo;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Erreur de récupération des informations sur l'écran");
        close(fb);
        return -1;
    }

    *largeur = vinfo.xres;
    *hauteur = vinfo.yres;

    *framebuffer = mmap(NULL, (*largeur) * (*hauteur) * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (*framebuffer == MAP_FAILED) {
        perror("Impossible de mapper la mémoire du framebuffer");
        close(fb);
        return -1;
    }

    return fb;
}

// Fonction pour libérer le framebuffer
void liberer_framebuffer(int fb, char *framebuffer, int largeur, int hauteur) {
    munmap(framebuffer, largeur * hauteur * 4);
    close(fb);
}

int main() {
    // Thread for the keyboard event
    pthread_t ThreadOnKey;
    pthread_create(&ThreadOnKey, NULL, getKey, NULL);


    // Init for the framebuffer
    char *framebuffer;
    int largeur, hauteur;

    int fb = initialiser_framebuffer("/dev/fb0", &framebuffer, &largeur, &hauteur);
    if (fb == -1) {
        return 1;
    }

    // Backup for the screen
    char *backup = malloc(largeur * hauteur * 4);
    if (backup == NULL) {
        perror("Erreur d'allocation mémoire pour sauvegarde");
        liberer_framebuffer(fb, framebuffer, largeur, hauteur);
        return 1;
    }

    sauvegarder_ecran(framebuffer, backup, largeur, hauteur);
    
    // Load the letters on the Heap
    const char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *bin_file = (char *) malloc(20*sizeof(char));
    char **bitmap = (char**) malloc(26*sizeof(char*));
    for (int i = 0; i < 26; i++){
    	sprintf(bin_file, "letters/bin64/%c.bin", alphabet[i]);
	bitmap[i] = malloc(64*64*4*sizeof(char));
	load_bin(bin_file, bitmap[i]);
    }


    // Offset for the display
    int offset_x = (largeur - SQUARE_SIZE) / 2;  // Centrer horizontalement
    int offset_y = (hauteur - SQUARE_SIZE) / 2 + 100;  // Descendre le carré de 100 pixels vers le bas

    // Load Letter A --> TODO
    afficher_lettre_A(framebuffer, bitmap[10], largeur, hauteur, offset_x, offset_y);
    sleep(10); // Attendre une seconde pour que l'utilisateur voie l'affichage

    // Restore the screen
    restaurer_ecran(framebuffer, backup, largeur, hauteur);

    // free memory
    free(backup);
    free(bitmap);

    // free the framebuffer
    liberer_framebuffer(fb, framebuffer, largeur, hauteur);

    return 0;
}
