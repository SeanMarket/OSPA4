#include "array.h"

bool array_isFull(array *s){
    if(s->back + 1 % ARR_SIZE == s->front){
        return true;
    }
    return false;
}

bool array_isEmpty(array *s){
    if((s->front == -1) && (s->back == -1)){
        return true;
    }
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

int array_put(array *s, char *hostname){
    if(array_isFull(s)){
        printf("Cannot put. Array is full\n");
        return -1;
    }
    else if(array_isEmpty(s)){
        s->front++;
    }

    s->back = (s->back + 1) % ARR_SIZE;
    s->arr[s->back] = hostname;
}

int array_get(array *s, char **hostname){
    if(array_isEmpty(s)){
        printf("Cannot get. Array is empty\n");
        return -1;
    }

    hostname = s->arr[s->front];

    if(s->front == s->back){
        s->front = s->back = -1;
    }
    else{
        s->front = (s->front + 1) % ARR_SIZE;
    }

    return 0;
}

void array_free(array *s){
    for(int i = 0; i < ARR_SIZE; i++){
        s->arr[i] = NULL;
    }
    s->front == -1;
    s->back == -1;
}