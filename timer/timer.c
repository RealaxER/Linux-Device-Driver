#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/stdarg.h>

static void counter(int num,...){
    int i =0;
    va_list my_var;
    va_start(my_var, num);
    for(i =0;i<num;i++){
        pr_info("%d\n",va_arg(my_var,int));
    }
    va_end(my_var);
}


int init_module(void)
{
    pr_info("bui dinh hien\n");
    counter(5,1,2,3,4,5);
    return 0;
}

void cleanup_module(void)
{

}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bui Dinh Hien");
MODULE_DESCRIPTION("GPIO led kernel module");