// SPDX-License-Identifier: GPL-2.0-only
/*
 * socket_can.c  Driver socket can in userspace for IMX6U connect with stm32
 *
 * Copyright 2023 VinaLinux.
 *
 * Author: Bùi Hiển (Mark) buihien29112002@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <pthread.h>

struct can_frame receiveframe = {0};
struct can_frame sendframe = {0};
struct can_filter filters[3];
int sockfd;
int flag = 1;

void CAN_Config(void){
	struct ifreq ifr = {0};
	struct sockaddr_can can_addr = {0};
	int i;
	int ret;

	if((sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0){
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	strcpy(ifr.ifr_name, "can0");
	if((ioctl(sockfd, SIOCGIFINDEX, &ifr)) != 0){
		perror("ioctl error");
		exit(EXIT_FAILURE);
	}

	can_addr.can_family = AF_CAN;
	can_addr.can_ifindex = ifr.ifr_ifindex;
	if((ret = bind(sockfd, (struct sockaddr *)&can_addr, sizeof(can_addr))) < 0){
		perror("bind error");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	filters[0].can_id = 0x123;
	filters[1].can_id = 0x234;
	filters[2].can_id = 0x456;
	for(i = 0;i < 3;i++)
		filters[i].can_mask = 0x7FF; // ID
	setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, &filters, sizeof(filters)); 

	sendframe.can_id = 0x123; // I0x123
	sendframe.can_dlc = 8; 
	sendframe.data[0] = 0x01;
	sendframe.data[1] = 0x02;
	sendframe.data[2] = 0x03;
	sendframe.data[3] = 0x04;
	sendframe.data[4] = 0x05;
	sendframe.data[5] = 0x06;
	sendframe.data[6] = 0x07;
	sendframe.data[7] = 0x08;
}

void *canReceive(void *arg){
	while(1){
		int i = 0;
		if((read(sockfd, &receiveframe, sizeof(struct can_frame))) < 0){
			perror("read error");
			break;
		}

		if(receiveframe.can_id & CAN_ERR_FLAG){ 
			printf("Error frame!\n");
			break;
		}

		printf("Received CAN data from STM32F103ZET6\n");
		if(receiveframe.can_id & CAN_EFF_FLAG) 
			printf("Received <0x%08x> \n", receiveframe.can_id & CAN_EFF_MASK);
		else
			printf("Received <0x%03x> \n", receiveframe.can_id & CAN_SFF_MASK);

		if(receiveframe.can_id & CAN_RTR_FLAG){ 
			printf("remote request\n");
			continue;
		}

		printf("can_dlc: [%d] \ndata: ", receiveframe.can_dlc);
		for(i;i < receiveframe.can_dlc;i++)
			printf("%02x ", receiveframe.data[i]);
		printf("\n\n");
		//usleep(5000);
		flag = 1;
	}
}

void *canSend(void *arg){
	while(1){
		sleep(2);
		if(flag == 1){
			if((write(sockfd, &sendframe, sizeof(sendframe))) != sizeof(sendframe)){ 
				perror("write error");
				break;
			}
			flag = 0;
		}
	}	
}

void threadCreate(void){
	pthread_t t1;
	pthread_t t2;
	int ret;
	
	if(ret = pthread_create(&t1, NULL, canReceive, NULL)){
		printf("canReceive thread create fail\r\n");
		return;
	}
	pthread_detach(t1);

	if(ret = pthread_create(&t2, NULL, canSend, NULL)){
		printf("canSend thread create fail\r\n");
		return;
	}
	pthread_detach(t2);	
}

int main(void){
	CAN_Config();
	//printf("CAN_Config end\n");
	threadCreate();
	//printf("threadCreate end\n");
	while(1){
		sleep(10000);
	}
	return 0;
}
