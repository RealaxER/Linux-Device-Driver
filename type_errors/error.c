#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uapi.h>


#include <errno-base.h> 
// All types define error in kernel
#define	EPERM		 1	/* Operation not permitted //Không được phép hoạt động
/*permitted : được 
operation : hoạt động 
*/

#define	ENOENT		 2	/* No such file or directory */ /*Không có file và thư mục nào như vậy*/
/*such : như là , như vậy 
directory : thư mục 
*/



#define	ESRCH		 3	/* No such process */ /*khoogn có tiến trình nào như vậy*/
#define	EINTR		 4	/* Interrupted system call *//*bị gián đoạn system call*/
/*interrup : ngắt 
interruped : bị gián đoạn 
*/
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long *//*đối số quá dài*/
/*argument : lập luận , đối số
*/


#define	ENOEXEC		 8	/* Exec format error */
/*executable : thực thi
format : định dạng 
exec : giám đốc điều hành 
*/

#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */ /*không có tiến trình con*/
/*process : quá trình
processed : tiến trình 
*/
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */ /*hết bộ nhớ*/
/*out of : hết gì đó 
*/

#define	EACCES		13	/* Permission denied */
/*permission : sự cho phép
cùng nghĩa với permitted : cho phép
tuy nhiên nó dùng để làm danh tư ghép 

denied : từ chối
permission denied : quyền bị từ chối 
*/


#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World kernel module");
/*ĐỐI VỚI CÁC CON TRỎ NULL TRẢ VỀ FUNCITON TRẢ VỀ CON TRỎ
CHÚNG TA CẦN PHẢI DÙNG HÀM SUPPORT 

void *ERR_PTR(long error);
- chuyển lỗi từ biến thành một con trỏ trả về kiểu long lỗi 
example return ERR_PTR(-ENOMEM);


long IS_ERR(const void *ptr); 
//kiểm tra xem con trỏ đó có lỗi không , ví dụ như out of memory or memory null
// lỗi trả về khác 0

long PTR_ERR(const void *ptr);
// trích xuất lỗi từ con trỏ lỗi trả về
*/
int init_hello(void)
{
	printk("BuiHien Hello world 1.\n");

	/* 
	 * A non 0 return means init_module failed; module can't be loaded. 
	 */
	return 0;
}

void cleanup_hello(void)
{
	printk("BuiHien Goodbye world 1.\n");
}

/*ĐỐI VƠI*/