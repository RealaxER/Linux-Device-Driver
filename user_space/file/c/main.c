#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    /*Mở và tạo file trong C*/
    int fd = open("myfile.txt", O_CREAT | O_RDWR | O_APPEND, 0666);
    if (fd == -1) {
        perror("Error opening the file");
        return 1;
    }


    /*Ghi vào file ở cuối fiel với O_APPEND*/
    const char* data = "Hello";
    ssize_t bytes_written = write(fd, data, strlen(data));
    printf("Writing %ld bytes\n", bytes_written);
    if (bytes_written == -1) {
        perror("Error writing to the file");
        close(fd);
        return 1;
    }

    // Di chuyển con trỏ về đầu file
    lseek(fd, 0, SEEK_SET);
    // Di chuyển 0 vị trí tính từ đầu file 
    // SEEK_SET: Di chuyển con trỏ đến vị trí tính từ đầu file.
    // SEEK_CUR: Di chuyển con trỏ tính từ vị trí hiện tại của con trỏ.
    // SEEK_END: Di chuyển con trỏ tính từ cuối file.
    /*Đoc toàn bộ file và ghi vào buff*/
    char buff[100];
    memset(buff,0,sizeof(buff));
    ssize_t bytes_readed = read(fd, buff, sizeof(buff));
    if(bytes_readed == -1){
        perror("read file failed\n");
    }
    else if(bytes_readed == 0){
        printf("no bytes read\n");  
    }
    printf("Bytes read: %s\n", buff);
    close(fd);
    return 0;
}
