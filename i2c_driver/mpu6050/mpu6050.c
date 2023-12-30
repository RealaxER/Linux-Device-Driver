#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <mpu6050_reg.h>
#include <linux/uaccess.h>
/**
 * @implements : Steps implement i2c driver
 * step1 : impletment i2c driver, of_device_id , i2c_devivce_id,
 * step2 : add all id to i2c_driver 
 * now ,system will find id and 
*/

#define MPU6050_USE_INT 0

/*function prototype of struct file_opertaions*/
int mpu6050_open(struct inode *, struct file *);
int mpu6050_release(struct inode *, struct file *);
ssize_t mpu6050_read(struct file *, char __user *, size_t, loff_t *);
ssize_t mpu6050_write(struct file *, const char __user *, size_t, loff_t *);


/*function prototype of struct i2c_driver*/
int mpu6050_probe (struct i2c_client *client, const struct i2c_device_id *id);
int mpu6050_remove (struct i2c_client *client);

/*function prototype definetion device file*/
int mpu6050_create_df(void);


/*function prototype register */
static int mpu6050_i2c_read_reg(struct i2c_client *client, uint8_t reg_addr, uint8_t *data);
static int mpu6050_i2c_write_reg(struct i2c_client *client, uint8_t reg_addr, uint8_t data);
int mpu6050_initialization(void);
static int mpu6050_deinit(void);
static int mpu6050_read_id(uint8_t *id);
static int mpu6050_read_accel(struct mpu6050_accel *acc);
static int mpu6050_read_gyro(struct mpu6050_gyro *gyro);
static int mpu6050_read_temp(short *temp);
/*initialzation device file*/
static struct file_operations mpu6050_fops = {
  .owner= THIS_MODULE,
  .open = mpu6050_open,
  .release = mpu6050_release,
  .read = mpu6050_read,
  .write = mpu6050_write,
};

/*initialization name id of i2c cilent*/
static struct i2c_device_id mpu6050_id[] = {
  {"mpu6050",0},/*name,data private*/
  {},
};

/*initialization compatible for device tree*/
static struct of_device_id mpu6050_match[] = {
  {.compatible="ti,mpu6050"},
  {},
};
MODULE_DEVICE_TABLE(i2c,mpu6050_id);
MODULE_DEVICE_TABLE(of,mpu6050_match);

/*implement mpu6050 driver and define functions of i2c driver*/
struct i2c_driver mpu6050_driver = {
  .probe =mpu6050_probe,
  .remove = mpu6050_remove,
  .driver = {
    .name = "mpu6050",
    .owner = THIS_MODULE,
    .of_match_table = mpu6050_match,
  },
  .id_table = mpu6050_id,
};
struct mpu6050_dev {
  struct class *class;
  struct device *device;
  struct cdev cdev;
  dev_t devid;
  int major;
  int minor;
  struct mpu6050_data data;
  struct i2c_client *client;
};
struct mpu6050_dev mpu6050;
/**
 * @brief: Function of file_operations
*/
int mpu6050_open(struct inode * inode, struct file *file){
  int ret;
  uint8_t id_mpu6050;
  file->private_data = &mpu6050;
  printk("I'm in open\n");
  /*check id whoami*/
  if(mpu6050_read_id(&id_mpu6050) != 0)
  {
    printk("don't find %d\r\n", id_mpu6050);
    return -ENXIO;
  }
  /*initialization mpu6050*/
  ret = mpu6050_initialization();
  if(ret !=0){
    printk("%d-%s init failed %d\r\n", __LINE__, __FUNCTION__, ret);
    return -ENXIO;
  }
  return 0;

}
int mpu6050_release(struct inode *inode, struct file *file){
  int ret =0;
  struct mpu6050_dev *mpu6050_tmp = (struct mpu6050_dev*)file->private_data;
  printk("I'm in release");
  ret = mpu6050_deinit();  
  return ret;
}

ssize_t mpu6050_read(struct file *file, char __user * buffer, size_t length, loff_t * offset){
  int ret;
  struct mpu6050_dev *mpu6050_tmp = (struct mpu6050_dev*)file->private_data;
  struct mpu6050_data data;
  int size_data = sizeof(data);
  if(size_data !=length){
    printk("Erro get size data\n");
    return -1;
  }
  ret = mpu6050_read_accel(&data.accel);
  ret = mpu6050_read_gyro(&data.gyro);
  printk("I'm in read\n");
  if(copy_to_user(buffer,&data,sizeof(data)))
    ret = -1;
  else
    ret = size_data;
  return size_data;
}
ssize_t mpu6050_write(struct file *file, const char __user * buffer, size_t length, loff_t *offset ){
  printk("I'm in write\n");
  return length;
}

int mpu6050_create_df(void){
  if((alloc_chrdev_region(&mpu6050.devid, 0, 1, "mpu6050_dev")) <0){
    pr_err("Cannot allocate major number\n");
    return -1;
  }
  mpu6050.major = MAJOR(mpu6050.devid);
  mpu6050.minor = MINOR(mpu6050.devid);
  pr_info("Major = %d Minor = %d \n", mpu6050.major, mpu6050.minor);

  /*Creating cdev structure*/
  cdev_init(&mpu6050.cdev,&mpu6050_fops);

  /*Adding character device to the system*/
  if((cdev_add(&mpu6050.cdev,mpu6050.devid,1)) < 0){
    pr_err("Cannot add the device to the system\n");
    unregister_chrdev_region(mpu6050.devid,1);
    return -1;
  }
  /*Creating struct class*/
  if(IS_ERR(mpu6050.class = class_create(THIS_MODULE,"mpu6050_class"))){
    pr_err("Cannot create the struct class\n");
    unregister_chrdev_region(mpu6050.devid,1);
    return -1;
  }
  /*Creating struct device*/
  if(IS_ERR(mpu6050.device = device_create(mpu6050.class,NULL,mpu6050.devid,NULL,"mpu6050_device"))){
    pr_err("Cannot create the Device 1\n");
    class_destroy(mpu6050.class);
    unregister_chrdev_region(mpu6050.devid,1);
    return -1;
  }
  return 0;
}

/**
 * Function : mpu6050_initialization register 
*/
int mpu6050_initialization(void){

  uint8_t reg_val = 0;
  int ret = 0;

  //Power management register write all 0's to wake up sensor
  ret = mpu6050_i2c_write_reg(mpu6050.client, MPU6050_PWR_MGMT_1, 0x00); /*  */

  //Set data rate of 1KHz by writing SMPRT_DIV register
  ret = mpu6050_i2c_write_reg(mpu6050.client, MPU6050_SMPLRT_DIV, 0x07);

  ret = mpu6050_i2c_write_reg(mpu6050.client, MPU6050_CONFIG, 0x06);

  /* Configure the acceleration sensor to work in 16G mode, without self-test */
  ret = mpu6050_i2c_write_reg(mpu6050.client, MPU6050_ACCEL_CONFIG, 0x18);

  /* Gyroscope self-test and measurement range, typical value: 0x18 (no self-test, 2000deg/s) */
  ret = mpu6050_i2c_write_reg(mpu6050.client, MPU6050_GYRO_CONFIG, 0x18);
#if MPU6050_USE_INT 
  ret = mpu6050_i2c_read_reg(mpu6050_dev->client, MPU6050_INT_PIN_CFG, &reg_val);
  reg_val |= 0xC0;
  mpu6050_i2c_write_reg(mpu6050_dev->client, MPU6050_INT_PIN_CFG, reg_val);

  ret = mpu6050_i2c_read_reg(mpu6050_dev->client, MPU6050_INT_ENABLE, &reg_val);
  reg_val |= 0x01;
  mpu6050_i2c_write_reg(mpu6050_dev->client, MPU6050_INT_ENABLE, reg_val);
#endif
  return ret;
}
static int mpu6050_deinit(void)
{
  int ret = 0;
  /* mpu6050 reset, registers restore default values */
  ret = mpu6050_i2c_write_reg(mpu6050.client, MPU6050_PWR_MGMT_1, 0x80);
  return ret;
}

static int mpu6050_i2c_write_reg(struct i2c_client *client, uint8_t reg_addr, uint8_t data)
{
  int ret = 0; /*khởi tạo một biến ret để nhận return */
  uint8_t w_buf[2]; /*khởi tạo buffer 2 giá trị*/

  struct i2c_msg mpu6050_msg; /*khởi tạo i2c messegger chứa các giá trị như address byte ,flags*/

  w_buf[0] = reg_addr; /*biến đầu nhận địa chỉ cần ghi*/
  w_buf[1] = data;/*biến sau nhận giá trị ghi*/

  mpu6050_msg.addr = client->addr; /*copy address từ i2c_cilent sang cho gói messegger để nó thực hiện thay mình*/
  mpu6050_msg.buf = w_buf; /*copy gói buffer */
  mpu6050_msg.flags = 0; /* I2C direction ： write , để 0 là ghi 1 read*/
  mpu6050_msg.len = sizeof(w_buf); /*có 2 byte thôi nên sizeof*/

  ret = i2c_transfer(client->adapter, &mpu6050_msg, 1); /*cuối cùng gọi i2c_transfer ra để nó ghi vào, dùng adapter vì nó là bộ chuyển đổ hàm */
  if (ret < 0)
      return ret;
  else if (ret != 1)
      return -EIO;

  return 0;
}

static int mpu6050_i2c_read_reg(struct i2c_client *client, uint8_t reg_addr, uint8_t *data){
  int ret =0;
  struct i2c_msg mpu6050_msgs[2];/*nhận byte thì mình vẫn phải gửi byte write sau đó mới nhận được nên cần 2 msg*/

  /*cấu hình msg đầu để ghi*/
  mpu6050_msgs[0].addr = client->addr;
  mpu6050_msgs[0].buf = &reg_addr; /* send regsiter */
  mpu6050_msgs[0].flags = 0;       /* I2C direction ： write */
  mpu6050_msgs[0].len = 1; /*ghi mỗi địa chỉ cần đọc thôi nên là 1*/

  /*cấu hình msg để nhận giá trị*/
  mpu6050_msgs[1].addr = client->addr;
  mpu6050_msgs[1].buf = data;       /*gán địa chỉ của biến data cần ghi vào đây*/
  mpu6050_msgs[1].flags = I2C_M_RD; /* I2C direction ： read */
  mpu6050_msgs[1].len = 1;

  /*cấu hình chuyển byte*/
  ret = i2c_transfer(client->adapter, mpu6050_msgs, 2); /*2msg mà nên mình cần 2*/
  if (ret < 0)
    return ret;
  else if (ret != 2)
    return -EIO;
  return 0;
}

static int mpu6050_read_id(uint8_t *id)
{
  uint8_t data; /*khởi tạo các biến */
  int ret = 0;

  ret = mpu6050_i2c_read_reg(mpu6050.client, MPU6050_WHO_AM_I, &data);
  /*dùng để đọc ía trị mpu6050 xem đúng không thường 0xD0*/
  if (id != NULL)
    *id = data;//gán giá trị đọc được cho id

  if (data != MPU6050_IIC_ADDR) /*kiểm tra id xem có đúng id của mpu6050 không*/
    ret = -1;
  return ret;
}

static int mpu6050_read_accel(struct mpu6050_accel *acc)
{
  int i = 0;
  int ret = 0;
  uint8_t data[6] = {0};

  for (i = 0; i < 6; i++)
  {
    ret = mpu6050_i2c_read_reg(mpu6050.client, MPU6050_ACCEL_XOUT_H+i, &data[i]);
  }
  acc->x = (data[0] << 8) + data[1];
  acc->y = (data[2] << 8) + data[3];
  acc->z = (data[4] << 8) + data[5];
  return ret;
}

static int mpu6050_read_gyro(struct mpu6050_gyro *gyro)
{
  int i = 0;
  int ret = 0;
  uint8_t data[6] = {0};

  for (i = 0; i < 6; i++)
  {
      ret = mpu6050_i2c_read_reg(mpu6050.client, MPU6050_GYRO_XOUT_H+i, &data[i]);
  }
  gyro->x = (data[0] << 8) + data[1];
  gyro->y = (data[2] << 8) + data[3];
  gyro->z = (data[4] << 8) + data[5];
  return ret;
}

static int mpu6050_read_temp(int16_t *temp)
{
  int i = 0;
  int ret = 0;
  uint8_t data[2] = {0};

  for (i = 0; i < 2; i++)
  {
      ret = mpu6050_i2c_read_reg(mpu6050.client, MPU6050_TEMP_OUT_H+i, &data[i]);
  }
  *temp = (data[0] << 8) + data[1];
  return ret;
}

/**
 * @brief: Function static int mpu6050_read_temp(int16_t *temp)of i2c driver
*/
int mpu6050_probe (struct i2c_client *client, const struct i2c_device_id *id){
  printk("I'm in probe\n");
  /*if check have a id compatible it will jump here*/

  /*register device file*/
  if(mpu6050_create_df() <0){
    pr_info("Can't create device file\n");
    return -1;
  }
  /*Get client*/
  mpu6050.client = client;
  pr_info("Device Driver Insert...Done!!!\n");
  return 0;
}
int mpu6050_remove (struct i2c_client *client){
  printk("I'm in remove\n");
  device_destroy(mpu6050.class,mpu6050.devid);
  class_destroy(mpu6050.class);
  cdev_del(&mpu6050.cdev);
  unregister_chrdev_region(mpu6050.devid, 1);
  return 0;
}
/**
 * @brief: Function module init
*/
int __init mpu6050_init(void){
  int ret;
  printk("I'm in init\n");
  ret = i2c_add_driver(&mpu6050_driver);
  printk("Ret: %d\n", ret);
  if(ret <0){
    printk("Not registerd\n");
    return ret;
  }
  return 0;
}

/*function module exit*/
void __exit mpu6050_exit(void){
  printk("I'm in exit\n");
  i2c_del_driver(&mpu6050_driver);
}


module_init(mpu6050_init);
module_exit(mpu6050_exit);
/**
 * @brief: introduction module 
*/
MODULE_AUTHOR("BUI DINH HIEN");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mpu6050 a simple module");