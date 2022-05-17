#include "dev_driver.h"

//define the length of Student ID and Name
static int name_length=10;
static int student_id=8;

//Global variable usage and device address
static int dev_port_usage=0;
static unsigned char *iom_led_addr;
static unsigned char *iom_lcd_addr;
static unsigned char *iom_fnd_addr;
static unsigned char *iom_dot_addr;

static struct mylist{
    struct timer_list timer;
    int count;
};

//variables for fnd
unsigned char value[4];
unsigned short int init_fnd=0; 
unsigned short int pos=0;

//variables for timer
unsigned short int cnt;
unsigned short int time_interval;

//variabled for lcd
unsigned short int name_dir=1;
unsigned short int num_dir=1;
unsigned short int name_i=16;
unsigned short int num_i=0;
unsigned char text_lcd[33];

struct mylist mytimer;
struct group_data mydata;


//file_operations structure
static struct file_operations fops={
    .owner=THIS_MODULE,
    .open=dev_driver_open,
    .release=dev_driver_release,
    .unlocked_ioctl=dev_driver_ioctl,
};

//when dev_driver open, call this function
int dev_driver_open(struct inode *minode, struct file *mfile)
{
    if(dev_port_usage!=0)
        return -EBUSY;
    printk(KERN_ALERT"dev_driver_open\n");
    
    dev_port_usage=1;

    return 0;
}

//timer function
static void kernel_timer_blink(unsigned long timeout) {
	struct group_data* p_data = (struct group_data*)timeout;

    int i;
	//printk("kernel_timer_blink %d\n", p_data->count);

    if(!cnt)
    {
        memset(text_lcd,' ',sizeof(text_lcd));
        memset(value,0,4);
        display();
    }
    else
    {
        cnt--;
        //increment the fnd data
        value[pos]=(value[pos])%8+1;
        if(value[pos]==init_fnd)
        {
            value[pos]=0;
            pos=(pos+1)%4;
            value[pos]=init_fnd;
        }

        //Change the lcd data
        //move student id
        if(num_dir)
        {
            if(num_i+student_id<16)
            {
                for(i=num_i+student_id;i>num_i;i--)
                    text_lcd[i]=text_lcd[i-1];
                text_lcd[num_i++]=' ';
            }
            else
            {
                for(i=num_i-1;i<student_id+num_i;i++)
                    text_lcd[i]=text_lcd[i+1];
                num_dir=0;
               text_lcd[(num_i--)+student_id-1]=' ';
            }
        }
        else
        {
            if(num_i>0)
            {
                for(i=num_i-1;i<student_id+num_i;i++)
                    text_lcd[i]=text_lcd[i+1];
                text_lcd[(num_i--)+student_id-1]=' ';
            }
            else
            {
                for(i=num_i+student_id;i>num_i;i--)
                    text_lcd[i]=text_lcd[i-1];
                num_dir=1;
                text_lcd[num_i++]=' ';
            }
        }

        //move name
        if(name_dir)
        {
            if(name_i+name_length<32)
            {
                for(i=name_i+name_length;i>name_i;i--)
                    text_lcd[i]=text_lcd[i-1];
                text_lcd[name_i++]=' ';
            }
            else{
                for(i=name_i-1;i<name_i+name_length;i++)
                    text_lcd[i]=text_lcd[i+1];
                name_dir=0;
                text_lcd[(name_i--)+name_length-1]=' ';
            }
        }
        else
        {
            if(name_i>16)
            {
                for(i=name_i-1;i<name_i+name_length;i++)
                    text_lcd[i]=text_lcd[i+1];
                text_lcd[(name_i--)+name_length+1]=' ';
            }
            else
            {
                for(i=name_i+name_length;i>name_i;i--)
                    text_lcd[i]=text_lcd[i-1];
                name_dir=1;
                text_lcd[name_i++]=' ';
            }
        }
        
        display();
        mytimer.timer.expires = get_jiffies_64() + (time_interval* HZ/10);
	    mytimer.timer.data = (unsigned long)&p_data;
	    mytimer.timer.function = kernel_timer_blink;

	    add_timer(&mytimer.timer);
    }

	return ;
}

//when dev_driver close, call this function
int dev_driver_release(struct inode *minode,struct file *mfile)
{
    dev_port_usage=0;
    printk(KERN_ALERT"dev_driver_release\n");

    return 0;
}

//display all values on device
void display(void)
{
    int i;
    unsigned short int _s_value=0;

    //print dot
    for(i=0;i<=9;i++)
    {
        _s_value = fpga_number[value[pos]][i] & 0x7F;
		outw(_s_value,(unsigned int)iom_dot_addr+i*2);
    }

    //print led
    _s_value = 0x80>>(value[pos]-1);	    
    outw(_s_value,(unsigned int)iom_led_addr);

    //print fnd
    _s_value = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(_s_value,(unsigned int)iom_fnd_addr);	  

    //print lcd
    for(i=0;i<32;i++)
    {
        _s_value = (text_lcd[i] & 0xFF) << 8 | text_lcd[i + 1] & 0xFF;
		outw(_s_value,(unsigned int)iom_lcd_addr+i);
        i++;
    }


}

//set ioctl operation for device driver
long dev_driver_ioctl(struct file *file,unsigned int ioctl_num,unsigned long ioctl_param)
{
    
    int i;int ret;
    //initialize the variables
    switch(ioctl_num){
        case IOCTL_SET_MSG:
            //get data from user
            ret=copy_from_user(&mydata,(struct group_data*)ioctl_param,sizeof(struct group_data));

            if(ret)
                {
                    printk(KERN_ALERT "Error occured at copying from user!\n");
                    return -EFAULT;
                }

            name_i=16;name_dir=1;num_i=0;num_dir=1;
            cnt=mydata.timer_cnt-1;
            memcpy(value,mydata.timer_init,4);

            printk(KERN_ALERT"Timer_interval : %d Timer_cnt :%d Timer_init : %s\n",mydata.time_interval,mydata.timer_cnt,value);
            //find the initial number of fnd
            
            for(i=0;i<4;i++)
                if(value[i])
                {
                    pos=i;
                    init_fnd=value[i];
                    break;
                }
            
            time_interval=mydata.time_interval;
            //initialize lcd
            memset(text_lcd,' ',sizeof(text_lcd));
            for(i=0;i<student_id;i++)
                text_lcd[i]=id[i];
            for(i=16;i<16+name_length;i++)
                text_lcd[i]=user_name[i];

            display();
            break;
        case IOCTL_COMMAND :
            printk(KERN_INFO"Start the Timer!\n");
            mytimer.timer.expires=get_jiffies_64()+(time_interval*HZ/10);
            mytimer.timer.data=(unsigned long)&mydata;
            mytimer.timer.function=kernel_timer_blink;

            add_timer(&mytimer.timer);
            break;
        default:
            printk(KERN_WARNING"Command Error!\n");
            return -EFAULT;
            break;
    }
    
    return 0;
}

//init function
static int __init iom_timer_init(void)
{
   int result;
  

   result=register_chrdev(DEVICE_MAJOR,DEVICE_NAME,&fops);
   if(result<0)
   {
       printk(KERN_WARNING"error %d\n",result);
       return result;
   }
    printk("timer_dev_driver_init\n");
   //init timer and ioremap devices
   init_timer(&(mytimer.timer));
   iom_led_addr=ioremap(LED_ADDRESS,0x1);
   iom_fnd_addr=ioremap(FND_ADDRESS,0x4);
   iom_dot_addr=ioremap(DOT_ADDRESS,0x10);
   iom_lcd_addr=ioremap(LCD_ADDRESS,0x32);
   
   if(iom_led_addr==NULL||iom_fnd_addr==NULL||iom_dot_addr==NULL||iom_lcd_addr==NULL)
   {
       printk(KERN_WARNING"ioremap fail!\n");
       return -EFAULT;
   }

   printk("init module, %s major number : %d\n",DEVICE_NAME,DEVICE_MAJOR);
   return 0;
}

//exit function
void __exit iom_timer_exit(void)
{
    iounmap(iom_led_addr);
    iounmap(iom_fnd_addr);
    iounmap(iom_dot_addr);
    iounmap(iom_lcd_addr);

    del_timer_sync(&(mytimer.timer));
    unregister_chrdev(DEVICE_MAJOR,DEVICE_NAME);
    printk("Exit module\n");
}

module_init(iom_timer_init);
module_exit(iom_timer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LEEJAEHOON");