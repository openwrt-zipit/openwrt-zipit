/*
 * Platform independend driver for JZ4740.
 *
 * Copyright (c) 2007 Ingenic Semiconductor Inc.
 * Author: <jlwei@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#include <common.h>

#if defined(CONFIG_CMD_NAND)
#include <nand.h>
#include <asm/io.h>
#include <asm/jz4740.h>

#define __nand_ecc_enable()		(REG_EMC_NFECR = EMC_NFECR_ECCE | EMC_NFECR_ERST )
#define __nand_ecc_disable()		(REG_EMC_NFECR &= ~EMC_NFECR_ECCE)

#define JZ_NAND_DATA_ADDR ((void __iomem *)0xB8000000)
#define JZ_NAND_CMD_ADDR (JZ_NAND_DATA_ADDR + 0x8000)
#define JZ_NAND_ADDR_ADDR (JZ_NAND_DATA_ADDR + 0x10000)

#define BIT(x) (1 << (x))
#define JZ_NAND_ECC_CTRL_ENCODING	BIT(3)
#define JZ_NAND_ECC_CTRL_RS		BIT(2)
#define JZ_NAND_ECC_CTRL_RESET		BIT(1)
#define JZ_NAND_ECC_CTRL_ENABLE		BIT(0)

static struct nand_ecclayout qi_lb60_ecclayout_2gb = {
	.eccbytes = 72,
	.eccpos = {
		12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27, 
		28, 29, 30, 31, 32, 33, 34, 35,
		36, 37, 38, 39, 40, 41, 42, 43,
		44, 45, 46, 47, 48, 49, 50, 51, 
		52, 53, 54, 55, 56, 57, 58, 59, 
		60, 61, 62, 63, 64, 65, 66, 67, 
		68, 69, 70, 71, 72, 73, 74, 75, 
		76, 77, 78, 79, 80, 81, 82, 83},
	.oobfree = {
 		{.offset = 2,
		 .length = 10},
		{.offset = 84,
		 .length = 44}}
};

static int is_reading;

static void jz_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		if (ctrl & NAND_ALE)
			this->IO_ADDR_W = JZ_NAND_ADDR_ADDR;
		else if (ctrl & NAND_CLE)
			this->IO_ADDR_W = JZ_NAND_CMD_ADDR;
		else
			this->IO_ADDR_W = JZ_NAND_DATA_ADDR;

		if (ctrl & NAND_NCE)
			REG_EMC_NFCSR |= EMC_NFCSR_NFCE1; 
		else
			REG_EMC_NFCSR &= ~EMC_NFCSR_NFCE1;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

static int jz_nand_device_ready(struct mtd_info *mtd)
{
	udelay(20);
	return (REG_GPIO_PXPIN(2) & 0x40000000) ? 1 : 0;
}

void board_nand_select_device(struct nand_chip *nand, int chip)
{
	/*
	 * Don't use "chip" to address the NAND device,
	 * generate the cs from the address where it is encoded.
	 */
}

static int jz_nand_rs_calculate_ecc(struct mtd_info* mtd, const u_char* dat,
				u_char* ecc_code)
{
	uint32_t reg, status;
	int i;
	volatile u8 *paraddr = (volatile u8 *)EMC_NFPAR0;

	if(is_reading)
		return 0;

	do {
		status = REG_EMC_NFINTS;
	} while(!(status & EMC_NFINTS_ENCF));

	__nand_ecc_disable();

	for(i = 0; i < 9; i++)
		ecc_code[i] = *(paraddr + i);

	return 0;
}

static void jz_nand_hwctl(struct mtd_info* mtd, int mode)
{
	uint32_t reg;
	REG_EMC_NFINTS = 0;
	reg = REG_EMC_NFECR;
	reg |= JZ_NAND_ECC_CTRL_RESET;
	reg |= JZ_NAND_ECC_CTRL_ENABLE;
	reg |= JZ_NAND_ECC_CTRL_RS;

	switch(mode) {
	case NAND_ECC_READ:
		reg &= ~JZ_NAND_ECC_CTRL_ENCODING;
		is_reading = 1;
		break;
	case NAND_ECC_WRITE:
		reg |= JZ_NAND_ECC_CTRL_ENCODING;
		is_reading = 0;
		break;
	default:
		break;
	}

	REG_EMC_NFECR = reg;
}

/* Correct 1~9-bit errors in 512-bytes data */
static void jz_rs_correct(unsigned char *dat, int idx, int mask)
{
	int i;

	idx--;

	i = idx + (idx >> 3);
	if (i >= 512)
		return;

	mask <<= (idx & 0x7);

	dat[i] ^= mask & 0xff;
	if (i < 511)
		dat[i+1] ^= (mask >> 8) & 0xff;
}

static int jz_nand_rs_correct_data(struct mtd_info *mtd, u_char *dat,
				   u_char *read_ecc, u_char *calc_ecc)
{
	int k;
	uint32_t reg, status;
	volatile u8 *paraddr = (volatile u8 *)EMC_NFPAR0;

	/* Set PAR values */
	static uint8_t all_ff_ecc[] = {0xcd, 0x9d, 0x90, 0x58, 0xf4, 0x8b, 0xff, 0xb7, 0x6f};
	if (read_ecc[0] == 0xff && 
	    read_ecc[1] == 0xff &&
	    read_ecc[2] == 0xff &&
	    read_ecc[3] == 0xff &&
	    read_ecc[4] == 0xff &&
	    read_ecc[5] == 0xff &&
	    read_ecc[6] == 0xff &&
	    read_ecc[7] == 0xff &&
	    read_ecc[8] == 0xff) {
		for (k = 0; k < 9; k++)
			*(paraddr + k) = all_ff_ecc[k];
	} else {

		for (k = 0; k < 9; k++)
			*(paraddr + k) = read_ecc[k];
	}
	/* Set PRDY */
	REG_EMC_NFECR |= EMC_NFECR_PRDY;

	/* Wait for completion */
	do {
		status = REG_EMC_NFINTS;
	} while (!(status & EMC_NFINTS_DECF));

	__nand_ecc_disable();

	/* Check decoding */
	if (status & EMC_NFINTS_ERR) {
		if (status & EMC_NFINTS_UNCOR) {
			printk("uncorrectable ecc\n");
			return -1;
		}

		uint32_t errcnt = (status & EMC_NFINTS_ERRCNT_MASK) >> EMC_NFINTS_ERRCNT_BIT;
		switch (errcnt) {
		case 4:
			jz_rs_correct(dat, 
				      (REG_EMC_NFERR3 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, 
				      (REG_EMC_NFERR3 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);
		case 3:
			jz_rs_correct(dat, 
				      (REG_EMC_NFERR2 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, 
				      (REG_EMC_NFERR2 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);
		case 2:
			jz_rs_correct(dat, 
				      (REG_EMC_NFERR1 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, 
				      (REG_EMC_NFERR1 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);
		case 1:
			jz_rs_correct(dat, 
				      (REG_EMC_NFERR0 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT, 
				      (REG_EMC_NFERR0 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT);
			return errcnt;
		default:
			break;
		}
	}

	return 0;
}

/*
 * Main initialization routine
 */
int board_nand_init(struct nand_chip *nand)
{
	/* EMC setup, Set NFE bit */
	REG_EMC_NFCSR |= EMC_NFCSR_NFE1;
	REG_EMC_SMCR1 = 0x094c4400;
	/* REG_EMC_SMCR3 = 0x04444400; */

	nand->IO_ADDR_R		= JZ_NAND_DATA_ADDR;
	nand->IO_ADDR_W		= JZ_NAND_DATA_ADDR;
	nand->cmd_ctrl		= jz_nand_cmd_ctrl;
	nand->dev_ready		= jz_nand_device_ready;

	nand->ecc.hwctl		= jz_nand_hwctl;
	nand->ecc.correct	= jz_nand_rs_correct_data;
	nand->ecc.calculate	= jz_nand_rs_calculate_ecc;
	nand->ecc.mode		= NAND_ECC_HW_OOB_FIRST;
	nand->ecc.size		= CONFIG_SYS_NAND_ECCSIZE;
	nand->ecc.bytes		= CONFIG_SYS_NAND_ECCBYTES;
	nand->ecc.layout	= &qi_lb60_ecclayout_2gb;
	nand->chip_delay	= 50;

	return 0;
}
#endif /* (CONFIG_CMD_NAND) */
