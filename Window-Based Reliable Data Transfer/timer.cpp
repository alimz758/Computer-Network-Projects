#include <iostream>
#include <chrono>
#include <ctime>
#include <cmath>
#include <ratio>
class Timer
{
    public:
        void start(){
            startTime = std::chrono::high_resolution_clock::now();
            running = true;
            timeOut = startTime + std::chrono::milliseconds(500);
        }
        bool isTimeout(){
            return std::chrono::high_resolution_clock::now() > timeOut? true:false;
        }
        void reset(){
            stop = std::chrono::high_resolution_clock::now();
            running = false;
        }
        bool isRTO(){
            if(running){
                timeOut = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed_seconds = timeOut - startTime;
                return elapsed_seconds.count()>0.5? true:false;
            }
            return false;
        }
        bool isRunning(){
            return running? true:false;
        }
        double elapsedMilliseconds(){
            std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
            if(running)
            {
                endTime = std::chrono::high_resolution_clock::now();
            }
            else{
                endTime = stop;
            } 
            return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        }
        double elapsedSeconds(){
            return elapsedMilliseconds() / 1000.0;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> stop;
        std::chrono::time_point<std::chrono::high_resolution_clock> timeOut;
        bool                                               running = false;
};