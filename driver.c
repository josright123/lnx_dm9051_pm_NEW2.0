/* ops: driver main procedures */

void conf_spi_print(struct spi_device *spi)
{
	#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
	int rc;
	struct device_node *nc;
	u32 value;
	nc = of_find_compatible_node(NULL, NULL, DM_DM_CONF_DTS_COMPATIBLE_USAGE);
	if (!nc) {
		printk("[ *dm9051 dts WARN ] %s has no spi-max-frequency[= ?]\n", nc->full_name);
		return;
	}
	
	//;if (nc) { //get DTS define as spi0 or spi1 ?
	//;}
	
	rc = of_property_read_u32(nc, "spi-max-frequency", &value); // read to DTS 
	if (rc) {
		printk("[ *dm9051 dts WARN ] No DTS compatible not find, so SPI speed[= ?]\n");
		return;
	}
	
	/* DTS, this speed setting is in the dts file, not in the driver ! */
	//if (!rc) {
	//}
	
	printk("[ *dm9051 dts: yes] %s spi-max-frequency[= %d]\n", nc->full_name, value);
	printk("[ *dm9051 dts: yes] spi bus_num[= %d]\n", spi->master->bus_num);
	printk("[ *dm9051 dts: yes] spi  spi_cs[= %d]\n", spi->chip_select);
	return;
	#else
#if !LNX_KERNEL_v58
	printk("[ *dm9051 dts: no] CONFIG SPI speed[= %d]\n", dm9051_spi_board_devs[0].max_speed_hz);
#endif
	printk("[ *dm9051 dts: no] spi bus_num[= %d]\n", DRV_SPI_BUS_NUMBER);
	printk("[ *dm9051 dts: no] spi  spi_cs[= %d]\n", DRV_SPI_CHIP_SELECT);
	#endif	
}

//.void read_isr_print(board_info_t *db)
//.{
	//.#if 0
	//.unsigned  dat= ior(db, DM9051._ISR);
	//.printk("[dm9051.probe.ISR.MBNDRY_STS] data[= 0x%02x]\n", dat);
	//.#endif
//.}

/* event: play a schedule starter in condition */
static netdev_tx_t 
DM9051_START_XMIT(struct sk_buff *skb, struct net_device *dev) //void sta_xmit_sched_delay_work(board_info_t * db)
{
#if DM_CONF_APPSRC
	board_info_t *db = netdev_priv(dev);
#ifdef DM_CONF_POLLALL_INTFLAG	// ONly interrupt mode can has countted 'db->nSCH_INT'
	if (/*db->nSCH_INT &&*/ (db->nSCH_INT <= DM_CONF_SPI_DBG_INT_NUM))
		printk("[DM9051_START_XMIT %d][%d]/[%d].s\n", db->nSCH_XMIT, db->nSCH_INT, DM_CONF_SPI_DBG_INT_NUM);
#endif

  //.#if defined MORE_DM9051_MUTEX && defined  MORE_DM9051_MUTEX_EXT
  //.mutex_lock(&db->spi_lock);
  //.#endif 	
	#if DM_CONF_APPSRC & DM9051_CONF_TX
	toend_stop_queue1(dev, 1 );
	// dm9051_tx
	// Need "skb = skb_dequeue(&db->txq)" to get tx-data
	// JJ: a skb add to the tail of the list '&db->txq'
	skb_queue_tail(&db->txq, skb); 
	#endif

	//#if 0
	//driver_dtxt_step(db, '0');  // driver_dtxt_step(db, 'q'); // Normal
	//#endif
	
	db->nSCH_XMIT++;
  //.#if defined MORE_DM9051_MUTEX && defined  MORE_DM9051_MUTEX_EXT
  //.mutex_unlock(&db->spi_lock);
  //.#endif	
  
  #ifdef DM_CONF_POLLALL_INTFLAG
	dm9051_INTPschedule_isr(db, R_SCH_XMIT); // in 'dm9051_start_xmit'
  #endif

#ifdef DM_CONF_POLLALL_INTFLAG	// ONly interrupt mode can has countted 'db->nSCH_INT'
	if (/*db->nSCH_INT &&*/ (db->nSCH_INT <= DM_CONF_SPI_DBG_INT_NUM))
		printk("[DM9051_START_XMIT %d][%d]/[%d].e\n", db->nSCH_XMIT, db->nSCH_INT, DM_CONF_SPI_DBG_INT_NUM);
#endif		
#endif
	return NETDEV_TX_OK;
}

/* ops */
/* play with a schedule starter */
static void 
dm9051_set_multicast_list_schedule(struct net_device *dev)
{
#if DEF_PRO & DM_CONF_APPSRC
	board_info_t *db = netdev_priv(dev);
	sched_work(db);
#endif
}

/* ops */
/**
 * Open network device
 * Called when the network device is marked active, such as a user executing
 * 'ifconfig up' on the device.
 */
static int
dm9051_open(struct net_device *dev)
{
//	printk("[dm951_open].maincode.s ------- 02.s -------\n");
#if DEF_OPE
	do {
	board_info_t *db = netdev_priv(dev);
	SCH_clear(db);

#if 0		
#ifdef DM_CONF_POLLALL_INTFLAG	
/* (Must be after dm9051.open.code()) */
    int_begin(db->spidev, dev); 
/* (Splite this func to = 'int_get_attribute' + 'int_get_begin') */
#endif
#endif
#if 1
#ifdef DM_CONF_POLLALL_INTFLAG	
	int_get_attribute(db->spidev, dev);
#endif
#endif

	mutex_lock(&db->addr_lock); //Note: must 
	
	printk("dm9.[dm9051_open_c].s\n");
	
	if (db->chip_code_state==CCS_NUL)
		dm9051_open_code(dev, db);
		
	read_intcr_print(db);
	printk("dm9.[dm9051_open_c].e\n");
	
#if 1
#ifdef DM_CONF_POLLALL_INTFLAG	
/* (Must be after dm9051.open.code()) or later and before int_en() */
    int_get_begin(db->spidev, dev); // (disable.irq)_insided
#endif
#endif

#if DM_CONF_APPSRC	
	netif_carrier_off(dev); //new_add: (We add in begin for 'dm_schedule._phy' or 'dm_sched_start_rx' to detect and change to linkon.)
#endif    
  #if DM_CONF_APPSRC & DM9051_CONF_TX
    //[Init.] [Re-init compare to ptobe.] //db->tx_eq= 0; //db->tx_err= 0;
	skb_queue_head_init(&db->txq); 
	netif_start_queue(dev);
  #endif	
  
	mutex_unlock(&db->addr_lock);
	
  #if DM_CONF_APPSRC & DM9051_CONF_TX
    opening_wake_queue1(dev);
  #endif	
  
//#ifdef DM_CONF_POLLALL_INTFLAG	
//	printk("[dm951_open].maincode.m ------- 02.e.INTmode -------\n\n");
//#else
//	printk("[dm951_open].maincode.m ------- 02.e.POLLmode -------\n\n");
//#endif
  
#ifdef DM_CONF_POLLALL_INTFLAG	
	printk("[dm951_open].INT_EN.s -------\n");
#else
	printk("[dm951_open].POLL.s -------\n");
#endif	
	  
#if DEF_SPIRW
	printk("[dm951_open].[before-int_reg_start (IMR %02x ) statis nSCH_INT= %d] -------\n\n", /*ior*/ iior(db, DM9051_IMR), db->nSCH_INT);
#endif

#ifdef DM_CONF_POLLALL_INTFLAG	
	int_en(dev);
	int_reg_start(db, "[dm951_INT_open]"); //peek(imr_all)
#else
	int_reg_start(db, "[dm951_poll_open]"); //pol_reg_start(db);
#endif

#if DEF_SPIRW
	iiow(db, DM9051_RCR, db->rcr_all);
#endif
  #if defined DM_CONF_PHYPOLL & DM_CONF_APPSRC
	dm_schedule_phy(db); //.........dfbtyjuyukytru8k8.....
  #endif
#if DM_CONF_APPSRC
	dm_sched_start_rx(db); //Finally, start the delay work, to be the last calling, for you can not read/wrie dm9051 register since poling schedule work has began! 
  #endif
  
#ifdef DM_CONF_POLLALL_INTFLAG	
	printk("[dm951_open].INT_EN.e -------\n");
#else
	printk("[dm951_open].POLL.e -------\n");
#endif	

//#ifdef DM_CONF_POLLALL_INTFLAG	
//	printk("[dm951_open].maincode.e ------- 02.e.INTmode -------\n\n");
//#else
//	printk("[dm951_open].maincode.e ------- 02.e.POLLmode -------\n\n");
//#endif
	} while (0);
#endif
	return 0;
}

/**
 * dm951_net_stop - close network device
 * @dev: The device being closed.
 *
 * Called to close down a network device which has been active. Cancell any
 * work, shutdown the RX and TX process and then place the chip into a low
 * power state whilst it is not being used.
 */
static int dm9051_stop(struct net_device *dev)
{
	printk("[dm951_if_stop].s ------- 02.e -------\n");
#if DEF_STO
	do {
	board_info_t *db = netdev_priv(dev);
#ifdef DM_CONF_POLLALL_INTFLAG	
	int_dis(dev);
	 mutex_lock(&db->addr_lock);
	int_reg_stop(db);
	 mutex_unlock(&db->addr_lock);
    int_end(db->spidev, db);
#endif

	/* "kernel_call.ndo_set_multicast_list.first". */
	/* Then.call.ndo_stop                          */
	db->driver_state= DS_IDLE;
	db->chip_code_state= CCS_NUL;
	
#if DM_CONF_APPSRC	
	sched_delay_work_cancel(db);
	toend_stop_queue1(dev, NUM_QUEUE_TAIL);
#endif
	//JJ-Count-on
	netif_carrier_off(dev);
	
	/* dm9051_shutdown(dev) */
	dm9051_stop_code(dev, db);
	} while (0);
#endif
	return 0;
}

void
dm9051_set_mac_ops(struct net_device * ndev, void * p) {
  board_info_t * db = netdev_priv(ndev);
  u8 * s = p;
  int offset = 0;
  //u8 rmac[6];
  int i;
  //[param check]
  printk("dm9 [write mac permanently]\n");
  printk("set param mac dm9051 %02x %02x %02x  %02x %02x %02x\n", s[0], s[1],
    s[2], s[3], s[4], s[5]);
  //[dm9]				    
  iow(db, DM9051_PAR + 0, s[0]);
  iow(db, DM9051_PAR + 1, s[1]);
  iow(db, DM9051_PAR + 2, s[2]);
  iow(db, DM9051_PAR + 3, s[3]);
  iow(db, DM9051_PAR + 4, s[4]);
  iow(db, DM9051_PAR + 5, s[5]);
  //for (i = 0; i < 6; i++)
  //ndev->dev_addr[i]= s[i];
  for (i = 0; i < 6; i++) {
    ndev -> dev_addr[i] = ior(db, DM9051_PAR + i);
  }
  //[mac reg]
  for (i = 0; i < 6; i++) {
    if (ndev -> dev_addr[i] != s[i]) {
      break;
    }
  }
  if (i != 6) {
    printk("dm9 set mac(but not as parameters) chip mac %02x %02x %02x  %02x %02x %02x [Can't write]\n", ndev -> dev_addr[0], ndev -> dev_addr[1],
      ndev -> dev_addr[2], ndev -> dev_addr[3], ndev -> dev_addr[4], ndev -> dev_addr[5]);
    return;
  }
  //[eeprom]
  printk("write eeprom mac dm9051 %02x %02x %02x  %02x %02x %02x\n", s[0], s[1], s[2], s[3], s[4], s[5]);
  for (i = 0; i < 6; i += 2)
    dm9051_write_eeprom(db, (offset + i) / 2, s + i);
  printk("[dm9 write and then read]\n");
  dm9051_show_eeprom_mac(db);
}

int
dm9051_set_mac_address(struct net_device *dev, void *p)
{
	char *s = p;
	printk("dm9051_set_mac_address %x %x %x  %x %x %x\n", s[0],s[1],s[2],s[3],s[4],s[5]);
	printk("dm9051_set_mac_address (%02x %02x)  %02x %02x %02x  %02x %02x %02x\n", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
	dm9051_set_mac_ops(dev, s + 2);
	return eth_mac_addr(dev, p);
}

/* ops */
#if 1
//#if DEF_PRO
static const struct net_device_ops dm9051_netdev_ops = {
	.ndo_open		= 	dm9051_open,
	.ndo_stop		= 	dm9051_stop,
	.ndo_start_xmit		= 	DM9051_START_XMIT, //(dm9051_start_xmit)
//>.ndo_tx_timeout		= 	dm9000_timeout,
	.ndo_set_rx_mode 	= 	dm9051_set_multicast_list_schedule,
//..ndo_do_ioctl		= 	dm9051_ioctl,
//..ndo_set_features		= 	dm9000_set_features,
#if !LNX_KERNEL_v58
	.ndo_change_mtu		= 	eth_change_mtu,
#endif
	.ndo_validate_addr	= 	eth_validate_addr,
	.ndo_set_mac_address	= 	dm9051_set_mac_address, //eth_mac_addr,	
	#ifdef CONFIG_NET_POLL_CONTROLLER
	//.ndo_poll_controller= ...
	#endif
};
//#endif
#endif

void SPI_SPI_Setup(struct board_info *db) //(struct spi_device *spi)
{    
#if DMA3_P1_MTKSETUP
		SPI_PARAM_Set(db);
#endif

#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
		/* While DTS modle, Define spi max speed in the DTS file */
#else
#if LNX_KERNEL_v58
		db->spidev->max_speed_hz= DRV_MAX_SPEED_HZ;
#else
		db->spidev->max_speed_hz= dm9051_spi_board_devs[0].max_speed_hz;	
#endif
#endif
#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
		/* While DTS modle, Define spi max speed in the DTS file */
#else
		db->spidev->mode = SPI_MODE_0;
		db->spidev->bits_per_word = 8;
		
		printk("%s Driver spi_setup()\n", CARDNAME_9051);
		if(spi_setup(db->spidev)){
			printk("[dm95_spi] spi_setup fail\n");
			return;
		}
#endif		
}

#if DEF_PRO
static int SubNetwork_SPI_Init(struct board_info *db, int enable)
{
	//mutex_lock(&db->addr_lock);
	if(enable){	
#if defined MTK_CONF_XY6762TEMP /* || defined QCOM_CONF_BOARD_YES */
		SPI_GPIO_SetupPwrOn(db);
#endif	
#if 0 //DMA3_P1_MTKSETUP
        SPI_GPIO_Set(1); //mt_dm9051_pinctrl_init(db->spidev); //or, SPI_GPIO_Set(1);
#endif
        SPI_SPI_Setup(db);
	}
	//mutex_unlock(&db->addr_lock);
	return 0;
}

void conf_mii(struct net_device *dev, struct board_info *db)
{	
	db->mii.dev = dev;
	db->mii.phy_id_mask  = 1;   //db->mii.phy_id_mask  = 0x1f;
	db->mii.reg_num_mask = 0xf; //db->mii.reg_num_mask = 0x1f;
	db->mii.phy_id		= 1;
#if DEF_SPIRW
	db->mii.mdio_read    = dm9051_phy_read_lock;
	db->mii.mdio_write   = dm9051_phy_write_lock;
#endif
}
#endif

/*
 * Search DM9051 board, allocate space and register it
 */
static int
dm9051_probe(struct spi_device *spi)
{
    printk("[dm9051_i-probe].s --------- 01.s ---------\n");
#if DEF_PRO
	do {
	struct board_info *db;
	struct net_device *ndev;
	unsigned  chipid;
	const unsigned char *mac_src;
#if DEF_SPIRW
	int i;
#endif	
	int ret = 0;
   
	printnb_init(1); // 1 for print-log, 0 for no print-log 
	ndev = alloc_etherdev(sizeof(struct board_info));
	if (!ndev) {
		dev_err(&spi->dev, "failed to alloc ethernet device\n");
		return -ENOMEM;
	}
	printk("dm951: mtu %d\n", ndev->mtu);
	ndev->mtu = 1500; // My-eth-conf

	/* setup board info structure */
	db = netdev_priv(ndev);
	
	db->ndev = ndev;
	db->spidev = spi;

#if DEF_SPIRW
    Custom_Board_Init(spi);
	printk("SubNetwork_SPI_Init\n");
	SubNetwork_SPI_Init(db, 1); //contain with spi->bits_per_word = 8;
	if (dm9051_dbg_alloc(db)) {
		ret = -ENOMEM;
		goto err_first_prepare;
	}
	if (dm9051_spirw_begin(db)) {
		ret = -ENOMEM;
		goto err_prepare;
	}
	dm9051_spimsg_init(db);
#endif
	
#ifdef MORE_DM9051_MUTEX
	mutex_init(&db->spi_lock);
#endif
	mutex_init(&db->addr_lock);
	spin_lock_init(&db->statelock_tx1_rx1); // used in 'dm9051' 'start' 'xmit'
#if DM_CONF_APPSRC		
	define_delay_work(db);
#endif	
	/* initialise pre-made spi transfer messages */
#if DEF_SPICORE_IMPL0	
	spi_message_init(&db->spi_msg1);
	spi_message_add_tail(&db->spi_xfer1, &db->spi_msg1);
#endif
	
	/* setup mii state */
	conf_mii(ndev, db);          
	printk("dm951: mtu %d\n", ndev->mtu);
	
#if DM_CONF_APPSRC	
	toend_stop_queue1(ndev, NUM_QUEUE_TAIL); //ending_stop_queue1(ndev);	
#endif	
	skb_queue_head_init(&db->txq); //[Init.]
#if 1
	SET_NETDEV_DEV(ndev, &spi->dev);
    /*
	 * No need: db->dev = &pdev->dev;            
     * May need: dev_set_drvdata(&spi->dev, db); 
     */
    dev_set_drvdata(&spi->dev, db);
#endif

	dm9051_reset(db);
    conf_spi_print(spi);
	chipid= dm9051_chipid(db); 
	if (chipid!=(DM9051_ID>>16)) {
		printk("Read [DM9051_PID] = %04x\n", chipid);
		printk("Read [DM9051_PID] error!\n");
		goto err_id;
	}

	//MAC
	mac_src= "eeprom2chip.0"; //"Free-Style";
#if DEF_SPIRW
	for (i = 0; i < 6; i++)
      ndev->dev_addr[i]= ior(db, DM9051_PAR+i);
#endif
	//.printk("[dm9051.ior_mac(): [%02x %02x %02x %02x %02x %02x] (%s)\n",
	//.	ndev->dev_addr[0],ndev->dev_addr[1],ndev->dev_addr[2],
	//.	ndev->dev_addr[3],ndev->dev_addr[4],ndev->dev_addr[5],
	//.	mac_src);
	printk("dm951: at MAC: %pM (%s)\n",
		   ndev->dev_addr, "DBG-0");
		   
#if 1
	if (!is_valid_ether_addr(ndev->dev_addr)) {
		mac_src= "fixed2chip.0"; //"Free-Style";	
		//for (i = 0; i < 6; i++)
#if DEF_SPIRW
		iow(db, DM9051_PAR+0, 0x00);
		iow(db, DM9051_PAR+1, 0x60);
		iow(db, DM9051_PAR+2, 0x6e);
		iow(db, DM9051_PAR+3, 0x90);
		iow(db, DM9051_PAR+4, 0x51);
		iow(db, DM9051_PAR+5, 0xee);
		for (i = 0; i < 6; i++)
		  ndev->dev_addr[i]= ior(db, DM9051_PAR+i);
#endif
		printk("dm951: at MAC: %pM (%s)\n",
		   ndev->dev_addr, "DBG-0.1");
	}
#endif		   
	
	ndev->if_port = IF_PORT_100BASET;
	ndev->netdev_ops	= &dm9051_netdev_ops;
#if DMA3_P3_KT
      SET_ETHTOOL_OPS(ndev, &dm9051_ethtool_ops); /*3p*/  
#else
      ndev->ethtool_ops = &dm9051_ethtool_ops;
#endif

 //.printk("dm9051.check register netdev.s [eth created]\n");
	printk("dm951: mtu %d\n", ndev->mtu);
	printk("[dm9051_eth(n)-reg-netdev].s --------- 01.m1 ---------\n");
	ret = register_netdev(ndev);
    if (ret) {
		dev_err(&spi->dev, "failed to register network device\n");
        printk("[  dm9051  ] dm9051_probe {failed to register network device}\n");
        goto err_netdev;
    }
	printk("[dm9051_(%s)-reg-netdev].s --------- 01.m2 ---------\n", ndev->name);
	printk("dm951 %s: mtu %d\n", ndev->name, ndev->mtu);
    
	printk("dm951: at MAC: %pM (%s)\n",
		   ndev->dev_addr, "DBG-1");
		   
	//.mac_src= "eeprom2chip.1"; //"Free-Style";
	//.for (i = 0; i < 6; i++)
    //.  ndev->dev_addr[i]= ior(db, DM9051_PAR+i);   
	//.printk("dm951: at MAC: %pM (%s)\n",
	//.	   ndev->dev_addr, "DBG-2");
		   
    db->driver_state= DS_NUL;
    db->chip_code_state= CCS_NUL;
	
	//.read_isr_print(db);
	printk("dm951 %s: at MAC: %pM (%s)\n", //isNO_IRQ %d 
		   ndev->name, ndev->dev_addr, mac_src); //ndev->irq,
    printk("dm951 %s: bus_num %d, spi_cs %d\n",  //"(%s)", DRV_VERSION
           ndev->name, spi->master->bus_num, 
    	   spi->chip_select); 
	printk("[dm95_spi] spi_setup db->spidev->bits_per_word= %d\n",db->spidev->bits_per_word);
	printk("[dm95_spi] spi_setup db->spidev->mode= %d\n",db->spidev->mode);
	printk("[dm95_spi] spi_setup db->spidev->db->spidev->max_speed_hz= %d\n",db->spidev->max_speed_hz);
	Operation_clear(db); //[In probe, this should be essential.]
	SCH_clear(db); //[In probe, this should be not essential.]
	return 0;
		   
err_netdev:
err_id:
	dm9051_spirw_end(db);
#if DEF_SPIRW
err_prepare:
	dm9051_dbg_free(db);
err_first_prepare:
#endif
	free_netdev(ndev);
	return ret;
	} while (0);
#endif
	return 0;
}

//--------------------------------------------------------------------------------------

static int
dm9051_drv_remove(struct spi_device *spi)  // vs. dm9051_probe
{
    printk("[dm951_u-probe].s ------- 01.s -------\n");
#if DEF_REM    
	do {
    board_info_t *db = dev_get_drvdata(&spi->dev);
    
    dm9051_spirw_end(db);
    //kfree(db->spi_sypc_buf);
    //devm_kfree(&spi->dev, db->spi_sypc_buf);
    dm9051_dbg_free(db);
    
  //int._end(db->spidev, db);
	unregister_netdev(db->ndev);
	free_netdev(db->ndev);
	} while(0);
#endif
	return 0;
}       
