#include "../headers/Box.h"
#include <iostream>
#include <chrono>

using namespace std;

chrono::time_point<chrono::high_resolution_clock> start;
chrono::time_point<chrono::high_resolution_clock> endd;
long long check;

int sum;

chrono::time_point<std::chrono::high_resolution_clock>* Ends = (chrono::time_point<std::chrono::high_resolution_clock>*) malloc(sizeof(chrono::time_point<std::chrono::high_resolution_clock>) * 10000);

KeyDisplay::KeyDisplay(const char* label) : Fl_Window(750, 700, label)  {
    box = new Fl_Box(25, 25, 700, 650, "A");
    box->labelsize(450);
    this->AllTimes = (int*) malloc(sizeof(int)*10000);
    this->Starts = (chrono::time_point<std::chrono::high_resolution_clock>*) malloc(sizeof(chrono::time_point<std::chrono::high_resolution_clock>) * 10000);
    this->Ends = (chrono::time_point<std::chrono::high_resolution_clock>*) malloc(sizeof(chrono::time_point<std::chrono::high_resolution_clock>) * 10000);
    
    
}


int KeyDisplay::handle(int event) {
    switch(event) {
        case FL_KEYDOWN:
	    cout << "Key pressed" << endl;
            start = chrono::high_resolution_clock::now();
	    this->Starts[this->size] = start;
            this->box->label(Fl::event_text());
            endd = chrono::high_resolution_clock::now();
	    Ends[this->size] = endd;
            this->AllTimes[this->size] = chrono::duration_cast<chrono::nanoseconds>(endd-start).count();
            this->size++;
            return 1;
        case FL_PUSH:
            this->hide();
            for (int i = 0; i < this->size; i++){
                sum += this->AllTimes[i];
            }
            cout << "average speed to display " << sum/this->size << endl;
            return 1;
        default:
            return 0;
    }
}
