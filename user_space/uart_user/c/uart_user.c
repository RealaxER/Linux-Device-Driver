#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int main(void){
	int fd,len;
	struct termios uart4;
	char buff[100];
	fd = open("/dev/ttyS4", O_RDWR | O_NDELAY | O_NOCTTY);
	if(fd < 0){
		perror("uart4 note open failed\n");
		return -1;
	}
	printf("Uart4 open successful fd : %d\n",fd);

	/*Configuration for uart4*/
	uart4.c_cflag = B9600 | CS8 | CREAD | CLOCAL;

	uart4.c_iflag = IGNPAR;

	uart4.c_oflag = 0;

	uart4.c_lflag = 0;

	/*Add setting for uart4*/
	tcflush(fd,TCIFLUSH);
	tcsetattr(fd,TCSANOW, &uart4);	


	//write data to buff;
	strcpy(buff,"Hello,I'm Uart for BeagleBone Black\n\r");
	len = strlen(buff);

	len = write(fd,buff,len);
	printf("Uart4 writed %d bytes \n",len);

	printf("Sleep 5s\n");
	sleep(5);
	/*Read uart4*/
	memset(buff,0,100);
	len = read(fd,buff,100);
	printf("Uart4 readed %d bytes \n",len);
	printf("Recieved string : %s\n",buff);

	close(fd);
	return 0;

}