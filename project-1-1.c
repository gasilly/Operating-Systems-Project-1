#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#define BUFFER_SIZE 10

typedef struct{
	char value;
} item;

void *producer(int *in, int *out, item *buffer, FILE *fp);
void *consumer(int *in, int *out, item *buffer);
void readFile(item *next_produced, FILE *fp);

int main(){
	item buffer[BUFFER_SIZE];
	pthread_t tid;
	FILE *fp = fopen("usenix2019_v3.1.txt", "r");
	int in = 0;
	int out = 0;
	//make producers
	for(int i = 0; i < 5; i++){
		//pthread_create(&tid, NULL, producer, NULL);
	}
	//producer(&in, &out, buffer, fp);
	pthread_join(tid, NULL);	
	fclose(fp);
	return 0;
}

//possibly remove and simply put in the while loop in the producer function
void readFile(item *next_produced, FILE *fp){
	next_produced -> value = fgetc(fp);
}

//function for producers to place items in the buffer
void *producer(int *in, int *out, item *buffer, FILE *fp){
	item next_produced;
	while (true) {
		if(feof(fp)){
			break;		
		}
		readFile(&next_produced, fp);
		//printf("%c\n", next_produced.value);
		// produce an item in next_produced //
		//while(((*in + 1) % BUFFER_SIZE) == *out);
		buffer[*in] = next_produced;
		*in = (*in + 1) % BUFFER_SIZE;
	}
}
//function for consumers to take items from the buffer and print it to the screen
void *consumer(int *in, int *out, item *buffer){
	item next_consumed;
	/*
	while(true){
		while(*in == *out);
		next_consumed = buffer[out];
		out = (out + 1) % BUFFER_SIZE;
		 consume the next item in next_consumed by printing it to the screen and deleting it from the buffer
	}
	*/
}
