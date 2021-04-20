/*
 * drivers/net/dm9051.c
 * Copyright 2017 Davicom Semiconductor,Inc.
 *
 * 	This program is free software; you can redistribute it and/or
 * 	modify it under the terms of the GNU General Public License
 * 	as published by the Free Software Foundation; either version 2
 * 	of the License, or (at your option) any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * (C) Copyright 2017-2021 DAVICOM Semiconductor,Inc. All Rights Reserved.
 *  V2.0 - 20190829, Support DTS usage, Support Interrupt.
 *  V2.1 - 20190910, Support Non-DTS usage and Interrupt. 
 *  (Still not tesed for Polling. MTK cpu support is to be added.) 
 *  V2.2 - 20190912, Add spi_setup, add POLLING support
 *  (Test ... save file test ...and test ...)
 *  (Test ... save file test ...and test2 ...)
 *  V2.2xc - 20190924, Support ASR 1024 buf limitation.
 *         - With the tested log which is result-good with few RXBErr and fewer macErr.  
 *  V2.2yc - 20190929, Support ASR 0 buf limitation.
 *         - Verify with Module Mode, NON-DTS, INT OK.
 *         - Verify with Module Mode, NON-DTS, Polling OK.
 *         - Verify with Module Mode, DTS, INT OK.
 *         - Verify with Module Mode, DTS, Polling OK.
 *         - Verify DM_CONF_MAX_SPEED_HZ	individually as 15600000 and 31200000 OK.
 *  V2.2zc - 20191015, Test.Good
 *  V2.2zcd.beta- 20191023, Add Supp Conf SPI dma yes
 *  V2.2zcd - 20191024, test Supp Conf SPI dma yes, OK
 *  V2.2zcd_R1 - 20191105, make 5 major macros in 'conf_ver.h'
 *  V2.2zcd_R2b - 20191108, put spi fer/msg in 'board_info.h'
 *  V2.2zcd_R2b2_savecpu2 - 20191122, save cpu in polling mode
 *  V2.2zcd_R2b2_savecpu3i - 20191126, Correcting interrupt mode ('int_get_attribute' & 'int_get_begin')
 *  V2.2zcd_R2b2_savecpu3i2 - 20191126, Str[]
 *  V2.2zcd_R2b2_savecpu5i - 20191128, Interrupt & sched
 *  V2.2zcd_R2b2_savecpu5i2p_xTsklet_thread - 20191211, Tasklet Not Suitable, But request_threaded_irq()
 *  V2.2zcd_R2b2_savecpu5i2p_xTsklet1 - 20191213, ASL_RSRV_RX_HDR_LEN                                                              
 *  V2.2zcd_R2b2_savecpu5i2p_xTsklet1p - 20191215 code-format
 *  V2.2zcd_R2b2_savecpu5i2p_xTsklet2p - 20191218 RXB explore
 *  V2.2zcd_R2b2_savecpu5i2p_xTsklet3p - 20191218 MTK MT6762       
 *  V2.2zcd_R2b2_savecpu5i3p_xTsklet3p - 20191218 RX CHECK & mtu   
 *  V2.2zcd_R2b2_savecpu5i3p_xTsklet5p - 20191220 Quick process 1516_issue, Release mode (ON_RELEASE) support,  
 *                                       Create "conf_rev.h" which is for customization purpose~
 *  V2.2zcd_tsklet_savecpu_5pt - 20191227 Make the SCAN_LEN (also SCAN_LEN_HALF) as 98 KB (conf_ver.h)
 *                                       To improve the RX procedure, Also well analysis everything in RX.
 *  V2.2zcd_tsklet_savecpu_5pt_Jabber - 20200121 Make Jabber support
 *  V2.2zcd_tsklet_savecpu_5pt_JabberP - 20200206 Add rx errors, rx frames statistics
 *  V2.2zcd_tsklet_savecpu_5pt_JabberP_PM - 20200331 PM is for suspend/resume function
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/cache.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include <asm/delay.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <linux/spi/spi.h>
extern int spi_register_board_info(struct spi_board_info const *info, unsigned n); //had been. #define CONFIG_SPI, used in local static-make for dm9051.o

#include "dm9051.h"

#include "conf_ver.h"

/*#include "conf_rev.h"
#include "main/def_generation.h"
#include "main/def_const_opt.h"
#include "main/LOOP_dir/board_info.h"
#include "main/SUBB_dir/sub_dm9051.h"         
	#include "main/printnb.c"
	#include "main/LOOP_dir/board_info.c"		// re_pos
	#include "SPI/spi_dm9051.c"			//[lib/SPI_dir/spi_rw.c]
	#include "SPI/dma_spi_dm9051.c"			//[lib/SPI_dir/spi_rw.c]
	//#include "main/LOOP_dir/board_info.c"
	#include "main/SUBB_dir/spi_user.c"             
	//#include "main/SUB_dir/mutex_dm9051.c"	//[#if DEF_SPIRW] [#endif], all spi read/write dm9051
	//#include "main/SUB_dir/appinline_dm9051.c"	//[#if DM_CONF_APPSRC] [#endif]
	#define RXM_WrtPTR	0			// Write Read RX FIFO Buffer Writer Pointer
	#define MD_ReadPTR	1			// Write Read RX FIFO Buffer Read Pointer
	#include "main/LOOP_dir/skb_rx_head.c"
	#include "main/LOOP_dir/skb_rx_core.c"
	//#include "main/LOOP_dir/skb_rx_core_old.c"
	#include "main/LOOP_dir/skb_rx.c"		//[#if DM_CONF_APPSRC] [#endif], renamed, delayed_wrok_rx.c <----- [appsrc_user.c]
	#include "main/LOOP_dir/sched.c"
	#include "main/INT_dir/int_dm9051.c"
#if DM_CONF_APPSRC
#include "main/SUBB_dir/sub_dm9051.c"
#endif

#if DM_CONF_DBGSRC
#include "main/SUBB_dir/dbgsrc_dm9051.c"
#endif
#if DM_CONF_DTSSRC
#endif

#include "main/CUST_dir/custom_gpio_dm9051.c"*/

#include "def_kt.h" 
#include "def_board.h"
#include "def_generation.h"
#include "def_const_opt.h"
#include "board_info.h"
#include "sub_dm9051.h"         

	#include "printnb.c"
	#include "board_info.c"		// re_pos
	#include "spi_dm9051.c"			//[lib/SPI_dir/spi_rw.c]
	#include "dma_spi_dm9051.c"			//[lib/SPI_dir/spi_rw.c]
	//#include "board_info.c"
	#include "spi_user.c"             
	//#include "mutex_dm9051.c"	//[#if DEF_SPIRW] [#endif], all spi read/write dm9051
	//#include "appinline_dm9051.c"	//[#if DM_CONF_APPSRC] [#endif]
	#define RXM_WrtPTR	0			// Write Read RX FIFO Buffer Writer Pointer
	#define MD_ReadPTR	1			// Write Read RX FIFO Buffer Read Pointer
	#include "skb_rx_head.c"
	#include "skb_rx_core.c"
	//#include "skb_rx_core_old.c"
	#include "skb_rx.c"		//[#if DM_CONF_APPSRC] [#endif], renamed, delayed_wrok_rx.c <----- [appsrc_user.c]
	#include "sched.c"
	#include "int_dm9051.c"
#if DM_CONF_APPSRC
#include "sub_dm9051.c"
#endif
#if DM_CONF_DBGSRC
#include "dbgsrc_dm9051.c"
#endif
#include "custom_gpio_dm9051.c"

//void conf_spi_print(struct spi_device *spi);
/*void read_intcr_print(board_info_t *db);*/
//void read_isr_print(board_info_t *db);

#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
   /* this no need register board information! */
#else
static struct spi_board_info dm9051_spi_board_devs[] __initdata = {
	[0] = {
	.modalias = "dm9051",
	.max_speed_hz = DRV_MAX_SPEED_HZ,
	.bus_num = DRV_SPI_BUS_NUMBER,
	.chip_select = DRV_SPI_CHIP_SELECT,
	.mode = SPI_MODE_0,
  	  #ifdef DM_CONF_POLLALL_INTFLAG 
  	  // or, while swap between polling & interrupt and also conf module, .irq should keep same value for doing "dm9051_device_spi_delete()." 
	 .irq = DM_CONF_INTERRUPT_IRQ,  
  	  #endif
	},
};
#endif

/* ethtool ops block (to be "dm9051_ethtool.c") */
/*[DM9051_Ethtool_Ops.c]*/
static inline board_info_t *to_dm9051_board(struct net_device *dev)
{
	return netdev_priv(dev);
}

static void dm9051_get_drvinfo(struct net_device *dev,
				struct ethtool_drvinfo *info)
{
	strcpy(info->driver, CARDNAME_9051);
	strcpy(info->version, DRV_VERSION);
	strlcpy(info->bus_info, dev_name(dev->dev.parent), sizeof(info->bus_info));
}

static void dm9000_set_msglevel(struct net_device *dev, u32 value)
{
	board_info_t *dm = to_dm9051_board(dev);
	dm->msg_enable = value;
}

static u32 dm9000_get_msglevel(struct net_device *dev)
{
	board_info_t *dm = to_dm9051_board(dev);
	return dm->msg_enable;
}

#if LNX_KERNEL_v58
static int dm9000_get_link_ksettings(struct net_device *dev,
				     struct ethtool_link_ksettings *cmd)
{
	struct board_info *dm = to_dm9051_board(dev);
	mii_ethtool_get_link_ksettings(&dm->mii, cmd);
	return 0;
}
static int dm9000_set_link_ksettings(struct net_device *dev,
				     const struct ethtool_link_ksettings *cmd)
{
	struct board_info *dm = to_dm9051_board(dev);
	return mii_ethtool_set_link_ksettings(&dm->mii, cmd);
}

#else
static int dm9000_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	board_info_t *dm = to_dm9051_board(dev);
	mii_ethtool_gset(&dm->mii, cmd);
	return 0;
}

static int dm9000_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	board_info_t *dm = to_dm9051_board(dev);
	return mii_ethtool_sset(&dm->mii, cmd);
}
#endif

static int dm9000_nway_reset(struct net_device *dev)
{
	board_info_t *dm = to_dm9051_board(dev);
	return mii_nway_restart(&dm->mii);
}

static u32 dm9000_get_link(struct net_device *dev)
{
	board_info_t *dm = to_dm9051_board(dev);
	return (u32)dm->linkBool;
}

#define DM_EEPROM_MAGIC		(0x444D394B)

static int dm9000_get_eeprom_len(struct net_device *dev)
{
	return 128;
}

static int dm9000_get_eeprom(struct net_device *dev,
			     struct ethtool_eeprom *ee, u8 *data)
{

	printk("[dm9051_get_eeprom]\n");

#if 0
	board_info_t *dm = to_dm9051_board(dev);
	int offset = ee->offset;
	int len = ee->len;
	int i;

	// EEPROM access is aligned to two bytes 
	if ((len & 1) != 0 || (offset & 1) != 0)
		return -EINVAL;

	ee->magic = DM_EEPROM_MAGIC;

	for (i = 0; i < len; i += 2)
		dm9051_read_eeprom(dm, (offset + i) / 2, data + i);
#endif
	return 0;
}

static int dm9000_set_eeprom(struct net_device *dev,
			     struct ethtool_eeprom *ee, u8 *data)
{	
	printk("[dm9051_set_eeprom]\n");

#if 0	
	board_info_t *dm = to_dm9051_board(dev);
	int offset = ee->offset;
	int len = ee->len;
	int i;

	// EEPROM access is aligned to two bytes 
	if ((len & 1) != 0 || (offset & 1) != 0)
		return -EINVAL;

	if (ee->magic != DM_EEPROM_MAGIC)
		return -EINVAL;

	for (i = 0; i < len; i += 2)
		dm9051_write_eeprom(dm, (offset + i) / 2, data + i);
#endif		
	return 0;
}

static const struct ethtool_ops dm9051_ethtool_ops = {
	.get_drvinfo		= dm9051_get_drvinfo,
#if LNX_KERNEL_v58
	.get_link_ksettings	= dm9000_get_link_ksettings,
	.set_link_ksettings	= dm9000_set_link_ksettings,
#else
	.get_settings		= dm9000_get_settings,
	.set_settings		= dm9000_set_settings,
#endif
	.get_msglevel		= dm9000_get_msglevel,
	.set_msglevel		= dm9000_set_msglevel,
	.nway_reset			= dm9000_nway_reset,
	.get_link			= dm9000_get_link,
/*TBD	
	.get_wol			= dm9000_get_wol,
	.set_wol			= dm9000_set_wol,
*/
 	.get_eeprom_len		= dm9000_get_eeprom_len,
 	.get_eeprom			= dm9000_get_eeprom,
 	.set_eeprom			= dm9000_set_eeprom,
};

#include "driver.c"

/*static int
dm9000_drv_suspend(struct device *dev)
{
	if (ndev) {
		db->in_suspend = 1;
		if (!netif_running(ndev))
			return 0;
		netif_device_detach(ndev);

		if (!db->wake_state)
			dm9000_shutdown(ndev);
	}
	return 0;
}
static int
dm9000_drv_resume(struct device *dev)
{
	if (ndev) {
		if (netif_running(ndev)) {
			if (!db->wake_state) {
				dm9000_init_dm9000(ndev);
				dm9000_unmask_interrupts(db);
			}
			netif_device_attach(ndev);
		}

		db->in_suspend = 0;
	}
	return 0;
}

//static const struct dev_pm_ops dm9000_drv_pm_ops = {
//	.suspend	= dm9000_drv_suspend,
//	.resume		= dm9000_drv_resume,
//}; */

#if DMA3_P4_KT
/*3p*/
static int dm9051_drv_suspend(struct spi_device *spi, pm_message_t state)
{
    board_info_t *db = dev_get_drvdata(&spi->dev);
    struct net_device *ndev = db->ndev;
	if (ndev) {
//.	db->in_suspend = 1;
		if (!netif_running(ndev)) 
			return 0;

		netif_device_detach(ndev);
        dm9051_stop(ndev);
	}
	return 0;
}

static int dm9051_drv_resume(struct spi_device *spi)
{
    board_info_t *db = dev_get_drvdata(&spi->dev);
    struct net_device *ndev = db->ndev;
	if (ndev) {
		if (netif_running(ndev)) {
            dm9051_open(ndev);
			netif_device_attach(ndev);
		}

//.	db->in_suspend = 0;
	}
	return 0;
}
#endif

#ifdef CONFIG_PM_SLEEP
//[User must config KConfig to PM_SLEEP for the power-down function!!]
#if DMA3_P4N_KT
static int dm9051_drv_suspend(struct device *dev) //(struct spi_device *spi)  //...
{
    board_info_t *db = dev_get_drvdata(dev); //(&spi->dev);
    struct net_device *ndev = db->ndev;
	if (ndev) {
		if (!netif_running(ndev)) 
			return 0;
		netif_device_detach(ndev);
		dm9051_stop(ndev);
	}
	return 0;
}
static int dm9051_drv_resume(struct device *dev) //(struct spi_device *spi)   //...
{
    board_info_t *db = dev_get_drvdata(dev); //(&spi->dev);
    struct net_device *ndev = db->ndev;
	if (ndev) {
		if (netif_running(ndev)) {
			dm9051_open(ndev);
			netif_device_attach(ndev);
		}
	}
	return 0;
}
#endif
#endif

#if DMA3_P4N_KT
static SIMPLE_DEV_PM_OPS (dm9051_drv_pm_ops, dm9051_drv_suspend, dm9051_drv_resume);
#endif

static struct of_device_id dm9051_match_table[] = { //"davicom,dm9051" (dts_yes_driver_table.h)
	{	.compatible = "davicom,dm9051",	},
	{}
};

struct spi_device_id dm9051_spi_id_table = {"dm9051", 0}; //DRVNAME_9051 (dts_yes_driver_table.h)

static struct spi_driver dm9051_driver = {
	.driver	= {
		.name  = DRVNAME_9051, //"dm9051"
		.owner = THIS_MODULE,
		#if DMA3_P4N_KT
		.pm	= &dm9051_drv_pm_ops,
		#endif
		.of_match_table = dm9051_match_table,
		.bus = &spi_bus_type,
	},
	.probe   = dm9051_probe,
	.remove  = dm9051_drv_remove,
	.id_table = &dm9051_spi_id_table,
	#if DMA3_P4_KT
	/*3p*/ .suspend = dm9051_drv_suspend,
	/*3p*/ .resume = dm9051_drv_resume,
	#endif
	#if DMA3_P4N_KT
	//.suspend = dm9051_drv_suspend,
	//.resume = dm9051_drv_resume,
	#endif
};

// 1----------------------------------------------------------------------------------------------------------

#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
   /* this no need register board information ! */
#else
#if DMA3_P6_DRVSTATIC
 /* Joseph 20151030 */
 extern int spi_register_board_info(struct spi_board_info const *info, unsigned n);
#else
 /* Joseph: find/delete/new */
 static unsigned verbose = 3;
 module_param(verbose, uint, 0);
 MODULE_PARM_DESC(verbose,
 "0 silent, >0 show gpios, >1 show devices, >2 show devices before (default=3)");

 static struct spi_device *spi_device;

 static void dm9051_device_spi_delete(struct spi_master *master, unsigned cs)
 {
	struct device *dev;
	char str[32];

	snprintf(str, sizeof(str), "%s.%u", dev_name(&master->dev), cs);

	dev = bus_find_device_by_name(&spi_bus_type, NULL, str);
	if (dev) {
		if (verbose)
			pr_info(DRVNAME_9051": Deleting %s\n", str);
		device_del(dev);
	}
 }
 static int dm9051_spi_register_board_info(struct spi_board_info *spi, unsigned n)
 {
	/* Joseph_20151030: 'n' is always 1, ARRAY_SIZE(table) is 1 table-item in this design  */
	struct spi_master *master;

	master = spi_busnum_to_master(spi->bus_num);
	if (!master) {
		pr_err(DRVNAME_9051 ":  spi_busnum_to_master(%d) returned NULL\n",
								spi->bus_num);
		return -EINVAL;
	}
	/* make sure it's available */
	dm9051_device_spi_delete(master, spi->chip_select);
	spi_device = spi_new_device(master, spi);
	put_device(&master->dev);
	if (!spi_device) {
		pr_err(DRVNAME_9051 ":    spi_new_device() returned NULL\n");
		return -EPERM;
	}
	return 0;
 }

 static void dm9051_spi_unregister_board(void)
 {
	//----------------------
	//[#ifdef MODULE #endif]
	//----------------------
	if (spi_device) {
		device_del(&spi_device->dev);
		kfree(spi_device);
	}
 }
#endif
#endif

void conf_spi_board(void)
{
/* ------------------------------------------------------------------------------------------- */
/* #if DM_DM_CONF_INSTEAD_OF_DTS_AND_BE_DRVMODULE                                              */
/*   dm9051_spi_register_board_info(dm9051_spi_board_devs, ARRAY_SIZE(dm9051_spi_board_devs)); */
/* #else                                                                                       */
/* #endif                                                                                      */
/* ------------------------------------------------------------------------------------------- */

  #if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
   /* this no need register board information ! */
  #else
    #if DMA3_P6_DRVSTATIC
      spi_register_board_info(dm9051_spi_board_devs, ARRAY_SIZE(dm9051_spi_board_devs));
    #else
      dm9051_spi_register_board_info(dm9051_spi_board_devs, ARRAY_SIZE(dm9051_spi_board_devs));
    #endif
  #endif
}

void unconf_spi_board(void)
{
/* ---------------------------------------------- */
/* #if DM_DM_CONF_INSTEAD_OF_DTS_AND_BE_DRVMODULE */
/*     dm9051_spi_unregister_board();             */
/* #else                                          */
/* #endif                                         */
/* ---------------------------------------------- */

  #if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
   /* this no need register board information ! */
  #else
    #if DMA3_P6_DRVSTATIC
    #else
      dm9051_spi_unregister_board();
    #endif
  #endif
}

static int __init  
dm9051_init(void)
{
    printk("\n");
  //printk("[dm951_insmod].s -------- 00.s --------\n");
    printk("[dm951_s_insmod].s ");
    printk("  --");
    printk("  --");
    printk("  --");
    printk("--00.e--");
#if 0
    conf_spi_print(-);
    //;;NO 'db' here! / ;;NO 'spi' here!
	//;unsigned chipid= dm9051_chipid(db); 
	//;if (chipid==(DM9051_ID>>16))
	//;	printk("Read [DM9051_PID] = %04x OK\n", chipid);
#endif    
    printk("  --");
    printk("  --");
    printk("  --\n");
    printk("%s, DRV %s\n", CARDNAME_9051, MSTR_DTS_VERSION);
    printk("%s, DRV %s\n", CARDNAME_9051, MSTR_MOD_VERSION);
    printk("%s, DRV %s\n", CARDNAME_9051, MSTR_INT_VERSION);
    printk("%s, SPI %s\n", CARDNAME_9051, RD_MODEL_VERSION);
    printk("%s, SPI %s\n", CARDNAME_9051, WR_MODEL_VERSION);
    
    printk("%s Driver loaded, V%s (%s)\n", CARDNAME_9051, 
	DRV_VERSION, "LOOP_XMIT"); //str_drv_xmit_type="LOOP_XMIT"
	
#ifdef MTK_CONF_YES
    printk("%s, SPI %s\n", CARDNAME_9051, MSTR_MTKDMA_VERSION);
#endif
    printk("%s, SPI %s\n", CARDNAME_9051, MSTR_DMA_SYNC_VERSION);

#if 0
	dm9051_hw_reset();
#endif
	conf_spi_board();
	printk("[dm951_s_insmod].e\n");
	return spi_register_driver(&dm9051_driver);
}

static void dm9051_cleanup(void)
{
    printk("dm9051_exit.s\n");
	unconf_spi_board();
    printk("[dm951_e_rmmod].s ------- 00.s -------\n");
	spi_unregister_driver(&dm9051_driver);
    printk("[dm951_e_rmmod].e ------- 00.e -------\n");
}

module_init(dm9051_init);
module_exit(dm9051_cleanup);

MODULE_AUTHOR("Joseph CHANG <joseph_chang@davicom.com.tw>");
MODULE_DESCRIPTION("Davicom DM9051 network driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:dm9051");
