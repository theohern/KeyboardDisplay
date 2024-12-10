#include "../headers/Box.h"
#include <iostream>

using namespace std;


KeyDisplay::KeyDisplay(const char* label) : Fl_Window(750, 700, label)  {
    box = new Fl_Box(25, 25, 700, 650, "A");
    box->labelsize(450);
}


int KeyDisplay::handle(int event) {
    switch(event) {
        case FL_KEYDOWN:
            this->box->label(Fl::event_text());
            return 1;
        case FL_PUSH:
            this->hide();
            return 1;
        default:
            return 0;
    }
}