#include "Master.hpp"

int main(int argc, char* argv[]) {

    std::string file = argv[1]; 
    Data data(file); 
    
    Master prog(data, 0); 
    prog.build_SG(); 
    
    prog.SG.display_SG(); 
    prog.SG.display_SG_detail(); 

    return 0; 
}