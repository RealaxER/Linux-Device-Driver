#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h> // Thư viện chứa các hàm ngủ

static int hello_init(void)
{
    int count;
    pr_info("Module loaded\n");

    // Sử dụng cơ chế ngủ msleep
    pr_info("Sleeping for 2 seconds...\n");
    msleep(2000); // Ngủ trong 2 giây

    // Sử dụng cơ chế ngủ wait_event_interruptible
    wait_queue_head_t wq;
    init_waitqueue_head(&wq);
    while(count <5){
        count++;
    }

    pr_info("Waiting for condition to be true...\n");
    wait_event_interruptible(wq,count >=5); // Chờ đợi cho đến khi điều kiện thỏa mãn

    return 0;
}

static void hello_exit(void)
{
    pr_info("Module unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
