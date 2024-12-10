#ifndef BOX_H
#define BOX_H

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Window.H>

class KeyDisplay : public Fl_Window {
    Fl_Box *box;
    public:
        KeyDisplay(const char* label = 0);
        int handle(int event) override;
};

#endif

