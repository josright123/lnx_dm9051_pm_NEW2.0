#if DEF_PRO | DEF_OPE
/*
 *  init
 */	
static void SCH_clear(board_info_t *db) //open
{
	db->rx_count= //need in open only
    db->Enter_hash=
    db->sch_cause=
	db->nSCH_INIT=
	db->nSCH_LINK=
	db->nSCH_XMIT=
	db->nSCH_GoodRX = 
	db->nSCH_UniRX =
	db->nSCH_INT= db->nSCH_INT_B=
	db->nSCH_INT_Glue=
	db->nSCH_INT_NUm= db->nSCH_INT_NUm_A=
	db->nSCH_INFINI = 
	db->mac_process = 0;
	db->nSCH_INT_Num_Disp= 100;
}
#endif

#if DEF_PRO
void Operation_clear(board_info_t *db) // probe
{
	db->rwtrace1=
	db->RUNNING_rwregs[0]=
	db->RUNNING_rwregs[1]= 0;
	
	db->bC.OvrFlw_counter=
	db->bC.ERRO_counter=
	db->bC.RXBErr_counter=
	db->bC.LARGErr_counter=
	db->bC.StatErr_counter=
    db->bC.DO_FIFO_RST_counter= 
    db->rx_rst_quan= 
    db->rx_tot_quan= 0;
}
#endif

#if DM_CONF_APPSRC
/*
 *  init (AppSrc)
 */
 #if 0
 //[called] by [skb_rx_core_old.c] 
static void bcrdy_rx_info_clear(struct rx_ctl_mach *pbc)
{
	pbc->rxbyte_counter= 
	pbc->rxbyte_counter0= 0;
}
#endif
static void bcopen_rx_info_clear(struct rx_ctl_mach *pbc)
{
	pbc->rxbyte_counter= 
	pbc->rxbyte_counter0= 
	
	pbc->rxbyte_counter0_to_prt= 
#if 0	
	pbc->rx_brdcst_counter= 
	pbc->rx_multi_counter= 
#endif	
	pbc->rx_unicst_counter= 0;
	
	pbc->isbyte= 0xff; // Special Pattern
}
#endif

void Disp_RunningEqu(board_info_t *db)
{						
	if (db->RUNNING_rwregs[0]==db->rwregs[0] && db->RUNNING_rwregs[1]==db->rwregs[1])
	{		
		char s[50];
		if (db->bC.rxbyte_counter0_to_prt < 2)
		{
			//ISR 80 wrRd 3902/39fe (RO 98.0%) (RXB_00H Be ISR-PRS) (733 ++) @ rxb_cntr0 1
			//ISR 80 wrRd 3902/39fe (RO 98.0%) (RXB_00H No ISR-PRS) (733 ++) @ rxb_cntr0 1
			//ISR 80 wrRd 3902/39fe (RO 98.0%) Equ&(RXB_00H Be ISR-PRS)Impossible (733 ++) @ rxb_cntr0 1

			 if ((db->rwregs[0]!=db->rwregs[1]) && (db->bC.isbyte & ISR_PRS))
				sprintf(s, "(rxb00_cnt %02d Diff& Be ISR-PRS)Rare-case", db->bC.rxbyte_counter0); //printk("dm9_when: Points_diff_and_ISR-PRS \n");
			 else if (db->rwregs[0]!=db->rwregs[1])
				sprintf(s, "Diff(rxb00_cnt %02d  but no ISR-PRS)", db->bC.rxbyte_counter0); //printk("dm9_when: Points_diff only. No ISR-PRS\n");
			 else
				sprintf(s, "(rxb00_cnt %02d Equ& Be ISR-PRS)Impossible", db->bC.rxbyte_counter0); //printk("dm9_when: ISR-PRS only. Points-equ (Impossible!)\n"); //Impossible!	
				
			 printk("dm9-IOR wrRd %04x/%04x (RO %d.%d%c) ISR %02x %s (%2d ++)\n",
			   db->rwregs[0], db->rwregs[1], db->calc>>8, db->calc&0xFF, '%', db->bC.isbyte, s,
			   db->bC.rxbyte_counter0_to_prt);	
	   }
	   db->bC.rxbyte_counter0_to_prt += 1;
	}
	db->RUNNING_rwregs[0]= db->rwregs[0];
	db->RUNNING_rwregs[1]= db->rwregs[1];
}

void Disp_RunningEqu_Ending(board_info_t *db)
{
	char *s;
	if (db->bC.rxbyte_counter0_to_prt >= 2)
	{
	  if ((db->rwregs[0]!=db->rwregs[1]) && (db->bC.isbyte & ISR_PRS))
		s= "(---Accumulat times---)Rare-case";
	  else if (db->rwregs[0]!=db->rwregs[1])
		s= "(---Accumulat times---)Diff";
	  else
		s= "(---Accumulat times---)Impossible";

	  printk("dm9-IOR wrRd %04x/%04x (RO %d.%d%c) ISR %02x rxb= %02x %s (%2d ++)\n",
	    db->rwregs[0], db->rwregs[1], db->calc>>8, db->calc&0xFF, '%', db->bC.isbyte, db->bC.dummy_pad, s,
	    db->bC.rxbyte_counter0_to_prt);
	}
}

/*
 *  disp
 */
static void dm9051_fifo_reset_statistic(board_info_t *db)
{
	if (!(db->bC.DO_FIFO_RST_counter%10)) {
		rel_printk1("dm9-Mac_OvFlwEr.Rxb&LargEr RST_c %d\n", db->bC.DO_FIFO_RST_counter);
		rel_printk1(" %d %d.%d %d\n", 
			db->bC.ERRO_counter, db->bC.OvrFlw_counter, db->bC.RXBErr_counter, db->bC.LARGErr_counter);
		if (db->bC.StatErr_counter)
			rel_printk1("dm9-RareFnd StatEr %d\n", db->bC.StatErr_counter);
	}
}

// --- const, extern function and varibles---
// ---  evt-queue, event & sched  ---

#if DM9051_CONF_TX 	
static u16 check_cntStop(board_info_t *db)
{
#if 0	
	u16 cs;
	while (!spin_trylock(&db->statelock_tx1_rx1)) ; //if(!)
	cs = db->bt.prob_cntStopped;
	spin_unlock(&db->statelock_tx1_rx1); 
	return cs;
#endif	
	return (!skb_queue_empty(&db->txq));
	
}

static void opening_wake_queue1(struct net_device *dev) //( u8 flag)
{
#if 0	
	board_info_t *db= netdev_priv(dev);
	
	while (!spin_trylock(&db->statelock_tx1_rx1)) ; //if(!)
	if (db->bt.prob_cntStopped)
	{
		db->bt.prob_cntStopped= 0;
		netif_wake_queue(dev);
	}
	spin_unlock(&db->statelock_tx1_rx1);
#endif

	board_info_t *db= netdev_priv(dev);
	if (db->bt.prob_cntStopped) {
		db->bt.prob_cntStopped= 0;
		netif_wake_queue(dev);
	}
}

static void toend_stop_queue1(struct net_device *dev, u16 stop_cnt)
{
#if 0	
	board_info_t *db= netdev_priv(dev);	
	while (!spin_trylock(&db->statelock_tx1_rx1)) ; //if(!)
	switch(stop_cnt)
	{
		case 1:
		db->bt.prob_cntStopped++;
		break;
		case NUM_QUEUE_TAIL:
		default:
		db->bt.prob_cntStopped= stop_cnt;
		break;
	}
	spin_unlock(&db->statelock_tx1_rx1); 
	
	if (stop_cnt<NUM_QUEUE_TAIL)
		return; // true;
	if (stop_cnt==NUM_QUEUE_TAIL)
	{
	  	netif_stop_queue(dev);
		return; // true;
	}
	
	//.wrong path, but anyhow call stop for it
	netif_stop_queue(dev);
	printk("[.wrong path]: WARN, anyhow call stop for it .. ");
	printk("(cntStop %d)\n", db->bt.prob_cntStopped);
	driver_dtxt_disp(db); // OPTIONAL CALLED
	return; // false;
#endif	

	board_info_t *db= netdev_priv(dev);	
	switch(stop_cnt) {
		case 1:
		  db->bt.prob_cntStopped++;
		  break;
		case NUM_QUEUE_TAIL:
		  db->bt.prob_cntStopped= stop_cnt;
		  break;
	}	
	if (stop_cnt==NUM_QUEUE_TAIL)
	  	netif_stop_queue(dev);
}
#endif

//..
void rx_mutex_head(board_info_t *db)
{
  #ifdef DM_CONF_POLLALL_INTFLAG	
	  mutex_lock(&db->addr_lock);
	  //.iiow(db, DM9051._IMR, IMR._PAR); // Disable all interrupts 
  #elif DRV_POLL_1
	  mutex_lock(&db->addr_lock);
  #endif
}
//..
void rx_mutex_tail(board_info_t *db)
{
  #ifdef DM_CONF_POLLALL_INTFLAG
	//.iiow(db, DM9051._IMR, db->imr._all); // Re-enable interrupt mask 
    mutex_unlock(&db->addr_lock);
  #elif DRV_POLL_1
    mutex_unlock(&db->addr_lock);
  #endif
}
