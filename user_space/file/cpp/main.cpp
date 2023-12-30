#include <iostream>
#include <fstream>
#include <string>
int main() {

    std::fstream file("hello.txt",std::ios::out | std::ios::in | std::ios::app);
    if(!file){
        std::cerr << "file hello.txt not open" << std::endl;
    }
    /*Ghi vào file*/
    file << "Hello, C++ File Handling!" << std::endl;

    /*Di chuyển con trỏ vào đầu file */
    file.seekg(0, std::ios::beg); // Di chuyển con trỏ lên đầu file
    //file.seekg(0, std::ios::end); // Di chuyển con trỏ đến cuối file và dịch 0 vị trí , nó cũng y hệt C thôi 
    // đọc thư viện C++ thì nó khá khó nên phải đọc google tìm hiểu thôi 
    //seekg(): Được sử dụng để di chuyển con trỏ của std::ifstream (đọc) đến vị trí cụ thể trong file.
    //seekp(): Được sử dụng để di chuyển con trỏ của std::ofstream (ghi) đến vị trí cụ thể trong file.
    std::string line;
    // hàm củ chuối này nó trả về true khi gặp endline và trả về false khi lỗi hoặc là hết
    std::getline(file, line);
    std:: cout << line << std::endl;   

    // việc đọc trên chỉ đọc được 1 dòng thôi muốn đọc toàn file phải 
    while(std::getline(file, line)){
        std::cout << line << std::endl;
    }
    file.close();

    return 0;
}
