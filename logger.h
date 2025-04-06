#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

using namespace std;

string getCurrentTime() {
    time_t now = time(nullptr);           
    tm *ltm = localtime(&now);  

    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm); 
    return string(buf);        
}

class Logger {
public:
    Logger(const string& filename) {
        log_file.open(filename, ios::out | ios::app);
        if (!log_file.is_open()) {
            throw std::runtime_error("Failed to open log file");
        }
    }

    ~Logger() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }

    void log(const string &msg) {
        string timestamp = getCurrentTime();
        string logMsg = "[" + timestamp + "] " + msg;

        cout << logMsg << endl;
        if (log_file.is_open()) log_file << logMsg << endl;
    }

private:
    ofstream log_file;
};