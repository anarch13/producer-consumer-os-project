#pragma once
#include <pthread.h>
#include <fstream>
#include <string>
#include <queue>
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace std::chrono;

string getCurrentTime() {
    using namespace std::chrono;
    
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000; 

    time_t t = system_clock::to_time_t(now);
    tm* ltm = localtime(&t);

    char buf[30];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);

    stringstream ss;
    ss << buf << "." << setfill('0') << setw(3) << ms.count();

    return ss.str();
}


struct Logger {
    ofstream ofs;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
    queue<string> _queue;
    pthread_t _thread;
    bool stopping = false;

    Logger(const string &file) {
        ofs.open(file, ios::out | ios::trunc);
        if (!ofs.is_open())
            throw runtime_error("Cannot open log file: " + file);

        pthread_mutex_init(&mtx, nullptr);
        pthread_cond_init(&cv,  nullptr);

        pthread_create(&_thread, nullptr, threadEntry, this);
    }

    ~Logger() {
        pthread_mutex_lock(&mtx);
        stopping = true;
        pthread_mutex_unlock(&mtx);
        pthread_cond_signal(&cv);
        pthread_join(_thread, nullptr);

        ofs.close();
        pthread_mutex_destroy(&mtx);
        pthread_cond_destroy(&cv);
    }

    void log(const std::string &msg) {
        pthread_mutex_lock(&mtx);
        string timestamp = getCurrentTime();
        string log_msg = "[" + timestamp + "] " + msg;
        _queue.push(log_msg);
        pthread_mutex_unlock(&mtx);
        pthread_cond_signal(&cv);
    }

    static void* threadEntry(void* arg) {
        return ((Logger*)arg)->process();
    }

    void* process() {
        while (true) {
            pthread_mutex_lock(&mtx);
            while (_queue.empty() && !stopping) {
                pthread_cond_wait(&cv, &mtx);
            }
            if (_queue.empty() && stopping) {
                pthread_mutex_unlock(&mtx);
                break;
            }

            string msg = _queue.front();
            _queue.pop();
            pthread_mutex_unlock(&mtx);

            ofs << msg << "\n";
            ofs.flush();
        }
        return nullptr;
    }
};
