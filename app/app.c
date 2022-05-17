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
    int cnt=0;
    //change char into integer
    for(i=0;i<4;i++)
    {
        send_data.timer_init[i]=argv[3][i]-'0';
        if(send_data.timer_init[i])
            cnt++;
    }    
    
    if(cnt!=1)
    {
        printf("Timer Init should contain only one Non-Zero!\n");
        return 0;
    }

    send_data.time_interval=atoi(argv[1]);
    send_data.timer_cnt=atoi(argv[2]);

    if(send_data.time_interval<0||send_data.time_interval>100)
    {
        printf("The range of time interval is from 0 to 100.\n");
        return 0;
    }

    if(send_data.timer_cnt<0||send_data.timer_cnt>100)
    {
        printf("The range of timer cnt is from 0 to 100.\n");
        return 0;
    }
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