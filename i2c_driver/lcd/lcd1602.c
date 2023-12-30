#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <lcd1602_reg.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/string.h>
/**
 * @implements : Steps implement i2c driver
 * step1 : impletment i2c driver, of_device_id , i2c_devivce_id,
 * step2 : add all id to i2c_driver 
 * now ,system will find id and 
*/

/*function prototype of struct file_opertaions*/
int lcd_open(struct inode *, struct file *);
int lcd_release(struct inode *, struct file *);
ssize_t lcd_read(struct file *, char __user *, size_t, loff_t *);
ssize_t lcd_write(struct file *, const char __user *, size_t, loff_t *);


/*function prototype of struct i2c_driver*/
int lcd_probe (struct i2c_client *client, const struct i2c_device_id *id);
int lcd_remove (struct i2c_client *client);

/*function prototype definetion device file*/
int lcd_create_df(void);
void lcd_send_cmd(uint8_t cmd);
void lcd_send_data(uint8_t data);
void lcd_backlight(uint8_t state);
void lcd_clear(void);
void lcd_write_string(char *str);
void lcd_write_nibble(uint8_t nibble, uint8_t rs);
void lcd_set_cursor(uint8_t row, uint8_t column);

/*function prototype register */
static int lcd_i2c_read_reg(struct i2c_client *client, uint8_t reg_addr, uint8_t *data);
static int lcd_i2c_write_reg(struct i2c_client *client, uint8_t data);
int lcd_initialization(void);

/*initialzation device file*/
static struct file_operations lcd_fops = {
  .owner= THIS_MODULE,
  .open = lcd_open,
  .release = lcd_release,
  .read = lcd_read,
  .write = lcd_write,
};

/*initialization name id of i2c cilent*/
static struct i2c_device_id lcd_id[] = {
  {"lcd1602",0},/*name,data private*/
  {},
};

/*initialization compatible for device tree*/
static struct of_device_id lcd_match[] = {
  {.compatible="ti,lcd1602"},
  {},
};
MODULE_DEVICE_TABLE(i2c,lcd_id);
MODULE_DEVICE_TABLE(of,lcd_match);

size_t
/*implement lcd driver and define functions of i2c driver*/
struct i2c_driver lcd_driver = {
  .probe =lcd_probe,
  .remove = lcd_remove,
  .driver = {
    .name = "lcd",
    .owner = THIS_MODULE,
    .of_match_table = lcd_match,
  },
  .id_table = lcd_id,
};
struct lcd_dev {
  struct class *class;
  struct device *device;
  struct cdev cdev;
  dev_t devid;
  int major;
  int minor;
  struct i2c_client *client;
  char buffer[MAX_SIZE];
};
struct lcd_dev lcd;
/**
 * @brief: Function of file_operations
*/
int lcd_open(struct inode * inode, struct file *file){
  file->private_data = &lcd;
 // lcd_initialization();
  printk("I'm in open\n");
  return 0;
}

int lcd_release(struct inode *inode, struct file *file){
  struct lcd_dev *lcd_tmp = (struct lcd_dev*)file->private_data;
  printk("I'm in release");
  return 0;
}

ssize_t lcd_read(struct file *file, char __user * buffer, size_t length, loff_t * offset){
  struct lcd_dev *lcd_tmp = (struct lcd_dev*)file->private_data;
  return length;
}

ssize_t lcd_write(struct file *file, const char __user * buffer, size_t length, loff_t *offset ){
  static uint8_t state =0;
  printk("I'm in write\n");
  if(length >sizeof(lcd.buffer)){
    printk("length > buffer\n");
    return -EINVAL;
  }
  //get buffer 
  if(copy_from_user(lcd.buffer,buffer,strlen(buffer))){
    printk("copy from user err\n");
    return -1;
  }

  // handle buffer
  if(strstr(lcd.buffer,"-ON")!= NULL){
    lcd_backlight(1);
  }
  else if(strstr(lcd.buffer,"-OFF")!= NULL){
    lcd_backlight(0);
  }
  else if(strstr(lcd.buffer,"-CLEAR")!= NULL){
    lcd_clear();
  }
  else if(strstr(lcd.buffer,"-0")!= NULL){
    lcd_set_cursor(0,0);
  }
  else if(strstr(lcd.buffer,"-1")!= NULL){
    lcd_set_cursor(1,0);
  }
  else{
    printk("lcd buffer : %s\n",lcd.buffer);
    lcd_write_string(lcd.buffer);
  }
  
  return length;
}

int lcd_create_df(void){
  if((alloc_chrdev_region(&lcd.devid, 0, 1, "lcd_dev")) <0){
    pr_err("Cannot allocate major number\n");
    return -1;
  }
  lcd.major = MAJOR(lcd.devid);
  lcd.minor = MINOR(lcd.devid);
  pr_info("Major = %d Minor = %d \n", lcd.major, lcd.minor);

  /*Creating cdev structure*/
  cdev_init(&lcd.cdev,&lcd_fops);

  /*Adding character device to the system*/
  if((cdev_add(&lcd.cdev,lcd.devid,1)) < 0){
    pr_err("Cannot add the device to the system\n");
    unregister_chrdev_region(lcd.devid,1);
    return -1;
  }
  /*Creating struct class*/
  if(IS_ERR(lcd.class = class_create(THIS_MODULE,"lcd_class"))){
    pr_err("Cannot create the struct class\n");
    unregister_chrdev_region(lcd.devid,1);
    return -1;
  }
  /*Creating struct device*/
  if(IS_ERR(lcd.device = device_create(lcd.class,NULL,lcd.devid,NULL,"lcd_device"))){
    pr_err("Cannot create the Device 1\n");
    class_destroy(lcd.class);
    unregister_chrdev_region(lcd.devid,1);
    return -1;
  }
  return 0;
}

/**
 * Function : lcd_initialization register 
*/
int lcd_initialization(void){
  printk("lcd initialization\n");
  mdelay(50);
  lcd_write_nibble(0x03, 0);
  mdelay(5);
  lcd_write_nibble(0x03, 0);
  mdelay(1);
  lcd_write_nibble(0x03, 0);
  mdelay(1);
  lcd_write_nibble(0x02, 0);
  lcd_send_cmd(0x28);
  lcd_send_cmd(0x0C);
  lcd_send_cmd(0x06);
  lcd_send_cmd(0x01);
  mdelay(2);
  return 0;
}
void lcd_send_data(uint8_t data) {
  uint8_t upper_nibble = data >> 4;
  uint8_t lower_nibble = data & 0x0F;
  lcd_write_nibble(upper_nibble, 1);
  lcd_write_nibble(lower_nibble, 1);
}
void lcd_clear(void) {
	lcd_send_cmd(0x01);
  mdelay(2);
}

void lcd_write_string(char *str) {
  while (*str) {
    lcd_send_data(*str++);
  }
}

void lcd_backlight(uint8_t state) {
  if (state) {
    backlight_state = 1;
  } else {
    backlight_state = 0;
  }
}
void lcd_send_cmd(uint8_t cmd) {
  uint8_t upper_nibble = cmd >> 4;
  uint8_t lower_nibble = cmd & 0x0F;
  lcd_write_nibble(upper_nibble, 0);
  lcd_write_nibble(lower_nibble, 0);
  if (cmd == 0x01 || cmd == 0x02) {
    mdelay(2);
  }
}
void lcd_set_cursor(uint8_t row, uint8_t column) {
    uint8_t address;
    switch (row) {
        case 0:
            address = 0x00;
            break;
        case 1:
            address = 0x40;
            break;
        default:
            address = 0x00;
    }
    address += column;
    lcd_send_cmd(0x80 | address);
}
void lcd_write_nibble(uint8_t nibble, uint8_t rs) {
  uint8_t data = nibble << D4_BIT;
  data |= rs << RS_BIT;
  data |= backlight_state << BL_BIT; // Include backlight state in data
  data |= 1 << EN_BIT;
  lcd_i2c_write_reg(lcd.client,data);
  mdelay(1);
  data &= ~(1 << EN_BIT);
  lcd_i2c_write_reg(lcd.client,data);
}
static int lcd_i2c_write_reg(struct i2c_client *client,uint8_t data)
{
  int ret = 0; /*khởi tạo một biến ret để nhận return */
  uint8_t w_buf[1]; /*khởi tạo buffer 2 giá trị*/

  struct i2c_msg lcd_msg; /*khởi tạo i2c messegger chứa các giá trị như address byte ,flags*/

// w_buf[0] = SLAVE_ADDRESS_LCD; /*biến đầu nhận địa chỉ cần ghi*/
  w_buf[0] = data;/*biến sau nhận giá trị ghi*/

  lcd_msg.addr = client->addr; /*copy address từ i2c_cilent sang cho gói messegger để nó thực hiện thay mình*/
  lcd_msg.buf = w_buf; /*copy gói buffer */
  lcd_msg.flags = 0; /* I2C direction ： write , để 0 là ghi 1 read*/
  lcd_msg.len = sizeof(w_buf);  /*có 2 byte thôi nên sizeof*/

  ret = i2c_transfer(client->adapter, &lcd_msg, 1); /*cuối cùng gọi i2c_transfer ra để nó ghi vào, dùng adapter vì nó là bộ chuyển đổ hàm */
  if (ret < 0)
      return ret;
  else if (ret != 1)
      return -EIO;
  return 0;
}

static int lcd_i2c_read_reg(struct i2c_client *client, uint8_t reg_addr, uint8_t *data){
  int ret =0;
  struct i2c_msg lcd_msgs[2];/*nhận byte thì mình vẫn phải gửi byte write sau đó mới nhận được nên cần 2 msg*/

  /*cấu hình msg đầu để ghi*/
  lcd_msgs[0].addr = client->addr;
  lcd_msgs[0].buf = &reg_addr; /* send regsiter */
  lcd_msgs[0].flags = 0;       /* I2C direction ： write */
  lcd_msgs[0].len = 1; /*ghi mỗi địa chỉ cần đọc thôi nên là 1*/

  /*cấu hình msg để nhận giá trị*/
  lcd_msgs[1].addr = client->addr;
  lcd_msgs[1].buf = data;       /*gán địa chỉ của biến data cần ghi vào đây*/
  lcd_msgs[1].flags = I2C_M_RD; /* I2C direction ： read */
  lcd_msgs[1].len = 1;

  /*cấu hình chuyển byte*/
  ret = i2c_transfer(client->adapter, lcd_msgs, 2); /*2msg mà nên mình cần 2*/
  if (ret < 0)
    return ret;
  else if (ret != 2)
    return -EIO;
  return 0;
}


/**
 * @brief: Function static int lcd_read_temp(int16_t *temp)of i2c driver
*/
int lcd_probe (struct i2c_client *client, const struct i2c_device_id *id){
  printk("I'm in probe\n");
  /*if check have a id compatible it will jump here*/

  /*register device file*/
  if(lcd_create_df() <0){
    pr_info("Can't create device file\n");
    return -1;
  }
  /*Get client*/
  lcd.client = client;
  pr_info("Device Driver Insert...Done!!!\n");
  printk("client address : %d\n",lcd.client->addr);
  lcd_initialization();
  lcd_backlight(1);
  lcd_clear();
	lcd_set_cursor(0, 0);
  lcd_write_string("bui hien");
  return 0;
}
int lcd_remove (struct i2c_client *client){
  printk("I'm in remove\n");
  device_destroy(lcd.class,lcd.devid);
  class_destroy(lcd.class);
  cdev_del(&lcd.cdev);
  unregister_chrdev_region(lcd.devid, 1);
  return 0;
}
/**
 * @brief: Function module init
*/
int __init lcd_init(void){
  int ret;
  printk("I'm in init\n");
  ret = i2c_add_driver(&lcd_driver);
  printk("Ret: %d\n", ret);
  if(ret <0){
    printk("Not registerd\n");
    return ret;
  }
  return 0;
}

/*function module exit*/
void __exit lcd_exit(void){
  printk("I'm in exit\n");
  i2c_del_driver(&lcd_driver);
}


module_init(lcd_init);
module_exit(lcd_exit);
/**
 * @brief: introduction module 
*/
MODULE_AUTHOR("BUI DINH HIEN");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("lcd a simple module");