#include <string>
#include <iostream>
#include <fstream>

int main(int argc, char **argv){
    std::fstream file("/dev/my_device",std::ios::out);
    if(!file.is_open()){
        std::cerr << "Can't open device" << std::endl;
    }

    /*Write to file */
    if(argc != 0 ){
        file << argv[1] << std::endl;
    }
    file.close();
    return 0;
}