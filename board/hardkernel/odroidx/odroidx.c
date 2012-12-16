/*
 * Copyright (C) 2012-2013 Hardkernel Co.,Ltd.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <i2c.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/sromc.h>
#include <asm/arch/power.h>
#include "setup.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("\nBoard: " CONFIG_BOARDNAME "\n");
	return 0;
}
#endif

int board_init(void)
{
	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100UL);

	/* Put PS-Hold high */
	writel(0x5300, EXYNOS4_POWER_BASE + 0x330c);

	return 0;
}

int dram_init(void)
{
	gd->ram_size	= get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_3, PHYS_SDRAM_3_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_4, PHYS_SDRAM_4_SIZE);

#ifdef CONFIG_RESERVED_DRAM
	gd->ram_size -= CONFIG_RESERVED_DRAM;
#endif

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((long *)PHYS_SDRAM_1,
							PHYS_SDRAM_1_SIZE);
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((long *)PHYS_SDRAM_2,
							PHYS_SDRAM_2_SIZE);
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = get_ram_size((long *)PHYS_SDRAM_3,
							PHYS_SDRAM_3_SIZE);
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = get_ram_size((long *)PHYS_SDRAM_4,
							PHYS_SDRAM_4_SIZE);

#ifdef CONFIG_RESERVED_DRAM
	gd->bd->bi_dram[CONFIG_NR_DRAM_BANKS - 1].size -= CONFIG_RESERVED_DRAM;
#endif
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	struct exynos4_gpio_part2 *gpio =
		(struct exynos4_gpio_part2 *)samsung_get_base_gpio_part2();
	int err;

	/* mmc0	 : eMMC (8-bit buswidth) */
	err = exynos_pinmux_config(PERIPH_ID_SDMMC4, PINMUX_FLAG_8BIT_MODE);
	if (err)
		debug("SDMMC4 not configured\n");

	/* eMMC_EN: SD_0_CDn: GPK0[2] Output High */
	s5p_gpio_direction_output(&gpio->k0, 2, 0);
	s5p_gpio_set_pull(&gpio->k0, 2, GPIO_PULL_NONE);

	/* mmc2	 : SD card (4-bit buswidth) */
	err = exynos_pinmux_config(PERIPH_ID_SDMMC2, PINMUX_FLAG_NONE);
	if (err) {
		debug("SDMMC2 not configured\n");
		return err;
	}

	return s5p_mmc_init(2, 4);
}
#endif

static int board_uart_init(void)
{
	int err;

	err = exynos_pinmux_config(PERIPH_ID_UART, PINMUX_FLAG_NONE);
	if (err)
		debug("UART%d not configured\n",
				PERIPH_ID_UART - PERIPH_ID_UART0);
	return err;
}

#ifdef CONFIG_SYS_I2C_INIT_BOARD
static int board_i2c_init(void)
{
	int i, err;

	for (i = 0; i < CONFIG_MAX_I2C_NUM; i++) {
		err = exynos_pinmux_config((PERIPH_ID_I2C0 + i),
						PINMUX_FLAG_NONE);
		if (err) {
			debug("I2C%d not configured\n", (PERIPH_ID_I2C0 + i));
			return err;
		}
	}
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	return 0;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	int err;

	err = board_uart_init();
	if (err) {
		debug("UART init failed\n");
		return err;
	}

#ifdef CONFIG_SYS_I2C_INIT_BOARD
	err = board_i2c_init();
#endif
	return err;
}
#endif 
