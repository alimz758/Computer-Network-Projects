#include <iostream>
#include <chrono>
#include <ctime>
#include <cmath>
#include <ratio>
class Timer
{
    public:
        void start(){
            m_StartTime = std::chrono::high_resolution_clock::now();
            m_bRunning = true;
        }
        void reset(){
            m_EndTime = std::chrono::high_resolution_clock::now();
            m_bRunning = false;
        }
        bool isRTO(){
            if(m_bRunning){
                timeOut = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed_seconds = timeOut - m_StartTime;
                return elapsed_seconds.count()>0.5? true:false;
            }
            return false;
        }
        bool isRunning(){
            return m_bRunning? true:false;
        }
        bool isTimeOut(double timeout){
            if(timeout<elapsedSeconds()){
                return true;
            }

            return false;
        }
        double elapsedMilliseconds(){
            std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
            if(m_bRunning)
            {
                endTime = std::chrono::high_resolution_clock::now();
            }
            else{
                endTime = m_EndTime;
            } 
            return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
        }
        double elapsedSeconds(){
            return elapsedMilliseconds() / 1000.0;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_EndTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> timeOut;
        bool                                               m_bRunning = false;
};