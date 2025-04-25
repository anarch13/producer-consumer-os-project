#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>

using namespace std;

class CircularBuffer {
public:
    CircularBuffer(int size) : SIZE(size), buffer(size), count(0), running(true) {
        sem_init(&empty, 0, SIZE);
        sem_init(&full, 0, 0);
        pthread_mutex_init(&mutex, nullptr);
    }

    ~CircularBuffer() {
        sem_destroy(&empty);
        sem_destroy(&full);
        pthread_mutex_destroy(&mutex);
    }

    void insertItem(int id, int item) {
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        if(!running) return;
        buffer[in] = item;
        string msg = "Producer " + to_string(id) + " produced: " + to_string(item) + " at index " + to_string(in);
        cout << msg << "\n";
        in = (in + 1) % SIZE;
        count++;

        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }

    void removeItem(int id) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        if(!running) return;
        int item = buffer[out];
        string msg = "Consumer " + to_string(id) + " consumed: " + to_string(item) + " at index " + to_string(out);
        cout << msg << "\n";
        out = (out + 1) % SIZE;
        count--;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }

    int getCount() const {
        return count;
    }

    int getSize() const {
        return SIZE;
    }

    int check() const {
        return running;
    }

    void end(int num_prod, int num_cons) {
        for(int i = 0; i < num_prod; i++) sem_post(&empty);
        for(int i = 0; i < num_cons; i++) sem_post(&full);   
        running = false;             
    }

private:
    vector<int> buffer;
    int SIZE;
    int in = 0, out = 0;
    int count;
    sem_t empty, full;
    pthread_mutex_t mutex;
    bool running;
};