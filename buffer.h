#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include <chrono>
#include "logger.h"

using namespace std;
using namespace std::chrono;

struct Item {
    int value;
    time_point<high_resolution_clock> produced_time;
};

struct CircularBuffer {
    vector<Item> buffer;
    int SIZE;
    int in = 0, out = 0;
    int count;
    sem_t empty, full;
    pthread_mutex_t mutex;
    bool running;
    long long total_wait_time;
    int total_produced, total_consumed;
    Logger log;

    CircularBuffer(int size) : SIZE(size), buffer(size), count(0), running(true), log("logs.txt") {
        sem_init(&empty, 0, SIZE);
        sem_init(&full, 0, 0);
        pthread_mutex_init(&mutex, nullptr);
        total_wait_time = 0;
        total_produced = total_consumed = 0;
    }

    ~CircularBuffer() {
        sem_destroy(&empty);
        sem_destroy(&full);
        pthread_mutex_destroy(&mutex);
    }

    void insertItem(int id, int item) {
        sem_wait(&empty);
        if(!running) return;
        pthread_mutex_lock(&mutex);

        buffer[in] = {item, high_resolution_clock::now()};
        total_produced++;
        string msg = "Producer " + to_string(id) + " produced: " + to_string(item) + " at index " + to_string(in);
        log.log(msg);
        in = (in + 1) % SIZE;
        count++;

        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }

    void removeItem(int id) {
        sem_wait(&full);
        if(!running) return;
        pthread_mutex_lock(&mutex);

        int item = buffer[out].value;
        auto consumed_time = high_resolution_clock::now();
        total_wait_time += duration_cast<milliseconds>(consumed_time - buffer[out].produced_time).count();
        total_consumed++;
        string msg = "Consumer " + to_string(id) + " consumed: " + to_string(item) + " at index " + to_string(out);
        log.log(msg);
        out = (out + 1) % SIZE;
        count--;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }

    double avg_wait_time() {
        if(total_consumed > 0) {
            double avg_wait_ms = (total_wait_time / (double)total_consumed) / 1000.0;
            return avg_wait_ms;
        }
        else return 0;
    }

    void end(int num_prod, int num_cons) {
        for(int i = 0; i < num_prod; i++) sem_post(&empty);
        for(int i = 0; i < num_cons; i++) sem_post(&full);   
        running = false;             
    }
};