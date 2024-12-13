#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <linux/input.h>
#include <fstream>

using namespace std;

const int BUFFER_SIZE = 10;

queue<int> buffer;
mutex mutex_buffer;
condition_variable buffer_not_empty;

void* produce(void *arg){
	const char *device ="/dev/input/event0";
	ifstream keyboard(device, ios::binary);
	if (!keyboard.is_open()){
		cerr << "Error : cannot open device" << device << endl;
		return NULL;
	}

	struct input_event ev;

	while(true){
		keyboard.read(reinterpret_cast<char*>(&ev), sizeof(ev));
		if (keyboard.gcount() < sizeof(ev)) {
			cerr << "Error : cannot read event" << endl;
			return NULL;
		}

		if (ev.type == EV_KEY && ev.value == 1) {
			unique_lock<mutex> lock(mutex_buffer);
			buffer.push((int) ev.code);
			buffer_not_empty.notify_one();
		}
	}
	return NULL;
}

void* consume(void *arg){
	int item;
	while (true) {
		unique_lock<mutex> lock(mutex_buffer);
		buffer_not_empty.wait(lock, [] {return !buffer.empty();});

		item = buffer.front();
		buffer.pop();
		cout << "Key code : " << item << endl;
	}
	return NULL;
}

int main(int argv, char *argc[]){
	const int numProducers = 1;
	const int numConsumers = 3;

	thread producers[numProducers];
	for (int i = 0; i < numProducers; i++){
		producers[i] = thread(produce, (void*) NULL);
	}
	
	thread consumers[numConsumers];
	for (int i = 0; i < numConsumers; i++){
		consumers[i] = thread(consume, (void*) NULL);
	}
	
	for (int i = 0; i < numProducers; i++){
		producers[i].join();
	}

	for (int i = 0; i < numConsumers; i++){
		consumers[i].join();
	}

	return 0;
}


