#include <iostream>
#include <fstream>
#include <linux/input.h>
#include <cstring>
#include <unistd.h>

using namespace std;

int main() {
    // Remplacez "event0" par le fichier correspondant à votre clavier.
    const char *device = "/dev/input/event0";  

    // Ouvrir le fichier d'événements.
    ifstream keyboard(device, ios::binary);
    if (!keyboard.is_open()) {
        cerr << "Erreur : impossible d'ouvrir le périphérique " << device << endl;
        return 1;
    }

    struct input_event ev;
    cout << "En attente d'événements clavier...\n";

    while (true) {
        // Lire un événement clavier.
        keyboard.read(reinterpret_cast<char*>(&ev), sizeof(ev));
        if (keyboard.gcount() < sizeof(ev)) {
            cerr << "Erreur de lecture d'événement.\n";
            break;
        }

        // Vérifier si l'événement est une pression de touche.
        if (ev.type == EV_KEY) { 
            if (ev.value == 1) {  // 1 = Touche appuyée
                cout << "Touche pressée : code = " << ev.code << endl;
            } else if (ev.value == 0) {  // 0 = Touche relâchée
                cout << "Touche relâchée : code = " << ev.code << endl;
            }
        }
    }

    keyboard.close();
    return 0;
}

