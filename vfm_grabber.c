#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/of_reserved_mem.h>

// Module version
#define VERSION_MAJOR 1
#define VERSION_MINOR 0

// Names / devices and general config

#define DRIVER_NAME	"jpeghwdec"
#define MODULE_NAME     DRIVER_NAME
#define DEVICE_NAME 	DRIVER_NAME
#define DEBUG_LOGFILE 	"/storage/jpeghwdec.log"


#define MEM_INPUT_MAX_SIZE      (4 * 1024 * 1024)	// Maximum JPEG input data size  
#define MEM_OUTPUT_MAX_SIZE     (4 * 3840 * 2160)	// Maximum decoded output size (4k)

// types definition
typedef struct
{
  unsigned long start;
  unsigned long size;
  unsigned long input_start;
  unsigned long output_start;
} reserved_mem_s;

// Variables
static struct class *jpeghwdec_class;
static struct device *jpeghwdec_dev;

static reserved_mem_s reserved_mem;

//////////////////////////////////////////////////
// Log functions 
//////////////////////////////////////////////////
static struct file* logfile = NULL;	// Current logfile pointer
static loff_t log_pos = 0;		// Current logfile position


// this function writes in a user file
void debug_log(char *prefix, char *format, ...)
{
  char logstr[300];
  char fullstr[512];
  va_list args;
  mm_segment_t old_fs;

  if (!logfile)
  {
    logfile = filp_open(DEBUG_LOGFILE, O_RDWR | O_CREAT, 0644);
    if (!logfile) return;
  }

  va_start(args, format);
  vsprintf(logstr, format, args);
  sprintf(fullstr,"%s : %s", prefix, logstr);
  va_end (args);

  old_fs = get_fs();
  set_fs(KERNEL_DS);
  vfs_write(logfile, fullstr, strlen(fullstr), &log_pos);
  set_fs(old_fs);
}

// this function writes in the kernel log
void system_log(int logtype, char *prefix, char *format, ...)
{
  char logstr[300];
  char fullstr[512];
  va_list args;

  va_start(args, format);
  vsprintf(logstr, format, args);
  sprintf(fullstr,"%s : %s", prefix, logstr);
  va_end (args);

  if (logtype==0)
    pr_info(KERN_ALERT "%s: %s", prefix, fullstr);
  else
    pr_err(KERN_ALERT "%s: %s", prefix,  fullstr);
}

#define DEBUG
#ifdef DEBUG
#define log_info(fmt, arg...)  	debug_log("info", fmt, ## arg)
#define log_error(fmt, arg...) 	debug_log("error", fmt, ## arg)
#else
#define log_info(fmt, arg...) 	system_log(0, DRIVER_NAME, fmt, ## arg) 
#define log_error(fmt, arg...)  system_log(1, DRIVER_NAME, fmt, ## arg)
#endif

//////////////////////////////////////////////////
// File operations functions
//////////////////////////////////////////////////
static int jpeghwdec_mmap(struct file *filp, struct vm_area_struct *vma)
{
  int ret = 0;
  return ret;
}

static long jpeghwdec_ioctl(struct file *file, unsigned int cmd, ulong arg)
{
  int ret = 0;
  return ret;
}

static int jpeghwdec_open(struct inode *inode, struct file *file)
{
  int ret = 0;
  return ret;
}

static int jpeghwdec_release(struct inode *inode, struct file *file)
{
  int ret = 0;
  return ret;
}


static const struct file_operations jpeghwdec_fops = 
{
  .owner = THIS_MODULE,
  .open = jpeghwdec_open,
  .mmap = jpeghwdec_mmap,
  .release = jpeghwdec_release,
  .unlocked_ioctl = jpeghwdec_ioctl,
};

//////////////////////////////////////////////////
// Probe and remove functions 
//////////////////////////////////////////////////
static int jpeghwdec_probe(struct platform_device *pdev)
{
  int ret;

  // gets the work memory area
  ret = of_reserved_mem_device_init(&pdev->dev);
  if (ret == 0)
  {
     log_error("failed to retrieve reserved memory.\n");
     return -EFAULT;
  }
  log_info("reserved memory retrieved successfully.\n");

  ret = register_chrdev(VERSION_MAJOR, DEVICE_NAME, &jpeghwdec_fops);
  if (ret < 0) 
  {
     log_error("can't register major for device\n");
     return ret;
  }

  jpeghwdec_class = class_create(THIS_MODULE, DEVICE_NAME);
  if (!jpeghwdec_class)
  {
    log_error("failed to create class\n");
    return -EFAULT;
  }

  jpeghwdec_dev = device_create(jpeghwdec_class, NULL,
                                        MKDEV(VERSION_MAJOR, VERSION_MINOR),
                                        NULL, DEVICE_NAME);
  if (!jpeghwdec_dev)
  {
    log_error("failed to create device %s", DEVICE_NAME);
    return -EFAULT;
  }

  log_info("driver probed successfully\n");
  return 0;
}

static int jpeghwdec_remove(struct platform_device *pdev)
{
  device_destroy(jpeghwdec_class, MKDEV(VERSION_MAJOR, VERSION_MINOR));

  class_destroy(jpeghwdec_class);

  unregister_chrdev(VERSION_MAJOR, DEVICE_NAME);

  return 0;
}

//////////////////////////////////////////////////
// Module Init / Exit functions 
//////////////////////////////////////////////////
static const struct of_device_id jpeghwdec_dt_match[] = 
{
  {
    .compatible = "amlogic, jpeghwdec",
  },
  {},
};

static struct platform_driver jpeghwdec_driver = 
{
  .probe = jpeghwdec_probe,
  .remove = jpeghwdec_remove,
  .driver = 
  {
    .name = DRIVER_NAME,
    .owner = THIS_MODULE,
    .of_match_table = jpeghwdec_dt_match,
  }
};


static int __init jpeghwdec_init(void)
{
  if (platform_driver_register(&jpeghwdec_driver)) 
  {
    log_error("failed to register jpeghwdec module\n");
    return -ENODEV;
  }

  return 0;
}

static void __exit jpeghwdec_exit(void)
{
  platform_driver_unregister(&jpeghwdec_driver);
  return;
}

module_init(jpeghwdec_init);
module_exit(jpeghwdec_exit);

//////////////////////////////////////////////////
// Memory Setup
//////////////////////////////////////////////////
static int jpeghwdec_mem_device_init(struct reserved_mem *rmem, struct device *dev)
{
   memset(&reserved_mem, 0, sizeof(reserved_mem_s));
   reserved_mem.start = rmem->base;
   reserved_mem.size = rmem->size;
   log_info("memory resource found at %lx (%lx)\n", reserved_mem.start, reserved_mem.size);
   return 0;
}

static const struct reserved_mem_ops rmem_jpeghwdec_ops = 
{
  .device_init = jpeghwdec_mem_device_init,
};

static int __init jpeghwdec_mem_setup(struct reserved_mem *rmem)
{
  rmem->ops = &rmem_jpeghwdec_ops;
  log_info("doing share mem setup\n");
  return 0;
}

//////////////////////////////////////////////////
// Module declaration
//////////////////////////////////////////////////
RESERVEDMEM_OF_DECLARE(DRIVER_NAME, "amlogic, jpeghwdec_memory", jpeghwdec_mem_setup);
MODULE_DESCRIPTION("HW JPEG decoder driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lionel CHAZALLON <LongChair@hotmail.com>");

