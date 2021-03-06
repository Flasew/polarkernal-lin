/*
 * Filename: gih.h
 * Author: Weiyang Wang
 * Description: header file for the generic interrupt handler kernel module. 
 *              Declares several structures, functions, and constants that will
 *              be used in the module.
 * Date: Jul 20, 2017
 */

#ifndef _GIH_H
#define _GIH_H

MODULE_LICENSE("Dual BSD/GPL");

#define TRUE 1
#define FALSE 0

#define DEBUG 1

/* device names */
#define GIH_DEV         "gih"       /* device that accepts user input */
#define LOG_DEV         "gihlog"    /* logging device for interrupt happen */
#define LOG_DEV_FMT     "gihlog%d"  /* formatted version */

/* minor number for all logging devices */
#define INTR_LOG_MINOR 0
#define WQ_N_LOG_MINOR 1
#define WQ_X_LOG_MINOR 2

/* gih ioctl */
#define GIH_IOC 'G'
/*
 * ioctl operations are for gih configuration.
 */
#define GIH_IOC_CONFIG_IRQ      _IOW(GIH_IOC, 1, int) 
#define GIH_IOC_CONFIG_DELAY_T  _IOW(GIH_IOC, 2, unsigned int) 
#define GIH_IOC_CONFIG_WRT_SZ   _IOW(GIH_IOC, 3, size_t) 
#define GIH_IOC_CONFIG_PATH     _IOW(GIH_IOC, 4, const char *)
#define GIH_IOC_CONFIG_START    _IO (GIH_IOC, 5)
#define GIH_IOC_CONFIG_STOP     _IO (GIH_IOC, 6)
#define GIH_IOC_CONFIG_MISS     _IOW(GIH_IOC, 7, int)

/* individual log, contains a timespec and a irq identifier */
struct log {
    ssize_t byte_sent;              /* number of bytes sent this time
                                       only set by wqlogx device */
    unsigned long irq_count;        /* irq identifier */
    struct timeval time;            /* time of the log */
};

/* FIFO buffer for logging devices */
#define LOG_FIFO_SZ 8192                        /* buffer size of FIFO */
#define LOG_STR_BUF_SZ 256                      /* max len for log string */
DEFINE_KFIFO(ilog_buf, struct log, LOG_FIFO_SZ);    
DEFINE_KFIFO(wq_n_buf, struct log, LOG_FIFO_SZ);
DEFINE_KFIFO(wq_x_buf, struct log, LOG_FIFO_SZ);

/* log device structure */
typedef struct log_dev {
    unsigned long irq_count;        /* total number of irq caught; in N & X
                                       devices they represent number of 
                                       not missed irq */
    dev_t dev_num;                  /* device number */
    struct kfifo * buffer;          /* FIFO buffer */
    struct class * log_class;       /* for sysfs, log class */
    struct device * log_device;     /* for sysfs, log device */
    struct mutex dev_open;          /* device can only open once a time*/
} log_dev;

/* gih device structure */
#define DATA_FIFO_SZ (1<<20)        /* 1MB */
DECLARE_KFIFO(data_buf, unsigned char, DATA_FIFO_SZ);

#define IRQ_NAME "gih irq handler"
#define IRQ_WQ_NAME "irq work queue"
#define PATH_MAX_LEN 128            /* Just a file name... should be enough */

#define TIME_DELTA 200               /* time correction value, wait time will
                                       be reduced by this TIME_DELTA microsec
                                       to account for internal delays */ 

typedef struct gih_dev {
    bool setup;                        /* if the device has been setup */
    bool keep_missed;                  /* keep the missing data on write? */
    int irq;                           /* irq line to be registered */
    unsigned int sleep_msec;           /* time to sleep */
    size_t write_size;                 /* how much to write each time */
    dev_t dev_num;                     /* device number */
    struct workqueue_struct * irq_wq;  /* work queue */
    struct file * dest_filp;           /* destination file pointer */
    struct class * gih_class;          /* for sysfs, class */
    struct device * gih_device;        /* for sysfs, device */
    atomic_t data_wait;                /* number of data on wait */
    struct work_struct work;           /* work to be put in the queue */
    struct mutex dev_open;             /* dev can only be opening once */
    struct mutex wrt_lock;             /* mutex to protect write to file */
    struct cdev gih_cdev;              /* gih char device */
    struct cdev log_cdev;              /* log char device */
    struct kfifo data_buf;             /* buffer of data */
    char path[PATH_MAX_LEN];           /* destination file path */
} gih_dev;

#endif
