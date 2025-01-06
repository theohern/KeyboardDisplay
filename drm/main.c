// Includes
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
} ConsumerArgs;

// Initializations

char *keyboard = "/dev/input/event0";
char *screen = "/dev/fb0";

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
	sprintf(filename, "letters/bin64/%c.bin", letter);
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

		if (ev.type == EV_KEY && ev.value == 1){
			int code = ev.code - 16;
			if (code == 14){
				printf("Thread quitting normally...\n");
				return NULL;
			}
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
	printf("try to display letter\n");
	int index = letter - 65; // 'A' = 65
	for (int i = 0; i < bitmapSize; i++){
		for (int j = 0; j < bitmapSize; j++){
			int x = (frame->width/2) + j; // +225 to center
			int y = (frame->height/2) + i; // +500 to center
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
		printf("Letter %c displayed\n", letter);
		out = (out + 1) % BUFFER_SIZE;
		pthread_cond_signal(&cond_empty);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

// Main function
int main(int argc, char* argv[]){
	//Load Letters on the Heap
	char **letters = LoadLetters();
	if (letters == NULL){
		return -1;
	}
	
	//Map the frame on the screen
	Frame *frame = GetScreenFrame();
	char* backup = SaveScreen(frame);
	
	// Start both Thread
	pthread_t EventKeyboard;
	pthread_create(&EventKeyboard, NULL, GetEventKeyboard, NULL);

	ConsumerArgs* args = malloc(sizeof(ConsumerArgs));
	args->letters = letters;
	args->frame = frame;
	pthread_t Display;
	pthread_create(&Display, NULL, LoopDisplay, (void*) args);


	int a = 0;
	while (1){
		a = (a+1)%10;
	}
	RestoreScreen(frame, backup);
	free(letters);
	FreeFrame(frame);
	return 0;
}

