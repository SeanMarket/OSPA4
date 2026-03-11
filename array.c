#include "array.h"

bool array_isFull(array *s, sem_t *mutex){
    sem_wait(mutex);
    if((s->back + 1) % ARR_SIZE == s->front){ //have to lock because of s->back + 1 comp?
        sem_post(mutex);
        return true;
    }
    sem_post(mutex);
    return false;
}

bool array_isEmpty(array *s, sem_t *mutex){
    sem_wait(mutex);
    if((s->front == -1) && (s->back == -1)){
        sem_post(mutex);
        return true;
    }
    sem_post(mutex);
    return false;
}

int array_init(array *s){
    s->front = -1;
    s->back = -1;
    for (int i = 0; i < ARR_SIZE; i++){
        s->arr[i] = NULL;
    }
    return 0;
}

int array_put(array *s, char *hostname, sem_t *mutex){
    if(array_isFull(s, mutex)){
        printf("Cannot put. Array is full\n");
        return -1;
    }
    else if(array_isEmpty(s, mutex)){
        sem_wait(mutex);
        s->front++;
    }

    s->back = (s->back + 1) % ARR_SIZE;
    s->arr[s->back] = hostname;
    sem_post(mutex);
    return 0;
}

int array_get(array *s, char **hostname, sem_t *mutex){
    if(array_isEmpty(s, mutex)){
        printf("Cannot get. Array is empty\n");
        return -1;
    }

    sem_wait(mutex);

    *hostname = s->arr[s->front];

    if(s->front == s->back){
        s->front = s->back = -1;
    }
    else{
        s->front = (s->front + 1) % ARR_SIZE;
    }
    sem_post(mutex);
    return 0;
}

void array_free(array *s){
    for(int i = 0; i < ARR_SIZE; i++){
        s->arr[i] = NULL;
    }
    s->front = -1;
    s->back = -1;
}