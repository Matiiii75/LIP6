#include "Timer.hpp"

void Timer::start_timer() {
    this->start = std::chrono::steady_clock::now(); 
}

double Timer::get_temps_passe() {
    auto temps_passe = std::chrono::steady_clock::now(); 
    std::chrono::duration<double> duration; 
    duration = temps_passe - this->start;  
    return duration.count(); 
}

