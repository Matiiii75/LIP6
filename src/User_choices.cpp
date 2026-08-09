#include "User_choices.hpp"

void User_choices::set_elaging_LB2_ON() {
    this->elaging_LB2_ON = true; 
}

void User_choices::set_elaging_LB2_percentage(double _elaging_LB2_percentage) {
    if(_elaging_LB2_percentage < 0.0 || _elaging_LB2_percentage > 1.0) 
        throw std::runtime_error("User_choices::set_elaging_LB2_percentage : percent not in [0.0,1.0]");
    this->elaging_LB2_percentage = _elaging_LB2_percentage; 
}