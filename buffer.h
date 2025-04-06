#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include "logger.h"

using namespace std;

class CircularBuffer {
public:
    CircularBuffer(int size) : SIZE(size), buffer(size), log("LOG_FILE.txt") {
        sem_init(&emptySlots, 0, SIZE);
        sem_init(&fullSlots, 0, 0);
        pthread_mutex_init(&mutex, nullptr);
    }

    ~CircularBuffer() {
        sem_destroy(&emptySlots);
        sem_destroy(&fullSlots);
        pthread_mutex_destroy(&mutex);
    }

    void insertItem(int item) {
        sem_wait(&emptySlots);
        pthread_mutex_lock(&mutex);

        buffer[in] = item;
        string msg = "Produced: " + to_string(item) + " at index " + to_string(in);
        log.log(msg);
        in = (in + 1) % SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&fullSlots);
    }

    int removeItem() {
        sem_wait(&fullSlots);
        pthread_mutex_lock(&mutex);

        int item = buffer[out];
        string msg = "Consumed: " + to_string(item) + " at index " + to_string(in);
        log.log(msg);
        out = (out + 1) % SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&emptySlots);

        return item;
    }
private:
    vector<int> buffer;
    int SIZE;
    int in = 0, out = 0;
    sem_t emptySlots, fullSlots;
    pthread_mutex_t mutex;
    Logger log;
};