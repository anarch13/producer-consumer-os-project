#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include "buffer.h"

using namespace std;

CircularBuffer buffer(11);
vector<pthread_t> producer_threads, consumer_threads;

void* producer(void* arg) {
    while (buffer.check()) {
        int item = rand() % 100;
        buffer.insertItem(*(int*)arg, item);
        usleep(500000);  // Simulate work
    }
    return nullptr;
}

void* consumer(void* arg) {
    while (buffer.check()) {
        buffer.removeItem(*(int*)arg);
        usleep(700000);  // Simulate work
    }
    return nullptr;
}

// Function to create a new producer thread
void add_producer() {
    pthread_t tid;
    int* id = new int(producer_threads.size());
    pthread_create(&tid, nullptr, producer, id);
    producer_threads.push_back(tid);
    cout << "Added a producer. Total producers: " << producer_threads.size() << endl;
}

// Function to create a new consumer thread
void add_consumer() {
    pthread_t tid;
    int* id = new int(consumer_threads.size());
    pthread_create(&tid, nullptr, consumer, id);
    consumer_threads.push_back(tid);
    cout << "Added a consumer. Total consumers: " << consumer_threads.size() << endl;
}

int main() {
    srand(time(nullptr));

    // Start with 1 producer and 1 consumer
    add_producer();
    add_consumer();

    for (int i = 0; i < 10; ++i) {
        int prevCount = buffer.getCount();
        sleep(5);
        int currCount = buffer.getCount();

        cout << "\n[Monitor] Buffer previous count: " << prevCount << " and current count: " << currCount << endl;

        if (currCount > prevCount) {
            cout << "Buffer count is increasing, so adding consumer\n";
            add_consumer();
        }    
        else if (currCount < prevCount) {
            cout << "Buffer count is decreasing, so adding producer\n";
            add_producer();
        }    
        else{
            cout << "Buffer size is same, so nothing added\n";
        }
    }

    buffer.end(producer_threads.size(), consumer_threads.size());

    for (auto& tid : producer_threads) pthread_join(tid, nullptr);
    for (auto& tid : consumer_threads) pthread_join(tid, nullptr);

    return 0;
}