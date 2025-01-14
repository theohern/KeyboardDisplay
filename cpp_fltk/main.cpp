#include <iostream>
#include <thread>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/names.h>
#include "headers/Box.h"
#include <linux/input.h>
#include <fstream>

using namespace std;

typedef struct{
	chrono::time_point<std::chrono::high_resolution_clock>* starts;
	int* size;
} ThreadArgs;



void* GetKey(void* args){
	chrono::time_point<chrono::high_resolution_clock> start;
	ThreadArgs* arg = (ThreadArgs*) args;
	const char *device =  "/dev/input/event1";
	cout << "size in Thread " << *(arg->size) << endl;

	struct input_event ev;
	ifstream keyboard(device, ios::binary);
	while(true){
		keyboard.read(reinterpret_cast<char*>(&ev), sizeof(ev));
		if (ev.type == EV_KEY && ev.value == 1){
			start = chrono::high_resolution_clock::now();
			arg->starts[*(arg->size)] = start;
			*(arg->size) = *(arg->size) + 1;
			if (ev.code == 30){
				cout << "Thread is quitting with number " << *(arg->size) << endl;
				return NULL;
			}
			cout << "Key : " << ev.code << endl;
		}
	}
	return NULL;
}

int main(int argc, char **argv) {
    chrono::time_point<std::chrono::high_resolution_clock> *Thread_Starts = (chrono::time_point<std::chrono::high_resolution_clock>*) malloc(sizeof(chrono::time_point<std::chrono::high_resolution_clock>) * 10000);
    int ThreadSize = 0;
    ThreadArgs* args = (ThreadArgs*) malloc(sizeof(ThreadArgs));
    args->starts = Thread_Starts;
    args->size = &ThreadSize;

    thread KeyListener = thread(GetKey,(void*)  args);
    KeyDisplay* key_display = new KeyDisplay("fenetre");
    key_display->end();
    key_display->show();

    Fl::run();

    int *total = (int*) malloc(sizeof(int*)*10000);
    int sizetot = 0;
    long long int sum = 0;
    for (int i = 0; i < *(args->size); i++){
	    total[i] = chrono::duration_cast<chrono::nanoseconds>(key_display->Ends[i] - args->starts[i]).count();
	    sum += total[i];
	    sizetot++;
    }

    cout << "average speed in general " << sum/sizetot << endl;
}
