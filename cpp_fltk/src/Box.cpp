#include "../headers/Box.h"
#include <iostream>
#include <chrono>

using namespace std;

chrono::time_point<chrono::high_resolution_clock> start;
chrono::time_point<chrono::high_resolution_clock> endd;
long long check;

int sum;


KeyDisplay::KeyDisplay(const char* label) : Fl_Window(750, 700, label)  {
    box = new Fl_Box(25, 25, 700, 650, "A");
    box->labelsize(450);
    this->AllTimes = (int*) malloc(sizeof(int)*10000);
}


int KeyDisplay::handle(int event) {
    switch(event) {
        case FL_KEYDOWN:
	    cout << "Key pressed" << endl;
            start = chrono::high_resolution_clock::now();
            this->box->label(Fl::event_text());
            endd = chrono::high_resolution_clock::now();
            this->AllTimes[this->size] = chrono::duration_cast<chrono::nanoseconds>(endd-start).count();
            this->size++;
            return 1;
        case FL_PUSH:
            this->hide();
            for (int i = 0; i < this->size; i++){
                sum += this->AllTimes[i];
            }
            cout << "average speed in nanosecond " << sum/this->size << endl;
            return 1;
        default:
            return 0;
    }
}
