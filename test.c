#include "array.h"
#include <pthread.h>
#include <unistd.h>

#define NUM_PRODUCERS 5
#define ITEMS_PER_PRODUCER 10

#define NUM_CONSUMERS 5
#define MAX_TOTAL_ITEMS (NUM_PRODUCERS * ITEMS_PER_PRODUCER)

typedef struct{
    bool processed;
    bool produced;
    char* string;
    int consume_count;
    int producer_id;
    int sequence_num;
} test;

typedef struct{
    array *arr;
    int thread_id;
    int produced;
    int consumed;
} thread_data;


sem_t test_mutex;

test item_tracker[NUM_PRODUCERS][ITEMS_PER_PRODUCER];

void init_tracker(){
    sem_init(&test_mutex,0,1);
    for(int i = 0; i < NUM_PRODUCERS; i++){
        for(int j = 0; j < ITEMS_PER_PRODUCER; j++){
            item_tracker[i][j].producer_id = i;
            item_tracker[i][j].sequence_num = j;
            item_tracker[i][j].produced = false;
            item_tracker[i][j].processed = false;
            item_tracker[i][j].consume_count = 0;
            
        }
    }
}



void* produce(void* arg){
    thread_data *data = (thread_data*) arg;

    for(int i = 0; i < ITEMS_PER_PRODUCER; i++){
        char* item = (char*)malloc(50);
        sprintf(item, "P%d_S%d", data->thread_id, i);

        sem_wait(&test_mutex);
        item_tracker[data->thread_id][i].produced = true;
        sem_post(&test_mutex);

        while(array_put(data->arr, item) != 0){ //keep trying until successful
            usleep(100);
        }

        data->produced++;
    }

    return NULL;

}

void* consume(void* arg){

    thread_data *data = (thread_data*) arg;

    for(int i = 0; i < ITEMS_PER_PRODUCER; i++){
        char* item = NULL;

        while(array_get(data->arr, &item) != 0){
            usleep(1000);
        }

        if (item != NULL){
            int prod_id, seq_num;

            if(sscanf(item, "P%d_S%d", &prod_id, &seq_num)==2){
                sem_wait(&test_mutex);
                item_tracker[prod_id][seq_num].processed = true;
                item_tracker[prod_id][seq_num].consume_count++;
                sem_post(&test_mutex);

                data->consumed++;
            }
            free(item);
        }
    }
    return NULL;
}

int main(){
    printf("Testing multithreaded\n");

    array s;

    array_init(&s);
    init_tracker();

    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    thread_data producer_data[NUM_PRODUCERS];
    thread_data consumer_data[NUM_CONSUMERS];

    printf("Starting %d producers and %d consumers...\n", NUM_PRODUCERS, NUM_CONSUMERS);

    for(int i = 0; i < NUM_PRODUCERS; i++){
        producer_data[i].arr = &s;
        producer_data[i].thread_id = i;
        producer_data[i].produced = 0;
        producer_data[i].consumed = 0;
        pthread_create(&producers[i], NULL, produce, &producer_data[i]);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_data[i].arr = &s;
        consumer_data[i].thread_id = i;
        consumer_data[i].produced = 0;
        consumer_data[i].consumed = 0;
        pthread_create(&consumers[i], NULL, consume, &consumer_data[i]);
    }

    for(int i = 0; i < NUM_PRODUCERS; i++){
        pthread_join(producers[i], NULL);
    }
    for(int i = 0; i < NUM_CONSUMERS; i++){
        pthread_join(consumers[i], NULL);
    }

    printf("All threads complete\n");
//produced
    int total_produced = 0;
    for(int i = 0; i < NUM_PRODUCERS; i++){
        total_produced += producer_data[i].produced;
    }

    if(total_produced == MAX_TOTAL_ITEMS){
        printf("Total produced items true\n");
    }
    else{
        printf("Incorrect produced item total\n");
    }

//consumed
    int total_consumed = 0;
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        total_consumed += consumer_data[i].consumed;
    }


    if(total_consumed == MAX_TOTAL_ITEMS){
        printf("Total consumed items true\n");
    }
    else{
        printf("Incorrect consumed item total\n");
        printf("Consumed: %d while %d was produced\n", total_consumed, total_produced);
    }

//produced but not consumed
    int missing_items = 0;
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        for (int j = 0; j < ITEMS_PER_PRODUCER; j++) {
            if (item_tracker[i][j].produced && !item_tracker[i][j].processed) {
                missing_items++;
                printf(" Item P%d_S%d was produced but never consumed\n", i, j);
            }
        }
    }

//race condition
    int duplicate_items = 0;
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        for (int j = 0; j < ITEMS_PER_PRODUCER; j++) {
            if (item_tracker[i][j].consume_count > 1) {
                duplicate_items++;
                printf("RACE CONDITION: Item P%d_S%d consumed %d times\n", 
                       i, j, item_tracker[i][j].consume_count);
            }
        }
    }

//consumed but not produced
    int phantom_items = 0;
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        for (int j = 0; j < ITEMS_PER_PRODUCER; j++) {
            if (!item_tracker[i][j].produced && item_tracker[i][j].processed) {
                phantom_items++;
                printf("  ⚠ Item P%d_S%d was consumed but never produced!\n", i, j);
            }
        }
    }


    array_free(&s);
    sem_destroy(&test_mutex);
}