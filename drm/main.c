// Includes

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <pthread.h>
#include <time.h>

// Defines
#define nChars 26
#define bitmapSize 64
#define BUFFER_SIZE 20


typedef struct{
	char *data;
	int width;
	int height;
	int size;
	int fb;
} Frame;

typedef struct{
	Frame* frame;
	char** letters;
	struct timespec *ends;
	int size;
} ConsumerArgs;

typedef struct{
	struct timespec *starts;
	int size;
} ProducerArgs;

// Initializations

char keyboard[20] = "/dev/input/event2";
char screen[10] = "/dev/fb2";

int Xoffset = 200;
int Yoffset = 200;

char buffer[BUFFER_SIZE];
int in = 0;
int out = 0; 
int count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;

// Functions

/* 
 * LoadLetters
 * Load the letters on the Heap memory
 * No arguments
 * Return a tab of bitmap
*/
char** LoadLetters(){
	// Create a char ** to stores all the bitmaps	
	int fileSize = bitmapSize*bitmapSize*4;
	char **letters = malloc(nChars*sizeof(char*));
	for (int i = 0; i < nChars; i++){
		letters[i] = malloc(fileSize*sizeof(char));
	}

	// Open each file and copy the bitmap on the Heap
	char filename[20];
	FILE* file;
	int index = 0;
	for (char letter = 'A'; letter <= 'Z'; letter++){
	sprintf(filename, "../letters/bin64/%c.bin", letter);
		file = fopen(filename, "rb");
		if (!file) {
			fprintf(stderr, "Cannot open letters file\n");
			for (int i = 0; i < nChars; i++){
				free(letters[i]);
			}
			free(letters);
			return NULL;
		}

		fread(letters[index],sizeof(char),fileSize, file);
		index++;
	}

	// Return the Heap Address
	return letters;
}

/*
 * GetEventKeyboard
 * Add the keyboard event in a buffer
 * No arguments
 * Running in a Single Thread (producer)
 * No Return
*/
void* GetEventKeyboard(void* args){
	ProducerArgs* ProArgs = (ProducerArgs*) args;
	// Get the keyboard device and the input event
	const char *alphabet = 	"AZERTYUIOP0000QSDFGHJKLM0000WXCVBN";
	struct input_event ev;
	int device = open(keyboard, O_RDONLY);
	if (device == -1){
		fprintf(stderr, "Cannot open the keyboard\n");
		fprintf(stderr, "Quitting...\n");
		return NULL;
	}
	printf("Thread is running\n");

	// While loop to read on the keyboard
	// the Q letter quit the thread 
	while (1) {
		ssize_t n = read(device, &ev, sizeof(ev));
		if (n < (ssize_t)sizeof(ev)){
			fprintf(stderr, "Cannot read event\n");
			fprintf(stderr, "Quitting...\n");
			return NULL;
		}

		if (ev.type == EV_KEY && ev.value == 1 && ev.code >= 16 && ev.code <= 49 && alphabet[ev.code-16] != 48 ){
			clock_gettime(CLOCK_MONOTONIC, ProArgs->starts+(ProArgs->size));
			ProArgs->size++;
			int code = ev.code - 16;
			pthread_mutex_lock(&mutex);
			while ((in + 1) % BUFFER_SIZE == out){
				pthread_cond_wait(&cond_empty, &mutex);
			}
			buffer[in] = alphabet[code];
			in = (in + 1) % BUFFER_SIZE;
			count++;
			printf("Key pressed : %d letter %c and in : %d\n", code, alphabet[code]);
			pthread_cond_signal(&cond_full);
			pthread_mutex_unlock(&mutex);
			if (code == 14){
				printf("Thread quitting normally...\n");
				return NULL;
			}
		}
	}

	// close and return
	close(device);
	return NULL;
}

/*
 * SaveScreen
 * Save The pixelzs of the screen before playing with it
 * frame : Frame of the current screen
 * Return the backup of the frame
*/
char* SaveScreen(Frame* frame){
	char *backup = malloc(frame->size*sizeof(char));
	memcpy(backup, frame->data, frame->size);
	return backup;
}

/*
 * RestoreScreen
 * Put back the backup on the screen
 * frame : Frame of the current screen
 * backup : backup of the screen before playing with it
 * No Return
*/
void RestoreScreen(Frame* frame, char* backup){
	memcpy(frame->data, backup, frame->size);
	return;
}
	
/*
 * GetScreenFrame
 * Map the Frame to the memory (Heap)
 * No arguments
 * Return the frame address on the heap
*/
Frame* GetScreenFrame(){
	int fb = open(screen, O_RDWR);
	if (fb == -1){
		fprintf(stderr, "Cannot open the screen device\n");
		return NULL;
	}
	
	struct fb_var_screeninfo vinfo;
	if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo)) {
		fprintf(stderr, "Cannot read screen data\n");
		close(fb);
		return NULL;
	}
	
	Frame *frame = malloc(sizeof(Frame));
	if (frame == NULL){
		fprintf(stderr, "Cannot create a Frame structure\n");
		close(fb);
		return NULL;
	}

	frame->width = vinfo.xres;
	frame->height = vinfo.yres;
	printf("size of the frame : %d x %d\n", frame->width, frame->height);
	frame->size = frame->height * frame->width * 4;
	frame->fb = fb;
	frame->data = mmap(NULL, frame->size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
	if (frame->data == MAP_FAILED){
		fprintf(stderr, "Cannot map the frame in the memory\n");
		close(fb);
		return NULL;
	}
	return frame;
}

/*
 * FreeFrame
 * Free the memory corresponding to the frame
 * frame :  Frame of the current screen
 * No Return
*/
void FreeFrame(Frame *frame){
	munmap(frame->data, frame->size);
	close(frame->fb);
	free(frame);
}


/*
 * DisplayLetter
 * Display the letter on the screen
 * frame :  Frame of the current screen
 * letter : the letter to display
 * letters : tab of all letters
 * No Return
*/
void DisplayLetter(Frame *frame, char letter, char** letters){
	int index = letter - 65; // 'A' = 65
	for (int i = 0; i < bitmapSize; i++){
		for (int j = 0; j < bitmapSize; j++){
			int x = (frame->width/2) + j + Xoffset; // +225 to center
			int y = (frame->height/2) + i + Yoffset; // +500 to center
			if (x < frame->width && y < frame->height){
				int pos = (y * frame->width + x) * 4;
				int loc = (i * bitmapSize + j) * 4;
				frame->data[pos] = letters[index][loc];
				frame->data[pos + 1] = letters[index][loc + 1];
				frame->data[pos + 2] = letters[index][loc + 2];
				frame->data[pos + 3] = letters[index][loc + 3];
			}
		}
	}
}


/*
 * LoopDisplay
 * Loop on the buffer to display letters
 * args : letters and frame
 * Run in single a Thread (consumer)
 * No Return
*/
void* LoopDisplay(void* args){
	ConsumerArgs* ConsArgs = (ConsumerArgs*) args;
	char **l = ConsArgs->letters;
	char letter;
	while (1){
		pthread_mutex_lock(&mutex);
		while (in == out){
			pthread_cond_wait(&cond_full, &mutex);
		}
		letter = buffer[out];
		DisplayLetter(ConsArgs->frame, letter, ConsArgs->letters);
		ConsArgs->size++;
		clock_gettime(CLOCK_MONOTONIC, ConsArgs->ends+(ConsArgs->size));
		printf("Letter %c displayed\n", letter);
		out = (out + 1) % BUFFER_SIZE;
		pthread_cond_signal(&cond_empty);
		pthread_mutex_unlock(&mutex);
		printf("Int letter : %d, char letter : %c\n", letter, letter);
		if (letter == 81){
			printf("Thread Display is quitting...\n");
			return NULL;
		}
	}
	return NULL;
}

// Main function
int main(int argc, char* argv[]){	
	int cpu = -1;
	int opt;
	while ((opt = getopt(argc, argv, "c:k:x:y:s:")) != -1) {
		switch(opt) {
			case 'k' :
				memcpy(keyboard+16, optarg, 1*sizeof(char));
				printf("device for keyboard : %s\n", keyboard);
				break;
			case 'x' : 
				Xoffset = atoi(optarg);
				printf("Xoffset : %d\n", Xoffset);
				break;
			case 'y' : 
				Yoffset = atoi(optarg);
				printf("Yoffset : %d\n", Yoffset);
				break;
			case 's' :
				memcpy(screen+7, optarg, 1*sizeof(char));
				printf("device for screen : %s\n", screen);
				break;
			case 'c' :
				cpu = atoi(optarg);
				printf("Thread assign to processor %d\n", cpu);
				break;
			default :
				printf("Usage: %s -k <1-9> -x <xoffset> -y <yoffset> -c <0-3>\n", argv[0]);
				return 1;
		}
	}

	//Load Letters on the Heap
	char **letters = LoadLetters();
	if (letters == NULL){
		return -1;
	}
	
	//Map the frame on the screen
	Frame *frame = GetScreenFrame();
	char* backup = SaveScreen(frame);
	
	// Start both Thread
	//

	ProducerArgs* proargs = malloc(sizeof(ProducerArgs));
	proargs->starts = malloc(sizeof(struct timespec) * 10000);
	proargs->size = 0;
	pthread_t EventKeyboard;

	ConsumerArgs* args = malloc(sizeof(ConsumerArgs));
	args->letters = letters;
	args->frame = frame;
	args->ends = malloc(sizeof(struct timespec) * 10000);
	args->size = 0;
	pthread_t Display;

	if (cpu >= 0){
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(2,&cpuset);
		int a = pthread_setaffinity_np(Display, sizeof(cpu_set_t), &cpuset);
		int b = pthread_setaffinity_np(EventKeyboard, sizeof(cpu_set_t), &cpuset);
		if (a < 0 && b < 0){
			printf("impossible to set thread on special cpu\n");
		}
	}
	pthread_create(&Display, NULL, LoopDisplay, (void*) args);
	pthread_create(&EventKeyboard, NULL, GetEventKeyboard,(void*) proargs);

	pthread_join(EventKeyboard, NULL);
	pthread_join(Display, NULL);

	RestoreScreen(frame, backup);
	free(letters);
	FreeFrame(frame);
	
	if (args->size != proargs->size){
		printf("problem with the counter pro : %d and cons : %d\n", proargs->size, args->size);
	}
	printf("counter : %d\n", args->size);
	int sum = 0;
	long sec = 0;
	long long nan = 0;
	int count = 0;
	for (int i = 0; i < args->size; i++){
		sec = args->ends[i].tv_sec - proargs->starts[i].tv_sec;
		nan = args->ends[i].tv_nsec - proargs->starts[i].tv_nsec;
		if (nan < 0){
			--sec;
			nan  += 1000000000;
		}
		sum += nan;
		count++;
	}
	//printf("average speed %ld\n", sum/count);
	printf("Pogram is quitting normally\n");
	return 0;
}
