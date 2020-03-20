/*
 * libmb85rs64v.c
 *
 * Small Raspberry Pi (4B) Access Library for the Adafruit SPI FRAM Breakout
 *
 * by Andreas Steinmetz, 2020
 *
 * This source is put in the public domain. Have fun!
 */

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "mb85rs64v.h"

#define	MB85RS64V_SPEED	20000000
#define MB85RS64V_MODE	SPI_MODE_0
#define MB85RS64V_BITS	8

#define MB85RS64V_WREN	0x06
#define MB85RS64V_WRDI	0x04
#define MB85RS64V_RDSR	0x05
#define MB85RS64V_WRSR	0x01
#define MB85RS64V_READ	0x03
#define MB85RS64V_WRITE	0x02
#define MB85RS64V_RDID	0x9f

int mb85rs64v_open(int bus,int device)
{
	int fd;
	uint8_t val;
	uint32_t val32;
	char bfr[32];

	snprintf(bfr,sizeof(bfr),"/dev/spidev%d.%d",bus,device);

	if((fd=open(bfr,O_RDWR))==-1)goto err1;

	val32=MB85RS64V_MODE;
	if(ioctl(fd,SPI_IOC_WR_MODE32,&val32))goto err2;

	val=MB85RS64V_BITS;
	if(ioctl(fd,SPI_IOC_WR_BITS_PER_WORD,&val))goto err2;

	val32=MB85RS64V_SPEED;
	if(ioctl(fd,SPI_IOC_WR_MAX_SPEED_HZ,&val32))goto err2;

	return fd;

err2:	close(fd);
err1:	return -1;
}

int mb85rs64v_close(int spifd)
{
	return close(spifd);
}

int mb85rs64v_get_id(int spifd,unsigned char *id)
{
	struct spi_ioc_transfer xfer[2];
	unsigned char bfr=MB85RS64V_RDID;

	memset(xfer,0,sizeof(xfer));
	xfer[0].tx_buf=(unsigned long)(&bfr);
	xfer[0].len=1;
	xfer[1].rx_buf=(unsigned long)id;
	xfer[1].len=4;

	return ioctl(spifd,SPI_IOC_MESSAGE(2),xfer)<0?-1:0;
}

int mb85rs64v_get_status(int spifd,unsigned char *status)
{
	struct spi_ioc_transfer xfer[2];
	unsigned char bfr=MB85RS64V_RDSR;

	memset(xfer,0,sizeof(xfer));
	xfer[0].tx_buf=(unsigned long)(&bfr);
	xfer[0].len=1;
	xfer[1].rx_buf=(unsigned long)status;
	xfer[1].len=1;

	return ioctl(spifd,SPI_IOC_MESSAGE(2),xfer)<0?-1:0;
}

int mb85rs64v_set_status(int spifd,unsigned char status)
{
	struct spi_ioc_transfer xfer;
	unsigned char bfr[2];

	memset(&xfer,0,sizeof(xfer));
	xfer.tx_buf=(unsigned long)(bfr);

	xfer.len=1;
	bfr[0]=MB85RS64V_WREN;
	if(ioctl(spifd,SPI_IOC_MESSAGE(1),&xfer)<0)return -1;

	xfer.len=2;
	bfr[0]=MB85RS64V_WRSR;
	bfr[1]=status;
	if(ioctl(spifd,SPI_IOC_MESSAGE(1),&xfer)<0)return -1;

	xfer.len=1;
	bfr[0]=MB85RS64V_WRDI;
	return ioctl(spifd,SPI_IOC_MESSAGE(1),&xfer)<0?-1:0;
}

int mb85rs64v_read(int spifd,unsigned int memaddr,unsigned char *data,int len)
{
	int max=sysconf(_SC_PAGESIZE)>>1;
	int n;
	struct spi_ioc_transfer xfer[2];
	unsigned char bfr[3];

	memset(xfer,0,sizeof(xfer));
	bfr[0]=MB85RS64V_READ;
	xfer[0].tx_buf=(unsigned long)(bfr);
	xfer[0].len=3;

	while(len)
	{
		n=(len>max?max:len);
		bfr[1]=(memaddr>>8)&0xff;
		bfr[2]=memaddr&0xff;
		xfer[1].rx_buf=(unsigned long)data;
		xfer[1].len=n;

		if(ioctl(spifd,SPI_IOC_MESSAGE(2),xfer)<0)return -1;

		memaddr+=n;
		data+=n;
		len-=n;
	}

	return 0;
}

int mb85rs64v_write(int spifd,unsigned int memaddr,unsigned char *data,int len)
{
	int max=sysconf(_SC_PAGESIZE)>>1;
	int n;
	int err=0;
	struct spi_ioc_transfer xfer[2];
	unsigned char bfr[3];

	memset(xfer,0,sizeof(xfer));
	xfer[0].tx_buf=(unsigned long)(bfr);

	while(len)
	{
		xfer[0].len=1;
		bfr[0]=MB85RS64V_WREN;
		if(ioctl(spifd,SPI_IOC_MESSAGE(1),xfer)<0)
		{
			err=-1;
			break;
		}

		n=(len>max?max:len);

		xfer[0].len=3;
		bfr[0]=MB85RS64V_WRITE;
		bfr[1]=(memaddr>>8)&0xff;
		bfr[2]=memaddr&0xff;
		xfer[1].tx_buf=(unsigned long)data;
		xfer[1].len=n;

		if(ioctl(spifd,SPI_IOC_MESSAGE(2),xfer)<0)
		{
			err=-1;
			break;
		}

		memaddr+=n;
		data+=n;
		len-=n;
	}

	xfer[0].len=1;
	bfr[0]=MB85RS64V_WRDI;

	if(ioctl(spifd,SPI_IOC_MESSAGE(1),xfer)<0)return -1;

	return err;
}
