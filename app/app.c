#include "app.h"

int main (int argc, char* argv[])
{
    int i;
    struct group_data send_data;

    if(argc!=4)
    {
        printf("Input should be like: ./app TIMER_INTERVAL[1-100] TIMER_CNT[1-100] TIMER_INIT[0001-8000]\n");
        return -1;
    }
    
    //change char into integer
    for(i=0;i<4;i++)
        send_data.timer_init[i]=argv[3][i]-'0';
    
    send_data.time_interval=atoi(argv[1]);
    send_data.timer_cnt=atoi(argv[2]);

    int fd;
    fd=open(DEVICE_NAME,O_WRONLY);
    if(fd<0)
    {
        printf("%s Open Error\n",DEVICE_NAME);
        return -1;
    }
    
    ioctl(fd,IOCTL_SET_MSG,&send_data);

    ioctl(fd,IOCTL_COMMAND);
    close(fd);

    return 0;

}