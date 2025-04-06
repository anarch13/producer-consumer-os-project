#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include "buffer.h"

using namespace std;

#define MAX_THREADS 4

CircularBuffer buffer(3);
vector<pthread_t> producerThreads, consumerThreads;
bool running = true;

void* producer(void* arg) {
    while (running) {
        int item = rand() % 100;
        buffer.insertItem(item);
        usleep(500000);  // Simulate work (0.5 sec)
    }
    return nullptr;
}

void* consumer(void* arg) {
    while (running) {
        buffer.removeItem();
        usleep(700000);  // Simulate work (0.7 sec)
    }
    return nullptr;
}

// Function to adjust the number of producers/consumers
void adjustThreads(int newProducers, int newConsumers) {
    int currentProducers = producerThreads.size();
    int currentConsumers = consumerThreads.size();

    while (currentProducers < newProducers && producerThreads.size() < MAX_THREADS) {
        pthread_t tid;
        pthread_create(&tid, nullptr, producer, nullptr);
        producerThreads.push_back(tid);
        currentProducers++;
    }

    while (currentConsumers < newConsumers && consumerThreads.size() < MAX_THREADS) {
        pthread_t tid;
        pthread_create(&tid, nullptr, consumer, nullptr);
        consumerThreads.push_back(tid);
        currentConsumers++;
    }

    while (currentProducers > newProducers && !producerThreads.empty()) {
        pthread_cancel(producerThreads.back());
        producerThreads.pop_back();
        currentProducers--;
    }

    while (currentConsumers > newConsumers && !consumerThreads.empty()) {
        pthread_cancel(consumerThreads.back());
        consumerThreads.pop_back();
        currentConsumers--;
    }
}

int main() {
    srand(time(nullptr));

    // Initial threads
    adjustThreads(2, 2);  

    for (int i = 0; i < 5; ++i) { // Loop 5 times
        sleep(5);  // Wait for 5 seconds

        int newProducers = rand() % (MAX_THREADS + 1);  // 0 to MAX_THREADS
        int newConsumers = rand() % (MAX_THREADS + 1);  // 0 to MAX_THREADS

        cout << "Adjusting threads to " << newProducers << " producers and "
             << newConsumers << " consumers." << endl;

        adjustThreads(newProducers, newConsumers);
    }

    // Stop execution
    running = false;

    for (auto& tid : producerThreads) pthread_join(tid, nullptr);
    for (auto& tid : consumerThreads) pthread_join(tid, nullptr);

    return 0;
}
