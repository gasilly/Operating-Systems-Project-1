#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

struct thread_args{
	FILE *fp;
	int search_count;
	pthread_mutex_t *lock;
};

#define MAX_UNIQUE_WORDS 10000

void* find_unique_words(void* _args);
int get_words(char *words[], struct thread_args *args);

char* FILES[] = {"dictionary1.txt", "dictionary2.txt", "dictionary3.txt", "dictionary4.txt"};
char* unique_words[9999];

int unique_word_count = 0;

int main(){
	pthread_t tid[4];
	pthread_mutex_t lock;
	pthread_mutex_init(&lock, NULL);
	FILE* fp[4];
	struct thread_args args[4];
	for(int i = 0; i < 4; i++){
		fp[i] = fopen(FILES[i], "r");
	        if(fp[i] == NULL){
			printf("Please provide all the required files\n");
			exit(1);
		}
		args[i].fp = fp[i];	
		args[i].lock = &lock;
		args[i].search_count = 0;
	}
	for(int i = 0; i < 4; i++){
		pthread_create(&tid[i], NULL, &find_unique_words, &args[i]);
	}
	for(int i = 0; i < 4; i++){
                pthread_join(tid[i], NULL);
        }
	for(int i = 0; i < 4; i++){
		fclose(fp[i]);
	}
	pthread_mutex_destroy(&lock);
	for(int i = 0; i < 4; i++){
		printf("thread %d searched through %d words\n", i + 1, args[i].search_count);
	}
	printf("The unique words found are: \n");
	for(int i = 0; i < unique_word_count; i++){
                printf("%s \n", unique_words[i]);
        }
	return 0;
}

void* find_unique_words(void* _args){
	int num = 0;
	int thread_search_count = 0;
	bool is_unique = false;
	struct thread_args *args;
	args = (struct thread_args *) _args;
	char *words[10];

	for(int i = 0; i < 10; i++){
		words[i] = (char *) malloc(999 * sizeof(char));
	}
	while(num = get_words(words, args)){
		for(int i = 0; i < num; i++){
			is_unique = true;
			//mutex lock here 
			pthread_mutex_lock(args->lock);
			for(int j = 0; j < unique_word_count; j++){
				if(strcmp(unique_words[j], words[i]) == 0){
					is_unique = false;
				        break;	
				}
				args->search_count += 1;
			}
			if(is_unique == true && unique_word_count < MAX_UNIQUE_WORDS){
				unique_words[unique_word_count] = strdup(words[i]);
				unique_word_count ++;
			}
			pthread_mutex_unlock(args->lock);
		}
	}
	for(int i = 0; i < 10; i++){
		free(words[i]);
	}	
}

int get_words(char *words[], struct thread_args *args){
	int word_count = 0;
	for(int i = 0; i < 10; i++){
		if((fgets(words[i], 999, args->fp)) != NULL){
			word_count ++;
			words[i][strcspn(words[i], "\n")] = '\0';
		}
		else{
			break;
		}
	}	
	return word_count;
}
