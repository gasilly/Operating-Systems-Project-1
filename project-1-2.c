#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

// Structure definitions
typedef struct {
	char value;
} item;

struct thread_args {
	FILE *fp;
	int *trips;
	int current_buffer_size;
	pthread_mutex_t *lock1;
	pthread_mutex_t *lock2;
};

// Globals
int BUFFER_SIZE[] = {2, 3, 5, 15, 20, 25, 30};
item *buffer;
int in = 0;
int out = 0;
int count = 0;
bool doneProducing = false;
bool file_has_printed = false;
void* producer(void* _args);
void* consumer(void* _args);

int main() {
	int trips = 0;
	FILE *fp = fopen("usenix2019_v3.1.txt", "r");
	if (fp == NULL) {
		printf("The file is invalid\n");
		exit(1);
	}

	pthread_t tid[10];
	pthread_mutex_t lock1, lock2;
	pthread_mutex_init(&lock1, NULL);
	pthread_mutex_init(&lock2, NULL);

	// Structure setup for multiple arguments in thread call
	struct thread_args args[10];
	for (int i = 0; i < 10; i++) {
		args[i].fp = fp;
		args[i].trips = &trips;
		args[i].lock1 = &lock1;
		args[i].lock2 = &lock2;
	}

	for (int i = 0; i < 7; i++) {
		buffer = (item *) malloc(BUFFER_SIZE[i] * sizeof(item));
		if (buffer == NULL) {
			perror("Failed to allocate memory for buffer");
			exit(1);
		}
		for (int j = 0; j < 10; j++) {
			args[j].current_buffer_size = BUFFER_SIZE[i];
		}
		// Create producers and consumers
		for (int j = 0; j < 10; j++) {
			if ((j % 2) != 0) {
				pthread_create(&tid[j], NULL, producer, &args[j]);
    			} 	
			else {
				pthread_create(&tid[j], NULL, consumer, &args[j]);
    			}
		}
		// Wait for threads to finish
		for (int j = 0; j < 10; j++) {
    			pthread_join(tid[j], NULL);
		}
		printf("\nNumber of trips for buffer size %d: %d\n", BUFFER_SIZE[i], trips);
		file_has_printed = true;
		// Free memory for the buffer after each iteration
		free(buffer);
		// Reset state for next buffer size
		rewind(fp);
		doneProducing = false;
		in = 0;
		out = 0;
		count = 0;
		trips = 0;
	}	
	pthread_mutex_destroy(&lock1);
	pthread_mutex_destroy(&lock2);
	fclose(fp);
	return 0;
}

// Function for producers to place items in the buffer
void* producer(void* _args) {
	item next_produced;
	struct thread_args *args = (struct thread_args *)_args;
	while (true) {
	if (feof(args->fp)) {
		doneProducing = true;
		break;
	}
	pthread_mutex_lock(args->lock1);
	next_produced.value = fgetc(args->fp);
	if(count != args->current_buffer_size){
		*(args->trips) = *(args->trips) + 1;
	}
	while ((count == args->current_buffer_size) && (doneProducing == false));  // Wait for space in buffer
	buffer[in].value = next_produced.value;
	in = (in + 1) % args->current_buffer_size;
	count += 1;
	pthread_mutex_unlock(args->lock1);
	}
	return NULL;
}

// Function for consumers to take items from the buffer and print it to the screen
void* consumer(void* _args) {
	item next_consumed;
	struct thread_args *args = (struct thread_args *)_args;
	while (true) { 
		pthread_mutex_lock(args->lock2);
		while ((count == 0) && (doneProducing == false));
 		if (count == 0 && doneProducing) {
			pthread_mutex_unlock(args->lock2);  // Exit when done producing and buffer is empty
			break;
		}	
		next_consumed = buffer[out];
		if(file_has_printed == false){
			printf("%c", next_consumed.value);
		}	
		buffer[out].value = '\0';
		out = (out + 1) % args->current_buffer_size;
		count -= 1;
		pthread_mutex_unlock(args->lock2);
	}
	return NULL;
}

