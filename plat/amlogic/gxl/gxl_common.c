/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/ep_info.h>
#include <bl31/interrupt_mgmt.h>
#include <meson_console.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <stdint.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

/*******************************************************************************
 * Platform memory map regions
 ******************************************************************************/
#define MAP_NSDRAM0	MAP_REGION_FLAT(AML_NSDRAM0_BASE,		\
					AML_NSDRAM0_SIZE,		\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_NSDRAM1	MAP_REGION_FLAT(AML_NSDRAM1_BASE,		\
					AML_NSDRAM1_SIZE,		\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_SEC_DEVICE0	MAP_REGION_FLAT(AML_SEC_DEVICE0_BASE,		\
					AML_SEC_DEVICE0_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_SEC_DEVICE1	MAP_REGION_FLAT(AML_SEC_DEVICE1_BASE,		\
					AML_SEC_DEVICE1_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_TZRAM	MAP_REGION_FLAT(AML_TZRAM_BASE,		\
					AML_TZRAM_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_SEC_DEVICE2	MAP_REGION_FLAT(AML_SEC_DEVICE2_BASE,		\
					AML_SEC_DEVICE2_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_SEC_DEVICE3	MAP_REGION_FLAT(AML_SEC_DEVICE3_BASE,		\
					AML_SEC_DEVICE3_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

static const mmap_region_t gxl_mmap[] = {
	MAP_NSDRAM0,
	MAP_NSDRAM1,
	MAP_SEC_DEVICE0,
	MAP_SEC_DEVICE1,
	MAP_TZRAM,
	MAP_SEC_DEVICE2,
	MAP_SEC_DEVICE3,
	{0}
};

/*******************************************************************************
 * Per-image regions
 ******************************************************************************/
#define MAP_BL31	MAP_REGION_FLAT(BL31_BASE,			\
				BL31_END - BL31_BASE,			\
				MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_BL_CODE	MAP_REGION_FLAT(BL_CODE_BASE,			\
				BL_CODE_END - BL_CODE_BASE,		\
				MT_CODE | MT_SECURE)

#define MAP_BL_RO_DATA	MAP_REGION_FLAT(BL_RO_DATA_BASE,		\
				BL_RO_DATA_END - BL_RO_DATA_BASE,	\
				MT_RO_DATA | MT_SECURE)

#define MAP_BL_COHERENT	MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,		\
				BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE, \
				MT_DEVICE | MT_RW | MT_SECURE)

/*******************************************************************************
 * Function that sets up the translation tables.
 ******************************************************************************/
void aml_setup_page_tables(void)
{
#if IMAGE_BL31
	const mmap_region_t gxl_bl_mmap[] = {
		MAP_BL31,
		MAP_BL_CODE,
		MAP_BL_RO_DATA,
#if USE_COHERENT_MEM
		MAP_BL_COHERENT,
#endif
		{0}
	};
#endif

	mmap_add(gxl_bl_mmap);

	mmap_add(gxl_mmap);

	init_xlat_tables();
}

/*******************************************************************************
 * Function that sets up the console
 ******************************************************************************/
static console_meson_t gxl_console;

void aml_console_init(void)
{
	int rc = console_meson_register(AML_UART0_AO_BASE,
					AML_UART0_AO_CLK_IN_HZ,
					AML_UART_BAUDRATE,
					&gxl_console);
	if (rc == 0) {
		/*
		 * The crash console doesn't use the multi console API, it uses
		 * the core console functions directly. It is safe to call panic
		 * and let it print debug information.
		 */
		panic();
	}

	console_set_scope(&gxl_console.console,
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);
}

/*******************************************************************************
 * Function that returns the system counter frequency
 ******************************************************************************/
unsigned int plat_get_syscnt_freq2(void)
{
	uint32_t val;

	val = mmio_read_32(AML_SYS_CPU_CFG7);
	val &= 0xFDFFFFFF;
	mmio_write_32(AML_SYS_CPU_CFG7, val);

	val = mmio_read_32(AML_AO_TIMESTAMP_CNTL);
	val &= 0xFFFFFE00;
	mmio_write_32(AML_AO_TIMESTAMP_CNTL, val);

	return AML_OSC24M_CLK_IN_HZ;
}
