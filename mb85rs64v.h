/*
 * mb85rs64v.h
 *
 * Small Raspberry Pi (4B) Access Library for the Adafruit SPI FRAM Breakout
 *
 * by Andreas Steinmetz, 2020
 *
 * This source is put in the public domain. Have fun!
 */

#ifndef MB85RS64V_H_INCLUDED
#define MB85RS64V_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* access SPI, for RPi bus is always 0 and device is 0 or 1 for CE0 or CE1 */

extern int mb85rs64v_open(int bus,int device);

/* terminate SPI access */

extern int mb85rs64v_close(int spifd);

/* read 4 identification bytes from MB85RS64V */

extern int mb85rs64v_get_id(int spifd,unsigned char *id);

/* read MB85RS64V status register */

extern int mb85rs64v_get_status(int spifd,unsigned char *status);

/* write MB85RS64V status register */

extern int mb85rs64v_set_status(int spifd,unsigned char status);

/* read data from MB85RS64V */

extern int mb85rs64v_read(int spifd,unsigned int memaddr,unsigned char *data,
	int len);

/* write data to MB85RS64V */

extern int mb85rs64v_write(int spifd,unsigned int memaddr,unsigned char *data,
	int len);

#ifdef __cplusplus
}
#endif

#endif
