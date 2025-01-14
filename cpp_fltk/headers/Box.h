#ifndef BOX_H
#define BOX_H

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Window.H>
#include <chrono>

using namespace std;

class KeyDisplay : public Fl_Window {
    Fl_Box *box;
    public:
        KeyDisplay(const char* label = 0);
        int size;
        int handle(int event) override;
        int *AllTimes;
        chrono::time_point<std::chrono::high_resolution_clock> *Starts;
        chrono::time_point<std::chrono::high_resolution_clock> *Ends;
};

#endif

