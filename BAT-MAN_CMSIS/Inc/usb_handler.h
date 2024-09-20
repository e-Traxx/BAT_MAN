#ifndef __USBH_H
#define __USBH_H

#ifdef __cplusplus
 extern "C" {
#endif

// Includes 
void USB_INIT(void);
void OTG_FS_IRQHandler();

#ifdef __cplusplus
}
#endif

#endif