/* ops */
/* event: play a schedule starter in condition */
//static netdev_tx_t 
//DM9051_START_XMIT(struct sk_buff *skb, struct net_device *dev) //void sta_xmit_sched_delay_work(board_info_t * db)
//{
//	return NETDEV_TX_OK;
//}

//..ok //*************************************************************//
/*  Set DM9051 multicast address */
static void
dm_hash_table_unlocked(struct net_device *dev)
{
	board_info_t *db = netdev_priv(dev);
#ifdef JABBER_PACKET_SUPPORT
	u8 rcr = RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN | RCR_DIS_WATCHDOG_TIMER;
#else	
	u8 rcr = RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN;
#endif	
#if DEF_SPIRW
	struct netdev_hw_addr *ha;
	int i, oft;
	u32 hash_val;
	u16 hash_table[4];
	for (i = 0, oft = DM9051_PAR; i < 6; i++, oft++)
		iiow(db, oft, dev->dev_addr[i]);

	/* Clear Hash Table */
	for (i = 0; i < 4; i++)
		hash_table[i] = 0x0;

	/* broadcast address */
	hash_table[3] = 0x8000;

	if (dev->flags & IFF_PROMISC)
		rcr |= RCR_PRMSC;

	if (dev->flags & IFF_ALLMULTI)
		rcr |= RCR_ALL;

	/* the multicast address in Hash Table : 64 bits */
	netdev_for_each_mc_addr(ha, dev) {
		hash_val = ether_crc_le(6, ha->addr) & 0x3f;
		hash_table[hash_val / 16] |= (u16) 1 << (hash_val % 16);
	}

	/* Write the hash table */
	for (i = 0, oft = DM9051_MAR; i < 4; i++) {
		iiow(db, oft++, hash_table[i]);
		iiow(db, oft++, hash_table[i] >> 8);
	}

	iow(db, DM9051_RCR, rcr);
#endif	
	db->rcr_all= rcr;
/*
//TEST
	db->rcr_all |= RCR_PRMSC | IFF_ALLMULTI;
	printk("Test db->rcr_all from %02x to %02x\n", rcr, db->rcr_all);
*/	
}
static void 
dm_hash_table(board_info_t *db)
{
	struct net_device *dev = db->ndev; //board_info_t *db = netdev_priv(dev);
	mutex_lock(&db->addr_lock);
	dm_hash_table_unlocked(dev);	
    mutex_unlock(&db->addr_lock);
}

static void 
rx_mutex_hash_table(board_info_t *db)
{
	if (db->Enter_hash)	
	{
		dm_hash_table(db);
		db->Enter_hash= 0;
	}
}

	#ifdef DM_CONF_TASKLET
	static void 
	dm_hash_table_task(unsigned long data)
	{
		board_info_t *db = (board_info_t *) data;
		db->Enter_hash= 1;
	}
	#else //~DM_CONF_TASKLET
	static void 
	dm_hash_table_work(struct work_struct *work)
	{
		board_info_t *db = container_of(work, board_info_t, rxctrl_work);
		db->Enter_hash= 1;
		//board_info_t *db = container_of(work, board_info_t, rxctrl_work);
		//dm_hash_table(db); //struct net_device *dev = db->ndev;
	}
	#endif

void sched_work(board_info_t *db)
{
	#ifdef DM_CONF_TASKLET
	tasklet_schedule(&db->rxctrl_tl);
	#else //~DM_CONF_TASKLET
	#if 1
	/*[DM9051_Schedule.c]*/
	/* spin_lock/spin_unlock(&db->statelock); no need */
	schedule_work(&db->rxctrl_work);
	#endif
	#endif
}

//..ok //*************************************************************//
#ifdef DM_CONF_PHYPOLL
#if 0
void dm_netdevice_carrier(board_info_t *db)
{
	struct net_device *dev = db->ndev;
	u8 nsr;
	int link;	
	mutex_lock(&db->addr_lock);
	nsr= iior(db, DM9051_NSR); 
	mutex_unlock(&db->addr_lock);	
	link= !!(nsr & NSR_LINKST);
	db->link= link;       //JJ-Add, Rasp-save
	if (netif_carrier_ok(dev) != link) {
		if (link)
		  netif_carrier_on(dev);
		else
		  netif_carrier_off(dev);
		printk("[DM9051.continue_poll] Link Status is: %d\n", link);
	}
}
#endif

int db_phy = 0;
int nAll_run_gap = 0;

void dm_schedule_phy(board_info_t *db)
{  
	#ifdef DM_CONF_TASKLET
	tasklet_schedule(&db->phy_poll_tl);
	#else //~DM_CONF_TASKLET
  //schedule_delayed_work(&db->phy._poll, HZ * 2); to be 3 seconds instead
  //schedule_delayed_work(&db->phy._poll, HZ * 3);
	schedule_delayed_work(&db->phy_poll, HZ * 2);
	#endif
}

	#ifdef DM_CONF_TASKLET
	static void 
	dm_phy_poll_task(unsigned long data)
	{
		board_info_t *db = (board_info_t *) data;
		int a, b;
		
#ifdef DM_CONF_POLLALL_INTFLAG 
  #if defined MORE_DM9051_MUTEX && defined  MORE_DM9051_MUTEX_EXT
	mutex_lock(&db->spi_lock);
	if (!DM9051_int_en) {
		mutex_unlock(&db->spi_lock);
		goto sched_phy;
	}
	mutex_unlock(&db->spi_lock);
  #else
	if (!DM9051_int_en)
		goto sched_phy;
  #endif
#else		
	//if (!DM9051_int_en_OF_poll) goto sched_phy;
#endif	

	//debug.NOT.in_rx_work.s!
	a = (int) db->nSCH_INT_NUm;
	b = (int) db->nSCH_INT_B;
	if (a != (b + nAll_run_gap)) { 
		nAll_run_gap = a - b; // record the diff.
	}
	db_phy++; 
	//debug.NOT.in_rx_work.e!
	
	dm9051_INTPschedule_isr(db, R_SCH_PHYPOLL);  //extended-add
	
#ifdef DM_CONF_POLLALL_INTFLAG 
sched_phy:
#else
//sched_phy:
#endif
	if (netif_running(db->ndev))
	  dm_schedule_phy(db);
	}
	#else //~DM_CONF_TASKLET
	
static void 
dm_phy_poll(struct work_struct *w)
{ 
//#ifdef DM_CONF_PHYPOLL
	struct delayed_work *dw = to_delayed_work(w);
	board_info_t *db = container_of(dw, board_info_t, phy_poll);
	int a, b;
	
	//if.in.rx_work.procedure.s!
#ifdef DM_CONF_POLLALL_INTFLAG 
  #if defined MORE_DM9051_MUTEX && defined  MORE_DM9051_MUTEX_EXT
	mutex_lock(&db->spi_lock);
	if (!DM9051_int_en) {
		mutex_unlock(&db->spi_lock);
		goto sched_phy;
	}
	mutex_unlock(&db->spi_lock);
  #else
	if (!DM9051_int_en)
		goto sched_phy;
  #endif
#else		
	//if (!DM9051_int_en_OF_poll) goto sched_phy;
#endif	
	//if.in.rx_work.procedure.e!
	
	//debug.NOT.in_rx_work.s!
	a = (int) db->nSCH_INT_NUm;
	b = (int) db->nSCH_INT_B;
	if (a != (b + nAll_run_gap)) { 
		nAll_run_gap = a - b; // record the diff.
		//.printk("dm_phypol %d[run-gap %d][PHY-SCHED-rx-work-OUT_OF-INT].CHK. INT.Num %5d(dis %5d), INT.Sch= %5d(en %d).\n",
		//.	db_phy, nAll_run_gap, db->nSCH_INT_NUm, db->nSCH_INT_NUm_A, db->nSCH_INT, db->nSCH_INT_B);
	}
	db_phy++; 
	//debug.NOT.in_rx_work.e!
	
	//dm_netdevice_carrier(db);
	dm9051_INTPschedule_isr(db, R_SCH_PHYPOLL);  //extended-add
	
#ifdef DM_CONF_POLLALL_INTFLAG 
sched_phy:
#else
//sched_phy:
#endif
	if (netif_running(db->ndev))
	  dm_schedule_phy(db);
//#endif
}
  #endif
#endif

//..ok //*************************************************************//
#if DM9051_CONF_TX
static void dm9051_tx_chk(struct net_device *dev, u8 *wrptr)
{
#if 0
    printk("dm9.tx_packets %lu ", dev->stats.tx_packets);
    printk("tx(%02x %02x %02x %02x %02x %02x ", wrptr[0], wrptr[1],wrptr[2],wrptr[3],wrptr[4],wrptr[5]);
    printk("%02x %02x   %02x %02x %02x %02x ", wrptr[6], wrptr[7],wrptr[8],wrptr[9],wrptr[10],wrptr[11]);
    printk("%02x %02x\n", wrptr[12],wrptr[13]);
#endif
}
#endif

#if DM9051_CONF_TX
static int
dm9051_continue_xmit_inRX(board_info_t *db) //dm9051_continue_poll_xmit
{
		    struct net_device *dev = db->ndev;
		    int nTx= 0;

		    db->bt.local_cntTXREQ= 0;
		    db->bt.local_cntLOOP= 0;
			while(!skb_queue_empty(&db->txq)) // JJ-20140225, When '!empty'
			{
				  struct sk_buff *tx_skb;
				  int nWait = 0;
				  tx_skb = skb_dequeue(&db->txq);
				  if (tx_skb != NULL) {
					  	
#if DEF_SPIRW
					        while( (ior(db, DM9051_TCR) & TCR_TXREQ) && (++nWait < 20)) 
								;
#endif
					        if (nWait ==20)
								printk("[dm9] tx_step timeout B\n");
					        
					        //while( ior(db, DM9051_TCR) & TCR_TXREQ ) 
							//	; //driver_dtxt_step(db, 'B');
				    
					        if(db->bt.local_cntTXREQ==2)
					        {
					           //while( ior(db, DM9051_TCR) & TCR_TXREQ ) 
					            // ; //driver_dtxt_step(db, 'Z');
					           db->bt.local_cntTXREQ= 0;
					        }

						    nTx++;

#if DEF_SPIRW
							#if 0	
							if (1)
 								{
 									int i;
 									char *pb = (char *) tx_skb->data;
 									for (i=0; i<tx_skb->len; i++ )
 										*pb++ = i;
 								}
							#endif	
						    dm9051_outblk(db, tx_skb->data, tx_skb->len);
						    iow(db, DM9051_TXPLL, tx_skb->len);
						    iow(db, DM9051_TXPLH, tx_skb->len >> 8);
#ifdef JABBER_PACKET_SUPPORT						    
						    iow(db, DM9051_TCR, TCR_TXREQ | TCR_DIS_JABBER_TIMER);
#else
						    iow(db, DM9051_TCR, TCR_TXREQ);
#endif
#endif
						    dev->stats.tx_bytes += tx_skb->len;
						    dev->stats.tx_packets++;
						    /* done tx */
					        #if 1
						    dm9051_tx_chk(dev, tx_skb->data);
					        #endif
						    dev_kfree_skb(tx_skb);
				            db->bt.local_cntTXREQ++;
				            db->bt.local_cntLOOP++;
							#if 0
							  {   
					            u16 mdwr;
					            u16 txmr;
						        while (ior(db, DM9051_TCR) & TCR_TXREQ) 
									;
								
								mdwr= ior(db, 0x7a);
								mdwr |= (u16)ior(db, 0x7b) << 8;
								txmr= ior(db, 0x22);
								txmr |= (u16)ior(db, 0x23) << 8;
								printk("TX.e [txmr %03x mdwr %03x]\n", txmr, mdwr);
							}
							#endif 
				  } //if
			} //while

			#if 0 //checked ok!!
			if (db->nSCH_INT_NUm >= db->nSCH_INT_Num_Disp) { //( /*nTx>1 || */ !(db->nSCH_INT_NUm%50))
			  char *jmp_mark= " ";
			  u16 update_num_calc = ((db->nSCH_INT_NUm/100)*100) + 100;
			  db->nSCH_INT_Num_Disp += 100;
			  if (db->nSCH_INT_Num_Disp != update_num_calc) { //i.e. (db->nSCH_INT_Num_Disp < update_num_calc)
				  jmp_mark= "*";
				  db->nSCH_INT_Num_Disp = update_num_calc;
			  }
			}
			#endif
			
		    return nTx;
}
#endif

static int dm9051_tx_irx(board_info_t *db)
{
	struct net_device *dev = db->ndev;
	int nTx = 0;
#if DM9051_CONF_TX

	if (check_cntStop(db)) 
	  //if (db->bt.prob_cntStopped)  
	  // This is more exactly right!!
	{
		  #if LOOP_XMIT
		    //mutex_lock(&db->addr_lock);
		    nTx= dm9051_continue_xmit_inRX(db); //=dm9051_continue_poll_xmit
		    opening_wake_queue1(dev); 
		    //mutex_unlock(&db->addr_lock);
		  #endif //LOOP_XMIT
	}
	
#endif //DM9051_CONF_TX
	return nTx;
}

//..ok //*************************************************************//
static int dm9051_sch_cnt_chang(u16 nEnter) // large equ +1 (as increasment)
{
	static u16 nSAVE= 0xffff;
	if (nEnter != nSAVE) {
		nSAVE= nEnter;
		return 1;
	}
	return 0;
}
int rx_work_carrier(board_info_t *db)
{
	struct net_device *dev = db->ndev;
	unsigned nsr;
	int link;	
	static int try= 0;
	//ststic int ng_found = 0
	
	//if (1) {
	//[here!]
	//do {
	mutex_lock(&db->addr_lock);
#if DEF_SPIRW
		nsr= iior(db, DM9051_NSR);
#endif
		link= !!(nsr & 0x40); //& NSR_LINKST
			
		if (!link && try && !(try%250) && try<=750)
		  printk("[DM9051.carrier] nsr %02x, link= %d (try %d)\n", nsr, link, try);
		
		if (link) {
			if (db->linkA<3) 
			  db->linkA++;
		} else {
			if (db->linkA)
			  db->linkA--;
		}

		//db->linkBool= db->linkA ? 1 : 0;  //Rasp-save
		if (db->linkA) {
			db->linkBool= 1;
			try= 0; //ng_found= 0;
		} else {
			db->linkBool= 0;
			try++; //ng_found= 1;
		}

	if (db->linkBool) //(netif_carrier_ok(dev))
	{
		if (dm9051_sch_cnt_chang(db->nSCH_LINK))
		  printk("\n[DM9051.carrier] Link Status is: %d nsr %02x [SCHEDU cnt %d. try %d]\n", link, nsr, db->nSCH_LINK, try);
	}
	else
	{
		db->nSCH_LINK++;
		if (db->nSCH_LINK<3)
		  printk("[DM9051.carrier] Link Status is: %d\n", link); //"nsr %02x", nsr
	}
	
		
		if (netif_carrier_ok(dev) != db->linkBool) { //Add	
			if (db->linkBool) {
			  netif_carrier_on(dev); //db->nSCH_LINK= 0;
			} else
			  netif_carrier_off(dev);
			printk("[DM9051.phypoll] Link Status is: %d\n", link);
		}
		
	mutex_unlock(&db->addr_lock);  
	return link;
	//} while ((++try < 8) && !db->linkBool);
	//}
}

void rx_work_cyclekeep(board_info_t *db, int has_txrx) // link_sched_delay_work, INT_glue_sched_delay_work, and infini_sched_delay_work
{
	//struct net_device *dev = db->ndev;
    //if (!netif_carrier_ok(dev) && db->nSCH_LINK < 65500) 	//new-add
	//	dm9051_INTPschedule_isr(db, R_SCH_LINK);         	//new-add
  #ifdef DM_CONF_POLLALL_INTFLAG
	static u32 SSave_Num = 0;
	static u32 SSave_INT_B = 0;
	char *jmp_mark= "*";
  #endif
   
  #ifdef DM_CONF_POLLALL_INTFLAG
	if (DM9051_int_token) DM9051_int_token--;
	if (DM9051_int_token)
		dm9051_INTPschedule_isr(db, R_SCH_INT_GLUE); //again (opt-0)
		
	if (has_txrx)	
		dm9051_INTPschedule_isr(db, R_SCH_INFINI); //again (opt-0)
		
	if (db->nSCH_INT_NUm != db->nSCH_INT_B) {
		if ((SSave_Num != db->nSCH_INT_NUm) || (SSave_INT_B != db->nSCH_INT_B)) {
			#if 0
			
			//.Check ok.
			//.printk("[DM9_cyclekeep Check] INT.Num %5d(dis %5d), INT.Sch= %5d(en %d)%s\n",
			//.	db->nSCH_INT_NUm, db->nSCH_INT_NUm_A, db->nSCH_INT, db->nSCH_INT_B, jmp_mark);
				
			#endif	
			SSave_Num = db->nSCH_INT_NUm;
			SSave_INT_B= db->nSCH_INT_B;
			
			if (db->nSCH_INT_NUm > (db->nSCH_INT_B+10)) {
				jmp_mark = "**";
				db->nSCH_INT_NUm_A= db->nSCH_INT= db->nSCH_INT_B= db->nSCH_INT_NUm;
				printk("[DM9_cyclekeep ALL-SYNC-EQUAL] INT.Num %5d(dis %5d), INT.Sch= %5d(en %d)%s\n",
				  db->nSCH_INT_NUm, db->nSCH_INT_NUm_A, db->nSCH_INT, db->nSCH_INT_B, jmp_mark);
			}
		}
	}
		
  #elif DRV_POLL_1
    
	#define DM_TIMER_EXPIRE1    1  //15
	#define DM_TIMER_EXPIRE2    0  //10
	#define DM_TIMER_EXPIRE3    0

	//dm9051_INTPschedule_isr(db, R_SCH_INFINI);
	//=
	// schedule_delayed_work(&db->rx_work, 0); //dm9051_rx_work
	
	if (db->DERFER_rwregs[RXM_WrtPTR] == db->DERFER_rwregs1[RXM_WrtPTR])
		dm9051_INTPschedule_weight(db, DM_TIMER_EXPIRE1);
	else {
		//if ((db->DERFER_calc1>>8) < 50)
		//	schedule_delayed_work(&db->rx_work, DM_TIMER_EXPIRE2); // slow ,
		//else
	#ifdef DM_CONF_TASKLET
			tasklet_schedule(&db->rx_tl);
	#else //~DM_CONF_TASKLET
			schedule_delayed_work(&db->rx_work,  DM_TIMER_EXPIRE3); // faster ,
	#endif
	}
	
  #endif
}

void IMR_DISABLE(board_info_t *db)
{
#ifdef DM_CONF_POLLALL_INTFLAG	
	if (!DM9051_int_en) { // Note.ok. 
		
	 //if (db->sch_cause!=R_SCH_INT) {
	//	printk("[Dbg condition: CASE-IS-IMPOSSIBLE] check (db->sch_cause!=R_SCH_INT) INTO rx-work~]\n");
	//	printk("[Dbg condition: CASE-IS-IMPOSSIBLE] list ([SCH_INIT,1][XMIT,2][INT,3][INFINI,4][GLUE,5][PHYPOLL,6]) db->sch_cause= %d\n", db->sch_cause);
	 //}
		
	 //if (db->sch_cause==R_SCH_INT) {
	 mutex_lock(&db->addr_lock);
	 int_reg_stop(db);
	 mutex_unlock(&db->addr_lock);  
	 //}
	}
#endif
}

bool ISR_RE_STORE(board_info_t *db)
{
#ifdef DM_CONF_POLLALL_INTFLAG	
	static unsigned short ctrl_rduce = 0;
	if (!DM9051_int_en)  // Note that: Come-in with 'if (db->sch_cause==R_SCH_INT)' TRUE.
	{

		#if defined WR_ISR_ENDOF_RXWORK_ONLY //to-do-check-how-to...
		mutex_lock(&db->addr_lock);
		db->bC.isbyte= ior(db, DM9051_ISR); // Got ISR
		if (db->bC.isbyte & 0x7e) 
		{

			//if (db->bC.isbyte == 0x82) ; // [only 'PT']

			if (db->bC.isbyte & 0x03) //(db->bC.isbyte & 0x01)
				ctrl_rduce++; // [with 'PT' or 'PR']
			else // somethings, BUT without PT or PR
				printk("[isr_reg] ISR= 0x%02x (somethings, BUT without PT or PR) Warn-Rare: overflow suspected\n", db->bC.isbyte);
				
			iiow(db, DM9051_ISR, db->bC.isbyte); // Clear ISR status
		}
		else 
		{
			if (db->bC.isbyte & 0x01)
				iiow(db, DM9051_ISR, db->bC.isbyte); // Clear ISR status //printk("[int_reg].e WITH PR: Wr ISR= 0x%02x\n", db->bC.isbyte);
		}
		mutex_unlock(&db->addr_lock);  
		#endif

		return true;
	}
#endif
	return false;
}

void IMR_ENABLE(board_info_t *db, int with_enable_irq)
{
	mutex_lock(&db->addr_lock);	
#ifdef DM_CONF_POLLALL_INTFLAG	
	if (!DM9051_int_en) { // Note that: Come-in with 'if (db->sch_cause==R_SCH_INT)' TRUE.
	 
      if (with_enable_irq)  {
		  
		if ((db->nSCH_INT <= DM_CONF_SPI_DBG_INT_NUM))  // || (db->nSCH_INT == 24)
			printk("[%s][%d].enable_irq\n", "dm951_irq", db->nSCH_INT); //from-"dm9051_rx_work"
		int_en(db->ndev);
      }
		
	  int_reg_start(db, "[dm9IMR]"); // "dm9IMR_irx_work", rxp no chg, if ncr-rst then rxp 0xc00 
	}
	if (DM9051_fifo_reset_flg) {
#if DEF_SPIRW
	  iiow(db, DM9051_RCR, db->rcr_all); // if ncr-rst then rx enable
#endif
	  DM9051_fifo_reset_flg = 0;
	}
#else	
	if (DM9051_fifo_reset_flg) {
	  int_reg_start(db, "[dmIMR_poll_rx_work]"); // exactly ncr-rst then rxp to 0xc00
#if DEF_SPIRW
	  iiow(db, DM9051_RCR, db->rcr_all); // exactly ncr-rst then rx enable
#endif
	  DM9051_fifo_reset_flg = 0;
	}
#endif
    mutex_unlock(&db->addr_lock);
}

//..ok //*************************************************************//

int rx_tx_isr0(board_info_t *db)
{
		struct net_device *dev = db->ndev;
		int nTX= 0, nRx= 0, n_tx_rx= 0;
		do {
			
			#ifdef DM_CONF_POLLALL_INTFLAG
			if (DM9051_int_en)  {
				nTX= dm9051_tx_irx(db);
				n_tx_rx += nTX;
			}
			else
			#else
			   //if (DM9051_int_en_OF_poll) {
			   // ...
			  //}
			  //else
			#endif
			{
				do {
				  nTX= dm9051_tx_irx(db);
				  nRx= dm9051_isr_ext2(dev); //dm9051_continue_poll_rx(db);
				  n_tx_rx += nTX;
				  n_tx_rx += nRx;
				} while(nRx);
			}
		} while (nTX || nRx);	
		return n_tx_rx;
}

static void dm9051_mutex_dm9051(board_info_t *db)
{
	//int link; link= 
	//printk("[dm9051.isr extend.s:\n");
	int	has_tx_rx= 0;
	static int dbg_first_in = 1;
	
	IMR_DISABLE(db);
	
	if (dbg_first_in) {
	  dbg_first_in = 0;
	  printk("[dm9051_rx_work] ------- 03.s.first in. ------\n");
	  //rx_mutex_head(db);
	  //dm9051_rx_cap(db); // get db->rwregs[0] & db->rwregs[1]
	  //rx_mutex_tail(db);
	  printk("[dm9051_rx_work] ------- 03.s. %x/%x. ------\n", db->rwregs[0], db->rwregs[1]);
	}

	rx_mutex_head(db);
	dm9051_disp_hdr_s_new(db);
	rx_mutex_tail(db);
	
	/* [dm9051_simple_mutex_dm9051].s.e */
	rx_work_carrier(db);
  #if 1	
	if (netif_carrier_ok(db->ndev)) {
  #endif
	do {
      rx_mutex_hash_table(db);
    
      //rx_tx_isr(db);=
      rx_mutex_head(db);
      has_tx_rx = rx_tx_isr0(db); // e.g. has_tx_rx = 0;
      rx_mutex_tail(db);
    
	} while(0);
  #if 1	  
	}
  #endif
  
	if (ISR_RE_STORE(db)) //if (IMR._ENABLE(db, 1))
		db->nSCH_INT_B++;

	rx_mutex_head(db);
    dm9051_disp_hdr_e_new(db);
	rx_mutex_tail(db);
    
	rx_work_cyclekeep(db, has_tx_rx); //[CYCLE-KEEP]
	IMR_ENABLE(db, 1);
}

static void dm9051_simple_mutex_dm9051(board_info_t *db)
{
	rx_work_carrier(db);
  #if 1	
	if (netif_carrier_ok(db->ndev)) {
  #endif
	do {
      rx_mutex_hash_table(db);
    
      //rx_tx_isr(db);=
      rx_mutex_head(db);
      /*has_tx_rx= */ rx_tx_isr0(db); // has_tx_rx = NOUSED.
      rx_mutex_tail(db);
    
	} while(0);
  #if 1	  
	}
  #endif
  
  #ifdef DM_CONF_POLLALL_INTFLAG 
  //[ASR gpio only (high trigger) raising trigger].s
  #ifdef MORE_DM9051_INT_BACK_TO_STANDBY 
   //#ifdef DM_CONF_POLLALL_INTFLAG 
	if (DM9051_int_en )
	{
  //#endif
	     mutex_lock(&db->addr_lock);	
         db->bC.isbyte= ior(db, DM9051_ISR); // Got ISR
      
		if (db->bC.isbyte & 0x01)
		{
		  iiow(db, DM9051_ISR, db->bC.isbyte); //~bhdbd~~ // Clear ISR status
		  //;printk("--- dm9 check DM9051_INT_BACK_TO_STANDBY [%d]--- \n", db->nSCH_INT);
		}      
        mutex_unlock(&db->addr_lock);  
		
   //#ifdef DM_CONF_POLLALL_INTFLAG 
	}
  //#endif
 #endif	
  //[ASR gpio only (high trigger) raising trigger].e
 #endif
}

	#ifdef DM_CONF_TASKLET
	static void dm9051_rx_task(unsigned long data) {
		board_info_t *db = (board_info_t *) data;
		#ifdef MORE_DM9051_MUTEX
		mutex_lock(&db->spi_lock);
		#endif

		dm9051_mutex_dm9051(db);
		
		#ifdef MORE_DM9051_MUTEX
		mutex_unlock(&db->spi_lock);
		#endif
	}
	#else //~DM_CONF_TASKLET
	#ifdef DM_CONF_THREAD_IRQ
	static void dm9051_rx_work_proc(board_info_t *db) {
		#ifdef MORE_DM9051_MUTEX
		mutex_lock(&db->spi_lock);
		#endif

		dm9051_mutex_dm9051(db);
		
		#ifdef MORE_DM9051_MUTEX
		mutex_unlock(&db->spi_lock);
		#endif
	}
	#else
  static void dm9051_rx_work(struct work_struct *work) { //TODO. (over-night ? result)
	struct delayed_work *dw = to_delayed_work(work);
	board_info_t *db = container_of(dw, board_info_t, rx_work);
	
#ifdef MORE_DM9051_MUTEX
	mutex_lock(&db->spi_lock);
#endif

	dm9051_mutex_dm9051(db);
	
#ifdef MORE_DM9051_MUTEX
	mutex_unlock(&db->spi_lock);
#endif
  }
  #endif
  #endif

#ifdef MORE_DM9051_MUTEX
	#ifdef DM_CONF_TASKLET
	static void dm9051_phypoll_tasklet(unsigned long data){
		board_info_t *db = (board_info_t *) data;
		mutex_lock(&db->spi_lock);
		dm9051_simple_mutex_dm9051(db); //dm9051_mutex_dm9051(db);
		mutex_unlock(&db->spi_lock);
	}
	static void dm9051_xmit_tasklet(unsigned long data){
		//[or by spin_lock_irq(&db->hwlock)/spin_unlock_irq(&db->hwlock)]
		board_info_t *db = (board_info_t *) data;
		mutex_lock(&db->spi_lock);
		dm9051_simple_mutex_dm9051(db); //dm9051_mutex_dm9051(db);
		mutex_unlock(&db->spi_lock);
	}
	#else //~DM_CONF_TASKLET
static void dm9051_phypoll_work(struct work_struct *work) {
	struct delayed_work *dw = to_delayed_work(work);
	board_info_t *db = container_of(dw, board_info_t, phypoll_work);
	mutex_lock(&db->spi_lock);
	dm9051_simple_mutex_dm9051(db); //dm9051_mutex_dm9051(db);
	mutex_unlock(&db->spi_lock);
}
static void dm9051_xmit_work(struct work_struct *work) {
	struct delayed_work *dw = to_delayed_work(work);
	board_info_t *db = container_of(dw, board_info_t, xmit_work);
	mutex_lock(&db->spi_lock);
	dm9051_simple_mutex_dm9051(db); //dm9051_mutex_dm9051(db);
	mutex_unlock(&db->spi_lock);
}
		#endif
#endif

/*[DM9051_Device_Ops.c]*/
void dm_sched_start_rx(board_info_t *db) // ==> OPEN_init_sched_delay_work
{
	if (db->driver_state!=DS_POLL) {
	   db->driver_state= DS_POLL;
	   dm9051_INTPschedule_isr(db, R_SCH_INIT); //#ifndef DM_CONF_POLLALL_INTFLAG, #endif
	}
}

//..ok //*************************************************************//

void define_delay_work(board_info_t * db)
{
	#ifdef DM_CONF_TASKLET
	tasklet_init(&db->rxctrl_tl, dm_hash_table_task,(unsigned long) db);
#ifdef DM_CONF_PHYPOLL	
	tasklet_init(&db->phy_poll_tl, dm_phy_poll_task,(unsigned long) db);
#endif
	tasklet_init(&db->rx_tl, dm9051_rx_task, (unsigned long) db);

#ifdef MORE_DM9051_MUTEX
	tasklet_init(&db->phypoll_tl, dm9051_phypoll_tasklet, (unsigned long) db);
	tasklet_init(&db->xmit_tl, dm9051_xmit_tasklet, (unsigned long) db);
#endif
	#else //~DM_CONF_TASKLET
	
	INIT_WORK(&db->rxctrl_work, dm_hash_table_work);
#ifdef DM_CONF_PHYPOLL	
	INIT_DELAYED_WORK(&db->phy_poll, dm_phy_poll);
#endif
    
    #ifndef DM_CONF_THREAD_IRQ
    INIT_DELAYED_WORK(&db->rx_work, dm9051_rx_work); //(dm9051_continue_poll); // old. 'dm9051_INTP_isr()' by "INIT_WORK"
    #endif
    
#ifdef MORE_DM9051_MUTEX
    INIT_DELAYED_WORK(&db->phypoll_work, dm9051_phypoll_work);
    INIT_DELAYED_WORK(&db->xmit_work, dm9051_xmit_work);
#endif
  #endif
}

//void sched_delay_work(board_info_t * db)  
//{  --for OPEN_init_sched_delay_work
//   --for hash(= sched_work)
//   --for sta_xmit_sched_delay_work
//}  --for link_sched_delay_work(= dm_schedule._phy), 
//  --INT_glue_sched_delay_work, and 
//  --infini_sched_delay_work

void sched_delay_work_cancel(board_info_t * db)
{
	#ifdef DM_CONF_TASKLET
	#ifdef DM_CONF_PHYPOLL
	tasklet_kill(&db->phy_poll_tl);
	#endif
	tasklet_kill(&db->rxctrl_tl);
	tasklet_kill(&db->rx_tl);
	#ifdef MORE_DM9051_MUTEX
	tasklet_kill(&db->phypoll_tl);
	tasklet_kill(&db->xmit_tl);
	#endif
	#else //~DM_CONF_TASKLET
	
	#ifdef DM_CONF_PHYPOLL
	cancel_delayed_work_sync(&db->phy_poll);
	#endif
	
//.flush_work(&db->rxctrl_work); /* stop any outstanding work */
  #ifndef DM_CONF_THREAD_IRQ
	cancel_delayed_work_sync(&db->rx_work); //flush_work(&db->rx_work);
	#endif
	
#ifdef MORE_DM9051_MUTEX
	cancel_delayed_work_sync(&db->phypoll_work);
	cancel_delayed_work_sync(&db->xmit_work);
#endif
  #endif
}

//void scded_work_flush(board_info_t * db)
//{
//}

//void temp_suspend_delay_work(board_info_t * db){}
//void temp_resume_delay_work(board_info_t * db){}
