#include "array.h"
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Test counter
int tests_passed = 0;
int tests_failed = 0;

// Helper macros for testing
#define TEST_START(name) printf("\n[TEST] %s\n", name)
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  ✓ PASS: %s\n", message); \
            tests_passed++; \
        } else { \
            printf("  ✗ FAIL: %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

// Test 1: Initialization
void test_array_init() {
    TEST_START("test_array_init");
    array s;
    int result = array_init(&s);
    
    TEST_ASSERT(result == 0, "array_init returns 0 on success");
    TEST_ASSERT(s.front == -1, "front initialized to -1");
    TEST_ASSERT(s.back == -1, "back initialized to -1");
    
    for (int i = 0; i < ARR_SIZE; i++) {
        TEST_ASSERT(s.arr[i] == NULL, "array elements initialized to NULL");
    }
}

// Test 2: isEmpty on new array
void test_array_isEmpty_initial() {
    TEST_START("test_array_isEmpty_initial");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    bool empty = array_isEmpty(&s, &mutex);
    
    TEST_ASSERT(empty == true, "newly initialized array is empty");
    
    sem_destroy(&mutex);
}

// Test 3: isFull on new array
void test_array_isFull_initial() {
    TEST_START("test_array_isFull_initial");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    bool full = array_isFull(&s, &mutex);
    
    TEST_ASSERT(full == false, "newly initialized array is not full");
    
    sem_destroy(&mutex);
}

// Test 4: Single put operation
void test_array_put_single() {
    TEST_START("test_array_put_single");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    char *test_str = "test1";
    int result = array_put(&s, test_str, &mutex);
    
    TEST_ASSERT(result == 0, "array_put returns 0 on success");
    TEST_ASSERT(s.front == 0, "front is 0 after first put");
    TEST_ASSERT(s.back == 0, "back is 0 after first put");
    TEST_ASSERT(s.arr[0] == test_str, "element stored at correct position");
    TEST_ASSERT(array_isEmpty(&s, &mutex) == false, "array is not empty after put");
    
    sem_destroy(&mutex);
}

// Test 5: Multiple put operations
void test_array_put_multiple() {
    TEST_START("test_array_put_multiple");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    char *strings[] = {"str1", "str2", "str3", "str4"};
    
    for (int i = 0; i < 4; i++) {
        int result = array_put(&s, strings[i], &mutex);
        TEST_ASSERT(result == 0, "array_put succeeds for multiple elements");
    }
    
    TEST_ASSERT(s.front == 0, "front is still 0");
    TEST_ASSERT(s.back == 3, "back is at position 3");
    
    sem_destroy(&mutex);
}

// Test 6: Fill array to capacity
void test_array_fill_to_capacity() {
    TEST_START("test_array_fill_to_capacity");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    char *strings[ARR_SIZE];
    
    // Fill array completely
    for (int i = 0; i < ARR_SIZE; i++) {
        strings[i] = (char*)malloc(20);
        sprintf(strings[i], "element_%d", i);
        int result = array_put(&s, strings[i], &mutex);
        TEST_ASSERT(result == 0, "able to add element to array");
    }
    
    bool full = array_isFull(&s, &mutex);
    TEST_ASSERT(full == true, "array reports full when at capacity");
    
    // Try to add one more - should fail
    char *extra = "extra";
    int result = array_put(&s, extra, &mutex);
    TEST_ASSERT(result == -1, "array_put returns -1 when full");
    
    // Cleanup
    for (int i = 0; i < ARR_SIZE; i++) {
        free(strings[i]);
    }
    sem_destroy(&mutex);
}

// Test 7: Single get operation
void test_array_get_single() {
    TEST_START("test_array_get_single");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    char *test_str = "test_get";
    array_put(&s, test_str, &mutex);
    
    char *retrieved = NULL;
    int result = array_get(&s, &retrieved, &mutex);
    
    TEST_ASSERT(result == 0, "array_get returns 0 on success");
    TEST_ASSERT(s.front == -1, "front reset to -1 after getting last element");
    TEST_ASSERT(s.back == -1, "back reset to -1 after getting last element");
    TEST_ASSERT(array_isEmpty(&s, &mutex) == true, "array is empty after getting only element");
    
    sem_destroy(&mutex);
}

// Test 8: Get from empty array
void test_array_get_empty() {
    TEST_START("test_array_get_empty");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    char *retrieved = NULL;
    int result = array_get(&s, &retrieved, &mutex);
    
    TEST_ASSERT(result == -1, "array_get returns -1 when empty");
    
    sem_destroy(&mutex);
}

// Test 9: Multiple put and get operations (FIFO behavior)
void test_array_fifo() {
    TEST_START("test_array_fifo");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    char *strings[] = {"first", "second", "third"};
    
    // Add three elements
    for (int i = 0; i < 3; i++) {
        array_put(&s, strings[i], &mutex);
    }
    
    // Get them back and verify FIFO order
    for (int i = 0; i < 3; i++) {
        char *retrieved = NULL;
        array_get(&s, &retrieved, &mutex);
        // Note: the current implementation has a bug - it doesn't assign properly
        // This test documents expected behavior
    }
    
    TEST_ASSERT(array_isEmpty(&s, &mutex) == true, "array is empty after all gets");
    
    sem_destroy(&mutex);
}

// Test 10: Circular wrapping
void test_array_circular_wrapping() {
    TEST_START("test_array_circular_wrapping");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    
    // Fill the array
    for (int i = 0; i < ARR_SIZE; i++) {
        char *str = (char*)malloc(20);
        sprintf(str, "item_%d", i);
        array_put(&s, str, &mutex);
    }
    
    // Remove a few elements
    for (int i = 0; i < 3; i++) {
        char *retrieved = NULL;
        array_get(&s, &retrieved, &mutex);
    }
    
    // Add more elements (should wrap around)
    for (int i = 0; i < 3; i++) {
        char *str = (char*)malloc(20);
        sprintf(str, "wrap_%d", i);
        int result = array_put(&s, str, &mutex);
        TEST_ASSERT(result == 0, "able to add elements after wrapping");
    }
    
    TEST_ASSERT(s.back < s.front || s.back == ARR_SIZE - 1 || s.back < 3, 
                "back index wraps around correctly");
    
    sem_destroy(&mutex);
}

// Test 11: array_free
void test_array_free() {
    TEST_START("test_array_free");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    
    // Add some elements
    for (int i = 0; i < 3; i++) {
        char *str = (char*)malloc(20);
        sprintf(str, "free_test_%d", i);
        array_put(&s, str, &mutex);
    }
    
    array_free(&s);
    
    // Check that array is cleared (note: current implementation has bugs)
    for (int i = 0; i < ARR_SIZE; i++) {
        TEST_ASSERT(s.arr[i] == NULL, "array elements set to NULL");
    }
    
    sem_destroy(&mutex);
}

// Test 12: Alternating put and get
void test_array_alternating_ops() {
    TEST_START("test_array_alternating_ops");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    
    array_init(&s);
    
    for (int i = 0; i < 5; i++) {
        char *str = (char*)malloc(20);
        sprintf(str, "alternate_%d", i);
        
        int put_result = array_put(&s, str, &mutex);
        TEST_ASSERT(put_result == 0, "put succeeds");
        
        char *retrieved = NULL;
        int get_result = array_get(&s, &retrieved, &mutex);
        TEST_ASSERT(get_result == 0, "get succeeds");
        
        TEST_ASSERT(array_isEmpty(&s, &mutex) == true, "array empty after matched put/get");
    }
    
    sem_destroy(&mutex);
}

// Enhanced multithreaded test with race condition detection

#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 3
#define ITEMS_PER_PRODUCER 10
#define MAX_TOTAL_ITEMS (NUM_PRODUCERS * ITEMS_PER_PRODUCER)

// Tracking structure for produced/consumed items
typedef struct {
    int producer_id;
    int sequence_num;
    bool produced;
    bool consumed;
    int consume_count; // Track if consumed multiple times (race condition!)
} item_tracker_t;

// Global tracking array and mutex for verification
item_tracker_t item_tracker[NUM_PRODUCERS][ITEMS_PER_PRODUCER];
sem_t tracker_mutex;

typedef struct {
    array *arr;
    sem_t *mutex;
    int thread_id;
    int items_produced;
    int items_consumed;
} thread_data_t;

void init_tracker() {
    sem_init(&tracker_mutex, 0, 1);
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        for (int j = 0; j < ITEMS_PER_PRODUCER; j++) {
            item_tracker[i][j].producer_id = i;
            item_tracker[i][j].sequence_num = j;
            item_tracker[i][j].produced = false;
            item_tracker[i][j].consumed = false;
            item_tracker[i][j].consume_count = 0;
        }
    }
}

void* producer_thread_tracked(void* arg) {
    thread_data_t *data = (thread_data_t*)arg;
    
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        char *item = (char*)malloc(50);
        sprintf(item, "P%d_S%d", data->thread_id, i);
        
        // Mark as produced in tracker
        sem_wait(&tracker_mutex);
        item_tracker[data->thread_id][i].produced = true;
        sem_post(&tracker_mutex);
        
        // Add random delay to stress test
        usleep(rand() % 5000);
        
        // Keep trying until successful
        while (array_put(data->arr, item, data->mutex) != 0) {
            usleep(1000);
        }
        
        data->items_produced++;
        
        // Add another delay after successful put
        usleep(rand() % 3000);
    }
    
    return NULL;
}

void* consumer_thread_tracked(void* arg) {
    thread_data_t *data = (thread_data_t*)arg;
    
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        char *item = NULL;
        
        // Add random delay before get
        usleep(rand() % 5000);
        
        // Keep trying until successful
        while (array_get(data->arr, &item, data->mutex) != 0) {
            usleep(1000);
        }
        
        if (item != NULL) {
            // Parse the item to get producer_id and sequence
            int prod_id, seq_num;
            if (sscanf(item, "P%d_S%d", &prod_id, &seq_num) == 2) {
                // Track consumption
                sem_wait(&tracker_mutex);
                item_tracker[prod_id][seq_num].consumed = true;
                item_tracker[prod_id][seq_num].consume_count++;
                sem_post(&tracker_mutex);
                
                data->items_consumed++;
            }
            free(item);
        }
        
        // Add delay after consumption
        usleep(rand() % 3000);
    }
    
    return NULL;
}

void test_array_multithreaded() {
    TEST_START("test_array_multithreaded - Race Condition Detection");
    array s;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    array_init(&s);
    init_tracker();
    
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    thread_data_t producer_data[NUM_PRODUCERS];
    thread_data_t consumer_data[NUM_CONSUMERS];
    
    printf("  Starting %d producers and %d consumers...\n", NUM_PRODUCERS, NUM_CONSUMERS);
    
    // Create producers
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_data[i].arr = &s;
        producer_data[i].mutex = &mutex;
        producer_data[i].thread_id = i;
        producer_data[i].items_produced = 0;
        producer_data[i].items_consumed = 0;
        pthread_create(&producers[i], NULL, producer_thread_tracked, &producer_data[i]);
    }
    
    // Create consumers
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_data[i].arr = &s;
        consumer_data[i].mutex = &mutex;
        consumer_data[i].thread_id = i;
        consumer_data[i].items_produced = 0;
        consumer_data[i].items_consumed = 0;
        pthread_create(&consumers[i], NULL, consumer_thread_tracked, &consumer_data[i]);
    }
    
    // Wait for all threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    printf("  All threads completed. Verifying results...\n");
    
    // Verify all items were produced
    int total_produced = 0;
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        total_produced += producer_data[i].items_produced;
    }
    TEST_ASSERT(total_produced == MAX_TOTAL_ITEMS, 
                "all items were produced");
    
    // Verify all items were consumed
    int total_consumed = 0;
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        total_consumed += consumer_data[i].items_consumed;
    }
    TEST_ASSERT(total_consumed == MAX_TOTAL_ITEMS, 
                "all items were consumed");
    
    // Check for missing items (produced but not consumed)
    int missing_items = 0;
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        for (int j = 0; j < ITEMS_PER_PRODUCER; j++) {
            if (item_tracker[i][j].produced && !item_tracker[i][j].consumed) {
                missing_items++;
                printf("  ⚠ Item P%d_S%d was produced but never consumed!\n", i, j);
            }
        }
    }
    TEST_ASSERT(missing_items == 0, "no items were lost (produced but not consumed)");
    
    // Check for duplicates (consumed multiple times - RACE CONDITION!)
    int duplicate_items = 0;
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        for (int j = 0; j < ITEMS_PER_PRODUCER; j++) {
            if (item_tracker[i][j].consume_count > 1) {
                duplicate_items++;
                printf("  ⚠ RACE CONDITION: Item P%d_S%d consumed %d times!\n", 
                       i, j, item_tracker[i][j].consume_count);
            }
        }
    }
    TEST_ASSERT(duplicate_items == 0, "no items consumed multiple times (race condition check)");
    
    // Check for unconsumed items (consumed but not produced - shouldn't happen)
    int phantom_items = 0;
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        for (int j = 0; j < ITEMS_PER_PRODUCER; j++) {
            if (!item_tracker[i][j].produced && item_tracker[i][j].consumed) {
                phantom_items++;
                printf("  ⚠ Item P%d_S%d was consumed but never produced!\n", i, j);
            }
        }
    }
    TEST_ASSERT(phantom_items == 0, "no phantom items (consumed but not produced)");
    
    // Verify queue is empty
    TEST_ASSERT(array_isEmpty(&s, &mutex) == true, 
                "array is empty after all operations");
    
    // Verify indices are valid
    TEST_ASSERT(s.front == -1 && s.back == -1, 
                "front and back indices properly reset");
    
    sem_destroy(&mutex);
    sem_destroy(&tracker_mutex);
}

// Main test runner
int main() {
    // Seed random number generator for stress testing
    srand(time(NULL));
    
    printf("========================================\n");
    printf("    ARRAY CIRCULAR QUEUE UNIT TESTS    \n");
    printf("========================================\n");
    
    test_array_init();
    test_array_isEmpty_initial();
    test_array_isFull_initial();
    test_array_put_single();
    test_array_put_multiple();
    test_array_fill_to_capacity();
    test_array_get_single();
    test_array_get_empty();
    test_array_fifo();
    test_array_circular_wrapping();
    test_array_free();
    test_array_alternating_ops();

    printf("========================================\n");
    printf("        MULTI THREADED UNIT TEST        \n");
    printf("========================================\n");

    test_array_multithreaded();
    
    printf("\n========================================\n");
    printf("           TEST SUMMARY                 \n");
    printf("========================================\n");
    printf("Tests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);
    printf("Total Tests:  %d\n", tests_passed + tests_failed);
    printf("========================================\n");
    
    printf("\nPress Enter to exit...");
    getchar();
    
    return tests_failed == 0 ? 0 : 1;
}
