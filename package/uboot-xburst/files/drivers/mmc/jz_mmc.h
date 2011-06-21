/*
 *  linux/drivers/mmc/jz_mmc.h
 *
 *  Author: Vladimir Shebordaev, Igor Oblakov
 *  Copyright:  MontaVista Software Inc.
 *
 *  $Id: jz_mmc.h,v 1.3 2007-06-15 08:04:20 jlwei Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef __MMC_JZMMC_H__
#define __MMC_JZMMC_H__

#define ID_TO_RCA(x) ((x)+1)
#define MMC_OCR_ARG		0x00ff8000	/* Argument of OCR */

/* Standard MMC/SD clock speeds */
#define MMC_CLOCK_SLOW    400000      /* 400 kHz for initial setup */
#define MMC_CLOCK_FAST  20000000      /* 20 MHz for maximum for normal operation */
#define SD_CLOCK_FAST   24000000      /* 24 MHz for SD Cards */

/* Use negative numbers to disambiguate */
#define MMC_CIM_RESET            -1
#define SET_BUS_WIDTH            6    /* ac   [1:0] bus width    R1  */    

#define R1_OUT_OF_RANGE		(1 << 31)	/* er, c */
#define R1_ADDRESS_ERROR	(1 << 30)	/* erx, c */
#define R1_BLOCK_LEN_ERROR	(1 << 29)	/* er, c */
#define R1_ERASE_SEQ_ERROR      (1 << 28)	/* er, c */
#define R1_ERASE_PARAM		(1 << 27)	/* ex, c */
#define R1_WP_VIOLATION		(1 << 26)	/* erx, c */
#define R1_CARD_IS_LOCKED	(1 << 25)	/* sx, a */
#define R1_LOCK_UNLOCK_FAILED	(1 << 24)	/* erx, c */
#define R1_COM_CRC_ERROR	(1 << 23)	/* er, b */
#define R1_ILLEGAL_COMMAND	(1 << 22)	/* er, b */
#define R1_CARD_ECC_FAILED	(1 << 21)	/* ex, c */
#define R1_CC_ERROR		(1 << 20)	/* erx, c */
#define R1_ERROR		(1 << 19)	/* erx, c */
#define R1_UNDERRUN		(1 << 18)	/* ex, c */
#define R1_OVERRUN		(1 << 17)	/* ex, c */
#define R1_CID_CSD_OVERWRITE	(1 << 16)	/* erx, c, CID/CSD overwrite */
#define R1_WP_ERASE_SKIP	(1 << 15)	/* sx, c */
#define R1_CARD_ECC_DISABLED	(1 << 14)	/* sx, a */
#define R1_ERASE_RESET		(1 << 13)	/* sr, c */
#define R1_STATUS(x)            (x & 0xFFFFE000)

#define MMC_CARD_BUSY	0x80000000	/* Card Power up status bit */

#define MMC_PROGRAM_CID          26   /* adtc                    R1  */
#define MMC_PROGRAM_CSD          27   /* adtc                    R1  */

#define MMC_GO_IRQ_STATE         40   /* bcr                     R5  */
#define MMC_GEN_CMD              56   /* adtc [0] RD/WR          R1b */
#define MMC_LOCK_UNLOCK          42   /* adtc                    R1b */
#define MMC_WRITE_DAT_UNTIL_STOP 20   /* adtc [31:0] data addr   R1  */
#define MMC_READ_DAT_UNTIL_STOP  11   /* adtc [31:0] dadr        R1  */
#define MMC_SEND_WRITE_PROT      30   /* adtc [31:0] wpdata addr R1  */


enum mmc_result_t {
	MMC_NO_RESPONSE        = -1,
	MMC_NO_ERROR           = 0,
	MMC_ERROR_OUT_OF_RANGE,
	MMC_ERROR_ADDRESS,
	MMC_ERROR_BLOCK_LEN,
	MMC_ERROR_ERASE_SEQ,
	MMC_ERROR_ERASE_PARAM,
	MMC_ERROR_WP_VIOLATION,
	MMC_ERROR_CARD_IS_LOCKED,
	MMC_ERROR_LOCK_UNLOCK_FAILED,
	MMC_ERROR_COM_CRC,
	MMC_ERROR_ILLEGAL_COMMAND,
	MMC_ERROR_CARD_ECC_FAILED,
	MMC_ERROR_CC,
	MMC_ERROR_GENERAL,
	MMC_ERROR_UNDERRUN,
	MMC_ERROR_OVERRUN,
	MMC_ERROR_CID_CSD_OVERWRITE,
	MMC_ERROR_STATE_MISMATCH,
	MMC_ERROR_HEADER_MISMATCH,
	MMC_ERROR_TIMEOUT,
	MMC_ERROR_CRC,
	MMC_ERROR_DRIVER_FAILURE,
};

enum card_state {
	CARD_STATE_EMPTY = -1,
	CARD_STATE_IDLE	 = 0,
	CARD_STATE_READY = 1,
	CARD_STATE_IDENT = 2,
	CARD_STATE_STBY	 = 3,
	CARD_STATE_TRAN	 = 4,
	CARD_STATE_DATA	 = 5,
	CARD_STATE_RCV	 = 6,
	CARD_STATE_PRG	 = 7,
	CARD_STATE_DIS	 = 8,
};

enum mmc_rsp_t {
	RESPONSE_NONE   = 0,
	RESPONSE_R1     = 1,
	RESPONSE_R1B    = 2,
	RESPONSE_R2_CID = 3,
	RESPONSE_R2_CSD  = 4,
	RESPONSE_R3      = 5,
	RESPONSE_R4      = 6,
	RESPONSE_R5      = 7,
        RESPONSE_R6      = 8,
};

struct mmc_response_r1 {
	u8  cmd;
	u32 status;
};

struct mmc_response_r3 {  
	u32 ocr;
}; 

/* the information structure of MMC/SD Card */
struct  mmc_info {
	int             id;     /* Card index */
        int             sd;     /* MMC or SD card */
        int             rca;    /* RCA */
        u32             scr;    /* SCR 63:32*/
	int             flags;  /* Ejected, inserted */
	enum card_state state;  /* empty, ident, ready, whatever */

	/* Card specific information */
	struct mmc_cid  cid;
	struct mmc_csd  csd;
	u32             block_num;
	u32             block_len;
	u32             erase_unit;
};

struct mmc_info mmcinfo;

struct mmc_request {
	int               index;      /* Slot index - used for CS lines */
	int               cmd;        /* Command to send */
	u32               arg;        /* Argument to send */
	enum mmc_rsp_t    rtype;      /* Response type expected */

	/* Data transfer (these may be modified at the low level) */
	u16               nob;        /* Number of blocks to transfer*/
	u16               block_len;  /* Block length */
	u8               *buffer;     /* Data buffer */
	u32               cnt;        /* Data length, for PIO */

	/* Results */
	u8                response[18]; /* Buffer to store response - CRC is optional */
	enum mmc_result_t result;
};

char * mmc_result_to_string(int);
int    mmc_unpack_csd(struct mmc_request *request, struct mmc_csd *csd);
int    mmc_unpack_r1(struct mmc_request *request, struct mmc_response_r1 *r1, enum card_state state);
int    mmc_unpack_r6(struct mmc_request *request, struct mmc_response_r1 *r1, enum card_state state, int *rca);
int    mmc_unpack_scr(struct mmc_request *request, struct mmc_response_r1 *r1, enum card_state state, u32 *scr);
int    mmc_unpack_cid(struct mmc_request *request, struct mmc_cid *cid);
int    mmc_unpack_r3(struct mmc_request *request, struct mmc_response_r3 *r3);

void   mmc_send_cmd(struct mmc_request *request, int cmd, u32 arg, 
		     u16 nob, u16 block_len, enum mmc_rsp_t rtype, u8 *buffer);
u32    mmc_tran_speed(u8 ts);
void   jz_mmc_set_clock(int sd, u32 rate);

static inline void mmc_simple_cmd(struct mmc_request *request, int cmd, u32 arg, enum mmc_rsp_t rtype)
{
	mmc_send_cmd( request, cmd, arg, 0, 0, rtype, 0);
}

#endif /* __MMC_JZMMC_H__ */
