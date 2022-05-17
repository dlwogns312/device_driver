#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include <linux/fs.h>

struct group_data{
	unsigned short int time_interval;
	unsigned short int timer_cnt;
	unsigned char timer_init[4];
};
//define major number and device driver name
#define DEVICE_MAJOR 242
#define DEVICE_NAME "/dev/dev_driver"

//Set the message of the device driver
#define IOCTL_SET_MSG _IOW(DEVICE_MAJOR,0,struct group_data)
#define IOCTL_COMMAND _IO(DEVICE_MAJOR,0)

