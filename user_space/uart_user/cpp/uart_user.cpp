#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <cstring>
int main() {
    int fd, len;
    char received_buff[255];
    struct termios options; /* Serial ports setting */

    fd = open("/dev/ttyS4", O_RDWR | O_NDELAY | O_NOCTTY);
    if (fd < 0) {
        std::cerr << "Error opening serial port" << std::endl;
        return -1;
    }

    /* Read current serial port settings */
    // tcgetattr(fd, &options);

    /* Set up serial port */
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;

    /* Apply the settings */
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &options);

    /* Write to serial port */
    std::string buff = "Hello from my BeagleBone Black\n\r";
    len = write(fd, buff.c_str(), buff.size());
    std::cout << "Wrote " << len << " bytes over UART" << std::endl;

    std::cout << "You have 5s to send me some input data..." << std::endl;
    sleep(5);

    /* Read from serial port */
    memset(received_buff, 0, sizeof(received_buff));
    len = read(fd, received_buff, sizeof(received_buff) - 1);
    std::cout << "Received " << len << " bytes" << std::endl;

    // Tạo std::string từ mảng char nhận được từ UART
    std::string received_data(received_buff);

    std::cout << "Received string: " << received_data << std::endl;

    close(fd);
    return 0;
}
