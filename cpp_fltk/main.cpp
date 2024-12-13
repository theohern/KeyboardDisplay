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


void* GetKey(void* args){
	const char *device =  "/dev/input/event0";

	struct input_event ev;
	ifstream keyboard(device, ios::binary);
	while(true){
		keyboard.read(reinterpret_cast<char*>(&ev), sizeof(ev));
		
		if (ev.type == EV_KEY && ev.value == 1){
			cout << "Key : " << ev.code << endl;
		}
	}
	return NULL;
}

int main(int argc, char **argv) {

    thread KeyListener = thread(GetKey,(void*)  NULL);
    KeyDisplay* key_display = new KeyDisplay("fenetre");

    key_display->end();
    key_display->show();

    return Fl::run();
}
