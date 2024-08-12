#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>

class Logger{
private:
    std::ofstream logfile;
public:
    Logger(const std::string& filename): logfile(filename,std::ios::app){
        if(!logfile.is_open()){
            throw std::runtime_error("Unable to open file:"+filename);
        }
    }

    void log(const std::string& message){
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        logfile<<std::ctime(&now_time) << ":" << message << std::endl;
    }
};