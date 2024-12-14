#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH  64  // Largeur maximale d'un glyphe
#define HEIGHT 64  // Hauteur maximale d'un glyphe

// Fonction pour sauvegarder un tableau 64x64x4 dans un fichier .bin
void save_to_bin(const char *filename, unsigned char data[HEIGHT][WIDTH][4]) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        exit(1);
    }

    // Sauvegarder les données sous forme brute
    fwrite(data, sizeof(unsigned char), HEIGHT * WIDTH * 4, file);
    fclose(file);
}

// Convertir un glyphe en un tableau 64x64x4 (RGBA)
void convert_glyph_to_rgba(FT_Bitmap *bitmap, unsigned char output[HEIGHT][WIDTH][4]) {
    // Remplir par défaut avec des pixels noirs transparents
    memset(output, 0, HEIGHT * WIDTH * 4);

    // Copier les données du glyphe dans le tampon RGBA
    for (int y = 0; y < bitmap->rows && y < HEIGHT; y++) {
        for (int x = 0; x < bitmap->width && x < WIDTH; x++) {
            unsigned char intensity = bitmap->buffer[y * bitmap->width + x];

            // Remplir les canaux (R=G=B=intensité, Alpha=255)
            output[y][x][0] = intensity; // Rouge
            output[y][x][1] = intensity; // Vert
            output[y][x][2] = intensity; // Bleu
            output[y][x][3] = 255;       // Alpha
        }
    }
}

void render_and_save_character(FT_Face face, char character) {
    char filename[64];
    unsigned char rgba_buffer[HEIGHT][WIDTH][4];

    // Charger le glyphe pour le caractère
    if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
        fprintf(stderr, "Erreur : Impossible de charger le caractère '%c'.\n", character);
        exit(1);
    }

    // Convertir le glyphe en tableau RGBA
    convert_glyph_to_rgba(&face->glyph->bitmap, rgba_buffer);

    // Créer le nom de fichier pour le caractère
    snprintf(filename, sizeof(filename), "%c.bin", character);

    // Sauvegarder les données RGBA dans un fichier .bin
    save_to_bin(filename, rgba_buffer);
    printf("Caractère '%c' sauvegardé dans %s\n", character, filename);
}

int main() {
    FT_Library ft;
    FT_Face face;

    // Initialiser FreeType
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Erreur : Impossible d'initialiser FreeType.\n");
        return 1;
    }

    // Charger la police
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 0, &face)) {
        fprintf(stderr, "Erreur : Impossible de charger la police.\n");
        return 1;
    }

    // Définir la taille des caractères
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Parcourir les lettres de A à Z
    for (char c = 'A'; c <= 'Z'; c++) {
        render_and_save_character(face, c);
    }

    // Libérer les ressources
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return 0;
}

