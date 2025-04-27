#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include <chrono>
#include "buffer.h"

using namespace std;
using namespace std::chrono;

CircularBuffer* buffer;
vector<pthread_t> producer_threads, consumer_threads;

void* producer(void* arg) {
    while (buffer->running) {
        int item = rand() % 100;
        buffer->insertItem((int)arg, item);
        usleep(500000);
    }
    return nullptr;
}

void* consumer(void* arg) {
    while (buffer->running) {
        buffer->removeItem((int)arg);
        usleep(500000);
    }
    return nullptr;
}

void add_producer() {
    pthread_t tid;
    int* id = new int(producer_threads.size());
    pthread_create(&tid, nullptr, producer, id);
    producer_threads.push_back(tid);
    string msg = "Added a producer. Total producers: " + to_string(producer_threads.size());
    buffer->log.log(msg);
}

void add_consumer() {
    pthread_t tid;
    int* id = new int(consumer_threads.size());
    pthread_create(&tid, nullptr, consumer, id);
    consumer_threads.push_back(tid);
    string msg = "Added a consumer. Total consumers: " + to_string(consumer_threads.size());
    buffer->log.log(msg);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <buffer_size> <iterations>\n";
        return 1;
    }

    int buf_size = stoi(argv[1]);
    int iterations = stoi(argv[2]);

    srand(time(nullptr));
    buffer = new CircularBuffer(buf_size);

    auto start = high_resolution_clock::now();

    add_producer();
    add_consumer();

    for (int i = 0; i < iterations; ++i) {
        int prevCount = buffer->count;
        sleep(3);
        int currCount = buffer->count;

        string msg = "Buffer previous count: " + to_string(prevCount) + " and current count: " + to_string(currCount);
        buffer->log.log(msg);

        if (currCount > prevCount) {
            msg = "Buffer count is increasing, so adding consumer";
            buffer->log.log(msg);
            add_consumer();
        }    
        else if (currCount < prevCount) {
            msg = "Buffer count is decreasing, so adding producer";
            buffer->log.log(msg);
            add_producer();
        }    
        else {
            msg = "Buffer size is same, so nothing added";
            buffer->log.log(msg);
        }
    }

    buffer->end(producer_threads.size(), consumer_threads.size());

    auto end = high_resolution_clock::now();
    double duration_s = duration_cast<milliseconds>(end - start).count() / 1000.0;

    for (auto& tid : producer_threads) pthread_join(tid, nullptr);
    for (auto& tid : consumer_threads) pthread_join(tid, nullptr);

    double avg_wait = buffer->avg_wait_time();
    double prod_throughput = buffer->total_produced / duration_s;
    double cons_throughput = buffer->total_consumed / duration_s;

    cout << "Average waiting time is: " << avg_wait << " ms \n";
    cout << "Producer throughput is: " << prod_throughput << " items/s \n";
    cout << "Consumer throughput is: " << cons_throughput << " items/s \n";
 
    delete buffer;
    return 0;
}