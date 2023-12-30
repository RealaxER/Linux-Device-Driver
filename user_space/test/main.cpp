#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <name>" << std::endl;
        return 1;
    }

    std::cout << argv[1] << std::endl;
    return 0;
}
