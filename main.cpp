#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/names.h>
#include "headers/Box.h"

using namespace std;


int main(int argc, char **argv) {
    KeyDisplay* key_display = new KeyDisplay("fenetre");

    key_display->end();
    key_display->show();

    return Fl::run();
}