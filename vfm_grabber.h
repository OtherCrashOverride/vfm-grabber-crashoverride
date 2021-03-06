#ifndef VFM_GRABBER_H
#define VFM_GRABBER_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>

#include <linux/amlogic/amports/vframe.h>
#include <linux/amlogic/amports/vframe_provider.h>
#include <linux/amlogic/amports/vframe_receiver.h>
#include <linux/amlogic/amports/vframe.h>

#include <linux/dma-buf.h>
#include <linux/scatterlist.h>

#include "linux/time.h"

// IOCTL defines
#define VFM_GRABBER_GET_FRAME   _IOWR('V', 0x01, vfm_grabber_frame)
#define VFM_GRABBER_GET_INFO    _IOWR('V', 0x02, vfm_grabber_info)

typedef struct
{
  int dma_fd;
  int width;
  int height;
  int stride;
  void *priv;
} vfm_grabber_frame;

typedef struct
{
  int frames_decoded;
  int frames_ready;
} vfm_grabber_info;


typedef struct
{
  int dma_fd;
  struct dma_buf* dmabuf;
} vfm_grabber_buffer;

#define MAX_DMABUF_FD 16

typedef struct
{
  struct class *device_class;
  struct device *file_device;
  struct vframe_receiver_s vfm_vf_receiver;
  int framecount;
  struct timeval starttime;
  int version_major;
  vfm_grabber_buffer buffer[MAX_DMABUF_FD];
  vfm_grabber_info info;

} vfm_grabber_dev;
#endif // VFM_GRABBER_H
