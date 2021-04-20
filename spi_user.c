#if DEF_OPE
void read_intcr_print(board_info_t *db)
{
	unsigned  rdv = 0;
	unsigned char *int_pol;
#if DEF_SPIRW
	rdv= iior(db, DM9051_INTCR);
#endif
	int_pol= "------- active high -------";
	if (rdv&0x01)
	  int_pol= "------- active low -------";
	printk("ior[REG39H][%02x] (b0: %d)(%s)\n", rdv, rdv&0x01, int_pol);
}
#endif

#if DEF_OPE | DM_CONF_APPSRC
/*
 *  INT 
 */
void int_reg_stop(board_info_t *db)
{
#if DEF_SPIRW
	iiow(db, DM9051_IMR, IMR_PAR); // Disable all interrupts 
	if (db->nSCH_INT && (db->nSCH_INT <= DM9_DBG_INT_ONOFF_COUNT))
		printk("[dm9IMR].[%02x].dis ------- nINT= %d\n",
			iior(db, DM9051_IMR), db->nSCH_INT);
#endif
}

void int_reg_start(board_info_t *db, char *headstr)
{		
#if DEF_SPIRW
	iiow(db, DM9051_IMR, db->imr_all); /*iow*/
	if (db->nSCH_INT && (db->nSCH_INT <= DM9_DBG_INT_ONOFF_COUNT))
		printk("%s.[%02x].ena ------- nINT= %d\n", headstr,
			iior(db, DM9051_IMR), db->nSCH_INT);// Re-enable by interrupt mask register
#endif
}
#endif

#if DEF_PRO | DEF_OPE | DM_CONF_APPSRC
#if DEF_SPIRW
static void
reset_rst(board_info_t * db) {	
	iiow(db, DM9051_NCR, NCR_RST);
	//= 
	//__le16 txb[2]; 
	 // wbuff(DM_SPI_WR| DM9051_NCR | NCR_RST<<8, txb); //org: wrbuff(DM9051_NCR, NCR_RST, txb)
	 // xwrbyte(db, txb);
	mdelay(1);
}
static void
reset_bnd(board_info_t * db) {
	iiow(db, DM9051_MBNDRY, MBNDRY_BYTE);
	//= 
	//.__le16 txb[2]; 
	 // wbuff(DM_SPI_WR| 0x5e | 0x80<<8, txb); //org: wrbuff(DM9051_NCR, NCR_RST, txb)
	 // xwrbyte(db, txb);
	mdelay(1);
	//printk("iow[%02x %02x]\n", 0x5e, 0x80); //iow[x].[Set.MBNDRY.BYTEBNDRY]
}
#endif

static void
dm9051_reset(board_info_t * db)
{
	  mdelay(2); // delay 2 ms any need before NCR_RST (20170510)
	  #if DEF_SPIRW
	  reset_rst(db);
	  reset_bnd(db);
	  #endif      
	  db->rwregs1 = 0x0c00;
	  //[db->validlen_for_prebuf = 0;]
}
#endif

#if DM_CONF_APPSRC
// ------------------------------------------------------------------------------
// state: 0 , fill '90 90 90 ...' e.g. dm9051_fi.fo_re.set(0, "fifo-clear0", db);
//		  1 , RST
//        2 , dump 'fifo-data...'
//		 11 , RST-Silent
// hstr:  String 'hstr' to print-out
//        NULL (no 'hstr' print)
// Tips: If (state==1 && hstr!=NULL)
//        Do increase the RST counter
// ------------------------------------------------------------------------------
static void dm9051_fifo_reset(u8 state, u8 *hstr, board_info_t *db)
{
	u8 pk;
	if (state==11)
	{
		if (hstr)
     		{
			db->rx_rst_quan = 0;
			++db->bC.DO_FIFO_RST_counter;
			Disp_RunningEqu_Ending(db);
#ifdef ON_RELEASE
			rel_printk2("dm9-%s Len %03d RST_c %d\n", hstr, db->bC.RxLen, db->bC.DO_FIFO_RST_counter);
#else
			printk("dm9-%s Len %03d RST_c %d\n", hstr, db->bC.RxLen, db->bC.DO_FIFO_RST_counter); //printlog
#endif
		}
		dm9051_reset(db);	
		#if DEF_SPIRW
		iiow(db, DM9051_FCR, FCR_FLOW_ENABLE);	/* Flow Control */
		iiow(db, DM9051_PPCR, PPCR_SETTING); /* Fully Pause Packet Count */
#ifdef DM_CONF_POLLALL_INTFLAG
		#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE

			//#if defined DM_CONF_INTERRUPT_TEST_DTS_FALLING
			//	iiow(db, DM9051._INTCR, 0x01); //low active
			if (db->irq_type & (IIRQ_TYPE_LEVEL_HIGH | IIRQ_TYPE_EDGE_RISING) )
				iiow(db, DM9051_INTCR, 0x00); //high active(default)
				//iiow(db, DM9051._INTCR, 0x01);  //test
			//#elif defined DM_CONF_INTERRUPT_TEST_DTS_RISING
			//	iiow(db, DM9051._INTCR, 0x00); //high active(default)
			else
				iiow(db, DM9051_INTCR, 0x01); //low active
				//iiow(db, DM9051._INTCR, 0x00); //test
			//#endif
			
		#else
			#if (DRV_IRQF_TRIGGER == IRQF_TRIGGER_LOW)
			iiow(db, DM9051_INTCR, 0x01); //low active
			#else
			iiow(db, DM9051_INTCR, 0x00); //high active(default)
			#endif
		#endif
		//.iiow(db, DM9051_IMR, IMR_PAR | IMR_PTM | IMR_PRM); //...
#else
	     	//.iiow(db, DM9051_IMR, IMR_PAR);
#endif
		//..iiow(db, DM9051_IMR, IMR_PAR); //=int_reg_stop(db); 
	     	//iiow(db, DM9051_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	     	//iiow(db, DM9051_RCR, db->rcr_all);
	     	#endif
		bcopen_rx_info_clear(&db->bC);
		DM9051_fifo_reset_flg = 1;
	     	return; 
     }
     if (state==1 || state==2 || state==3 || state==5)
     {
		if (hstr)
		{
			db->rx_rst_quan = 0;
			++db->bC.DO_FIFO_RST_counter;
			Disp_RunningEqu_Ending(db);

			if (state == 1)
#ifdef ON_RELEASE
				rel_printk2("dm9-%s Len %03d RST_c %d\n", hstr, db->bC.RxLen, db->bC.DO_FIFO_RST_counter);
#else
				printk("dm9-%s Len %03d RST_c %d\n", hstr, db->bC.RxLen, db->bC.DO_FIFO_RST_counter); //"LenNotYeh", " %d", db->bC.DO_FIFO_RST_counter
#endif
			else if (state == 3)
			{
#ifdef ON_RELEASE
				rel_printk2("dm9-%s Len %03d RST_c %d (RXBErr %d)\n", hstr, db->bC.RxLen, 
						db->bC.DO_FIFO_RST_counter, db->bC.RXBErr_counter);
#else
				if (db->mac_process)          
					printk("dm9-%s Len %03d RST_c %d (RXBErr %d)\n", hstr, db->bC.RxLen, 
						db->bC.DO_FIFO_RST_counter, db->bC.RXBErr_counter);
				else
					printlog("dm9-%s Len %03d RST_c %d (RXBErr %d)\n", hstr, db->bC.RxLen, 
						db->bC.DO_FIFO_RST_counter, db->bC.RXBErr_counter);
#endif
			}
			else  if (state == 2) // STATE 2
			{
#ifdef ON_RELEASE
				rel_printk2("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter);
#else
				if (db->mac_process)          
					printk("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter);
				else
					printlog("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter); //"Len %03d ", db->bC.RxLen
#endif
			}
			else // STATE 5
			{
#ifdef ON_RELEASE
				rel_printk2("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter);
#else
				if (db->mac_process)          
					printk("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter);
				else
					printlog("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter);
#endif
			}
		}
		dm9051_reset(db);
		#if DEF_SPIRW
		iiow(db, DM9051_FCR, FCR_FLOW_ENABLE);	/* Flow Control */
		if (hstr)
			iiow(db, DM9051_PPCR, PPCR_SETTING); /* Fully Pause Packet Count */
		else
		{
			pk= ior(db, DM9051_PPCR);
			iow(db, DM9051_PPCR, PPCR_SETTING); /* Fully Pause Packet Count */
		}
#ifdef DM_CONF_POLLALL_INTFLAG	
		#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
			//#if defined DM_CONF_INTERRUPT_TEST_DTS_FALLING
			//	iiow(db, DM9051._INTCR, 0x01); //low active
			if (db->irq_type & (IIRQ_TYPE_LEVEL_HIGH | IIRQ_TYPE_EDGE_RISING) )
				iiow(db, DM9051_INTCR, 0x00); //high active(default)
				//iiow(db, DM9051_INTCR, 0x01); //test
			//#elif defined DM_CONF_INTERRUPT_TEST_DTS_RISING
			//	iiow(db, DM9051._INTCR, 0x00); //high active(default)
			else
				iiow(db, DM9051_INTCR, 0x01); //low active
				//iiow(db, DM9051_INTCR, 0x00); //test
			//#endif	
		#else
			#if (DRV_IRQF_TRIGGER == IRQF_TRIGGER_LOW)
			iiow(db, DM9051_INTCR, 0x01); //low active
			#else
			iiow(db, DM9051_INTCR, 0x00); //high active(default)
			#endif
		#endif
	     	//.iiow(db, DM9051_IMR, IMR_PAR | IMR_PTM | IMR_PRM);
#else
	     	//.iiow(db, DM9051_IMR, IMR_PAR);
#endif
			//..iiow(db, DM9051_IMR, IMR_PAR); //=int_reg_stop(db); 
	     	//iiow(db, DM9051_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	     	//iiow(db, DM9051_RCR, db->rcr_all);
		#endif
		bcopen_rx_info_clear(&db->bC);
		DM9051_fifo_reset_flg = 1;
	     	return; 
	 }
     //if (state==2){ 
	    //printk("------- ---end--- -------\n");
	    //printk("Hang.System.JJ..\n");
	    //while (1);
	    
	    //("------- ---NO Do_reset( xx RxbErr ) --- -------\n");
     	//	if (hstr) printk("dm9-%s Len %03d (NO RST)(RXBErr %d)\n", hstr, db->bC.RxLen, db->bC.RXBErr_counter);
	    // 	bcopen_rx_info_clear(&db->bC);
     //}
     return; 
}

// when reset: return 1
int dm9051_fifo_ERRO(int ana_test, u8 rxbyte, board_info_t *db)
{	
	char hstr[72];
	if (db->bC.rxbyte_counter==5 || /*db->bC.rxbyte_counter0==(NUMRXBYTECOUNTER-1)*/ db->bC.rxbyte_counter0==NUMRXBYTECOUNTER) {
	     
	    db->bC.RXBErr_counter++;
	    
	    #if 0 
	     //one_and_two_and_three all the same!
	     printk("RXBErr %d: %04x/%04x. rxb=%02X, rxb_cntr,cntr0 %d,%d \n", db->bC.RXBErr_counter, 
			db->rwregs[0], db->rwregs[1], rxbyte, db->bC.rxbyte_counter, db->bC.rxbyte_counter0);
	    #endif
	     
	     if (/*1*/ ana_test < 3 ) { /* tested-check-ok: if (!(db->bC.RXBErr_counter % 3)) */
	      sprintf(hstr, "dmfifo_reset( 03 RxbErr ) rxb=%02X .%04x/%04x", rxbyte, db->rwregs[0], db->rwregs[1]);
	      dm9051_fifo_reset(3, hstr, db);
	     
		  //u16 calc= dm9051_rx_cap(db);
	      //printk("( RxbErr_cnt %d ) %d ++ \n", db->bC.RXBErr_counter, db->bC.rxbyte._counter0_to_prt);
	      //printk("rxb=%02X rxWrRd .%02x/%02x (RO %d.%d%c)\n", rxbyte, 
	      //  db->rwregs[0], db->rwregs[1], calc>>8, calc&0xFF, '%');
	      //if (!(db->bC.RXBErr_counter%5))
	      //{
	      //.driver_dtxt_disp(db);
	      //.driver_dloop_disp(db);
	      //}
	      //dm9051._fifo_reset(1, "dm9051._fifo_reset( RxbEr )", db);
	     
	      dm9051_fifo_reset_statistic(db);
	      return 1;
	     } 
	     else {                                                                                                                                                                                                                                                              
		if (db->mac_process)
		{
			sprintf(hstr, "Or. Do_reset( 02 RxbErr, macError-clear ) rxb=%02X .%04x/%04x", rxbyte, db->rwregs[0], db->rwregs[1]); //printk("macError {Just treat as a normal-unicast.}\n"); //, Get recover-clear
			dm9051_fifo_reset(5, hstr, db);
		}
		else
		{
			sprintf(hstr, "Or. Do_reset( 02 RxbErr ) rxb=%02X .%04x/%04x", rxbyte, db->rwregs[0], db->rwregs[1]);	
			dm9051_fifo_reset(2, hstr, db); //bcopen_rx_info_.clear(&db->bC); // as be done in 'dm9051._fifo_reset'
		}
		db->mac_process = 0;
	     }
	}
	return 0;
} 
#endif

#if DEF_PRO
#define DM9051_PHY		0x40	/* PHY address 0x01 */

//SPI:
// do before: mutex_lock(&db->addr_lock); | (spin_lock_irqsave(&db->statelock,flags);)
// do mid: spin_unlock_irqrestore(&db->statelock,flags);, spin_lock_irqsave(&db->statelock,flags);
// do after: (spin_unlock_irqrestore(&db->statelock,flags);) | mutex_unlock(&db->addr_lock);
static int dm9051_phy_read(struct net_device *dev, int phy_reg_unused, int reg)
{
	board_info_t *db = netdev_priv(dev);
	int ret;

	/* Fill the phyxcer register into REG_0C */
	iiow(db, DM9051_EPAR, DM9051_PHY | reg);
	iiow(db, DM9051_EPCR, EPCR_ERPRR | EPCR_EPOS);	/* Issue phyxcer read command */

	//dm9051_msleep(db, 1);		/* Wait read complete */
	//= 
	while ( ior(db, DM9051_EPCR) & EPCR_ERRE) ;

	iiow(db, DM9051_EPCR, 0x0);	/* Clear phyxcer read command */
	/* The read data keeps on REG_0D & REG_0E */
	ret = (ior(db, DM9051_EPDRH) << 8) | ior(db, DM9051_EPDRL);
	return ret;
}

static void dm9051_phy_write(struct net_device *dev,
		 int phyaddr_unused, int reg, int value)
{
	board_info_t *db = netdev_priv(dev);

	printk("iowPHY[%02d %04x]\n", reg, value);
	/* Fill the phyxcer register into REG_0C */
	iow(db, DM9051_EPAR, DM9051_PHY | reg);
	/* Fill the written data into REG_0D & REG_0E */
	iiow(db, DM9051_EPDRL, value);
	iiow(db, DM9051_EPDRH, value >> 8);
	iow(db, DM9051_EPCR, EPCR_EPOS | EPCR_ERPRW);	/* Issue phyxcer write command */

	//dm9051_msleep(db, 1);		/* Wait write complete */
	//= 
	while ( ior(db, DM9051_EPCR) & EPCR_ERRE) ;

	iow(db, DM9051_EPCR, 0x0);	/* Clear phyxcer write command */
}

static int dm9051_phy_read_lock(struct net_device *dev, int phy_reg_unused, int reg)
{
	int val;
	board_info_t *db = netdev_priv(dev);
	mutex_lock(&db->addr_lock);
	val= dm9051_phy_read(dev, 0, reg);
	mutex_unlock(&db->addr_lock);
	return val;
}
static void dm9051_phy_write_lock(struct net_device *dev, int phyaddr_unused, int reg, int value)
{
	board_info_t *db = netdev_priv(dev);
	mutex_lock(&db->addr_lock);
	dm9051_phy_write(dev, 0, reg, value);
	mutex_unlock(&db->addr_lock);
}
#endif

#define dm9051_spi_read_reg     dm9.iorb
#define dm9051_spi_write_reg     dm9.iowb

//[return 1 ok]
static int device_polling(board_info_t * db, u8 erre_bit, u8 expect) {
  int i;
  u8 tmp;
  for (i = 0; i < 1000; i++) {
    mdelay(1); //delay
    tmp = dm9051_spi_read_reg(db, DM9051_EPCR);
    if ((tmp & erre_bit) == expect) //ready
      break;
  }
  if (i == 1000) {
    printk("[dm9 read.write eeprom time out] on polling bit : 0x%02x (but want 0x%02x)\n", tmp, expect);
    return 0;
  }
  return 1; //OK
}

#if 1
//[of spi_user.c(used by 'dm9051_ethtool_ops')]
static void dm9051_read_eeprom(board_info_t *db, int offset, u8 *to)
{
#if DEF_SPIRW        
        mutex_lock(&db->addr_lock);

        dm9051_spi_write_reg(db, DM9051_EPAR, offset);
        dm9051_spi_write_reg(db, DM9051_EPCR, EPCR_ERPRR);

        device_polling(db, EPCR_ERRE, 0x00); //while ( dm9051_spi_read_reg(db, DM9051_EPCR) & EPCR_ERRE) ;

        dm9051_spi_write_reg(db, DM9051_EPCR, 0x0);

        to[0] = dm9051_spi_read_reg(db, DM9051_EPDRL);
        to[1] = dm9051_spi_read_reg(db, DM9051_EPDRH);

        mutex_unlock(&db->addr_lock);
#endif        
}

/*
 * Write a word data to SROM
 */
static void dm9051_write_eeprom(board_info_t *db, int offset, u8 *data)
{
#if DEF_SPIRW       
		int pr; 
        mutex_lock(&db->addr_lock);

        dm9051_spi_write_reg(db, DM9051_EPAR, offset);
        dm9051_spi_write_reg(db, DM9051_EPDRH, data[1]);
        dm9051_spi_write_reg(db, DM9051_EPDRL, data[0]);
        dm9051_spi_write_reg(db, DM9051_EPCR, EPCR_WEP | EPCR_ERPRW);

        pr = device_polling(db, EPCR_ERRE, 0x00); //while ( dm9051_spi_read_reg(db, DM9051_EPCR) & EPCR_ERRE) ;

        dm9051_spi_write_reg(db, DM9051_EPCR, 0);
		  if (pr) {
			printk("dm9 [write Word %d][polling OK] : %02x %02x\n", offset, data[0], data[1]);
		  }

        mutex_unlock(&db->addr_lock);

		  //[my delay]
		  mdelay(1); //delay
		  mdelay(2); //delay
		  mdelay(3); //delay
#endif        
}
#endif

#if DEF_PRO
unsigned dm9051_chipid(board_info_t * db)
{
	unsigned  chipid = 0;
#if DEF_SPIRW
	chipid= ior(db, DM9051_PIDL); //printk("ior %02x [DM9051_PIDL= %02x]\n", DM9051_PIDL, chipid);
	chipid |= (unsigned)ior(db, DM9051_PIDH) << 8; //printk("+ ior %02x [DM9051_PIDH <<8] = %04x\n", DM9051_PIDH, chipid);
	if (chipid == (DM9051_ID>>16))
		return chipid;
	
	chipid= ior(db, DM9051_PIDL); //printk("ior %02x [DM9051_PIDL] = %02x\n", DM9051_PIDL, chipid);
	chipid |= (unsigned)ior(db, DM9051_PIDH) << 8; //printk("+ ior %02x [DM9051_PIDH <<8] = %04x\n", DM9051_PIDH, chipid);
	if (chipid == (DM9051_ID>>16))
		return chipid;
	
	chipid= ior(db, DM9051_PIDL); //printk("ior %02x [DM9051_PIDL= %02x]\n", DM9051_PIDL, chipid);
	chipid |= (unsigned)ior(db, DM9051_PIDH) << 8; //printk("+ ior %02x [DM9051_PIDH <<8] = %04x\n", DM9051_PIDH, chipid);
	if (chipid == (DM9051_ID>>16))
		return chipid;
#endif
	return chipid;
}
#endif

#if DEF_OPE
static void dm9051_init_dm9051(struct net_device *dev)
{
	board_info_t *db = netdev_priv(dev);
#if DEF_SPIRW	
	int	phy4;
	iiow(db, DM9051_GPCR, GPCR_GEP_CNTL);	/* Let GPIO0 output */
	
	/* dm9051_reset(db); */

/* DBG_20140407 */
  phy4= dm9051_phy_read(dev, 0, MII_ADVERTISE);	
  dm9051_phy_write(dev, 0, MII_ADVERTISE, phy4 | ADVERTISE_PAUSE_CAP);	/* dm95 flow-control RX! */	
  dm9051_phy_read(dev, 0, MII_ADVERTISE);

	/* Program operating register */
	iow(db, DM9051_TCR, 0);	        /* TX Polling clear */
	iiow(db, DM9051_BPTR, 0x3f);	/* Less 3Kb, 200us */
	iiow(db, DM9051_SMCR, 0);        /* Special Mode */
	/* clear TX status */
	iiow(db, DM9051_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
	iow(db, DM9051_ISR, ISR_CLR_STATUS); /* Clear interrupt status */
#endif
	/* Init Driver variable */
	db->imr_all = IMR_PAR | IMR_PRM; /* "| IMR_PTM" */
#ifdef JABBER_PACKET_SUPPORT
	db->rcr_all= RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN | RCR_DIS_WATCHDOG_TIMER;
#else	
	db->rcr_all= RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN;
#endif

	/*
	 * (Set address filter table) 
	 * After.call.ndo_open
	 * "kernel_call.ndo_set_multicast_list.later".
	*/
    //(1)
    #if DM_CONF_APPSRC
    dm9051_fifo_reset(1, NULL, db); // 'NULL' for reset FIFO, and no increase the RST counter
    #endif
    int_reg_stop(db); //iiow(db, DM9051_IMR, IMR_PAR); //= int_reg_stop()
}

static void dm9051_open_code(struct net_device *dev, board_info_t *db) // v.s. dm9051_probe_code()
{
	//[(db->chip_code_state==CCS_NUL)].OK.JJ
	
    /* Note: Reg 1F is not set by reset */
#if DEF_SPIRW
    iow(db, DM9051_GPR, 0);	/* REG_1F bit0 activate phyxcer */
#endif
    mdelay(1); /* delay needs by DM9051 */ 
	
    /* Initialize DM9051 board */
    dm9051_reset(db);
	dm9051_init_dm9051(dev);
}
#endif

#if DEF_STO
static void dm9051_stop_code(struct net_device *dev, board_info_t *db) // v.s. dm9051_probe_code()
{
#if DEF_SPIRW	
	mutex_lock(&db->addr_lock);
	dm9051_phy_write(dev, 0, MII_BMCR, BMCR_RESET);	/* PHY RESET */
	iow(db, DM9051_GPR, 0x01);	/* Power-Down PHY */
	//int._reg_stop(db); //iow(db, DM9051_IMR, IMR_PAR);	/* Disable all interrupt */
	iow(db, DM9051_RCR, RCR_RX_DISABLE);	/* Disable RX */
	mutex_unlock(&db->addr_lock);
#endif
}
#endif
