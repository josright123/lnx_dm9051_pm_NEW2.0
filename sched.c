/* [Schedule Work to operate SPI rw] */

/*(or DM9051_Schedule.c)*/
/*static int dm9051_sch_cnt(u16 ChkD) // large equ +50 or less -3
{
	static u16 SavC= 0;
	
	if (SavC != ChkD) {
		
		u16 LessC= 0;
		if (SavC > 3)
			LessC= SavC - 3;
		
		if (ChkD < LessC) { //SavC
			SavC= ChkD;
			return 1; //less and reduce
		}
		
		if (ChkD >= (SavC+50)) {
			SavC= ChkD;
			return 1;
		}
	}
	return 0;
}*/
static int dm9051_sch_cnt1(u16 getVAL, u16 offset) // large equ +1 (as increasment)
{
	static u16 Sav1= 0;
	
	if (!offset) // an option that always no need printed, so return 0. 
		return 0;
	
	//offset default is as 1
	//if (Sav1 != getVAL) {
		if (getVAL >= (Sav1+offset)) {
			Sav1= getVAL;
			return 1;
		}
	//}
	return 0;
}

//Testing...JJ5_DTS
#define GLUE_LICENSE_PHYPOLL		(3+2)
#define GLUE_LICENSE_INT			(3+1)
#define GLUE_LICENSE_LE_EXPIRE		(3-1)

static void 
dm9051_INTPschedule_isr(board_info_t *db, int sch_cause)
{
	//spin_lock(&db->statelock_tx_rx);//mutex_lock(&db->addr_lock);
	
	//.printk("R_SCH_XMIT %d (=%d) dm9051_start_xmit, Need skb = skb_dequeue(&db->txq) to get tx-data\n", R_SCH_XMIT, sch_cause);
	if (dm9051_sch_cnt1(db->nSCH_XMIT, 0)) //1500, 5500, 0
		printk("dm9-INFO TX %02d, sched R_SCH_XMIT %d (=%d) send skb_dequeue(txq).. pre.nRX %d\n", db->nSCH_XMIT, R_SCH_XMIT, sch_cause, db->rx_count);
	
	db->sch_cause = sch_cause;
	
	#ifdef DM_CONF_POLLALL_INTFLAG
	if (sch_cause== R_SCH_INIT)
		return;
	if (sch_cause== R_SCH_INT_GLUE)
		return;
	//if (sch_cause== R_SCH._INFINI) 
	//	return;
	#endif
	
	switch (sch_cause)
	{
		case R_SCH_INIT:
			db->nSCH_INIT++; // if (m<250) m++; 
		//	schedule_delayed_work(&db->rx._work, 0); //dm9051_continue_poll
			break;
		case R_SCH_INFINI:
			db->nSCH_INFINI++;
		//	schedule_delayed_work(&db->rx._work, 0);  //dm9051_continue_poll
			break;
		//case R_SCH_LINK:
			//break;
		case R_SCH_PHYPOLL:
			break;
		case R_SCH_INT:
			db->nSCH_INT++;
		//	schedule_delayed_work(&db->rx._work, 0); //dm9051_continue_poll 
			break;
		case R_SCH_INT_GLUE:
			db->nSCH_INT_Glue++;
	#ifdef DM_CONF_POLLALL_INTFLAG		
			DM9051_int_token++; //.DM9051_int_token++;
	#endif	
			break;
		case R_SCH_XMIT:
	        //db->nSCH_XMIT++;
	#ifdef DM_CONF_POLLALL_INTFLAG	
			DM9051_int_token++;
	#endif			
			//.printk("(%d)dm9051_start_xmit, Need skb = skb_dequeue(&db->txq) to get tx-data\n", db->nSCH_XMIT);
			break;
	}
	
	#ifdef DM_CONF_TASKLET
	switch (sch_cause)
	{
		case R_SCH_INIT:
		case R_SCH_INFINI:
		//case R_SCH_LINK:
		case R_SCH_INT:
			tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0);
			break;
		case R_SCH_INT_GLUE:
			   tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0); 
			break;		
		case R_SCH_PHYPOLL:
#ifdef MORE_DM9051_MUTEX
			   tasklet_schedule(&db->phypoll_tl); //schedule_.delayed_work(&db->x, 0);
#else
			   tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0); 
#endif			   
			break;		
	#ifdef DM_CONF_POLLALL_INTFLAG		
		case R_SCH_XMIT:
#ifdef MORE_DM9051_MUTEX
			tasklet_schedule(&db->xmit_tl); //schedule_.delayed_work(&db->y, 0);
#else
			tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0);
#endif			   
			break;
	#endif			 	
	}
	#else //~DM_CONF_TASKLET
#if 1	
	//spin_lock(&db->statelock_tx_rx);//mutex_lock(&db->addr_lock);
	switch (sch_cause)
	{
  #ifdef DM_CONF_INTERRUPT
		#ifndef DM_CONF_THREAD_IRQ
		case R_SCH_INT:
			schedule_delayed_work(&db->rx_work, 0); 
			break;
		#endif
  #else
		case R_SCH_INIT:
		case R_SCH_INFINI: //'POLLING'
		case R_SCH_INT_GLUE:
		//case R_SCH_LINK:
			//schedule_delayed_work(&db->rx_work, 0); //dm9051_continue_poll
			//break;
			 schedule_delayed_work(&db->rx_work, 0); 
			break;		
  #endif
  
		case R_SCH_PHYPOLL:
#ifdef MORE_DM9051_MUTEX
			   schedule_delayed_work(&db->phypoll_work, 0);
#else
			   schedule_delayed_work(&db->rx_work, 0); 
#endif			   
			break;		
			
		#ifdef DM_CONF_INTERRUPT
		case R_SCH_XMIT:
		#ifdef MORE_DM9051_MUTEX
			schedule_delayed_work(&db->xmit_work, 0);
		#else
			schedule_delayed_work(&db->rx_work, 0); //dm9051_continue_poll
			   //[OR] schedule_delayed_work(&db->tx_work, 0); //(dm9051_tx_work) This which need tryLOck() or Mutex() design.
		#endif			   
		break;
		#endif			 	
		/* 0, Because @delay: number of jiffies to wait or 0 for immediate execution */
	}
	//spin_unlock(&db->statelock_tx_rx);//mutex_unlock(&db->addr_lock);
#endif	
	//spin_unlock(&db->statelock_tx_rx);//mutex_unlock(&db->addr_lock);
	#endif
}

 #ifdef DM_CONF_POLLALL_INTFLAG
 #elif DRV_POLL_1
static void
dm9051_INTPschedule_weight(board_info_t *db, unsigned long delay)
{
	static int sd_weight = 0;
	
	#ifdef DM_CONF_TASKLET
	if (db->DERFER_rwregs[MD_ReadPTR] != db->DERFER_rwregs1[MD_ReadPTR]) {
		tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0); 
		return;
	}
	
	if (db->DERFER_rwregs1[RXM_WrtPTR] == db->DERFER_rwregs1[MD_ReadPTR]) {
		tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, delay); 
		return;
	}
	
	sd_weight++;
	if (!(sd_weight%3)) {
		if (sd_weight>=6000) /*(sd_weight>=5000) in disp no adj*/ 
			sd_weight = 0;
			
		if ( sd_weight == 0 && (db->DERFER_calc1>>8 )!= 0) // fewer disp
			printk("-[dm9 SaveCPU for: MDWA 0x%x (RO %d.%d%c)]-\n", db->DERFER_rwregs1[RXM_WrtPTR], db->DERFER_calc1>>8, db->DERFER_calc1&0xff, '%');
		
		tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, delay);  // slower ,
		return;
	}
	tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0); 
	#else //~DM_CONF_TASKLET
	
	if (db->DERFER_rwregs[MD_ReadPTR] != db->DERFER_rwregs1[MD_ReadPTR]) {
		schedule_delayed_work(&db->rx_work, 0); 
		return;
	}
		
	//good.all.readout
	if (db->DERFER_rwregs1[RXM_WrtPTR] == db->DERFER_rwregs1[MD_ReadPTR]) { //THis is also 'db->DERFER_calc1>>8 == 0'
		//mdwa = 0;
		schedule_delayed_work(&db->rx_work, delay); 
		return;
	}
			
	sd_weight++;
	if (!(sd_weight%3)) /* "slower" by more delay_work(delay) */
	/*if (!(sd_weight%5)) */
	/*if (!(sd_weight%6)) */ {
		
		//warn.(NoWrPkt)But_Read_CutCut (too slow read or rx-pointer.Err)
		/*if ((db->DERFER_calc1>>8) > 5) {
			sd_weight = 0;
			dm9051_fifo_reset(1, "dm9 (RxPoint.Err)", db);
			dm9051_fifo_reset_statistic(db);
			schedule_..delayed_work(&db->rx_work, 1);  //or 'delay'
			return;
		}*/
	
		//normal
		if (sd_weight>=6000) /*(sd_weight>=5000) in disp no adj*/ 
			sd_weight = 0;
			
		if ( sd_weight == 0 && (db->DERFER_calc1>>8 )!= 0) // fewer disp
			printk("-[dm9 SaveCPU for: MDWA 0x%x (RO %d.%d%c)]-\n", db->DERFER_rwregs1[RXM_WrtPTR], db->DERFER_calc1>>8, db->DERFER_calc1&0xff, '%');
		
		schedule_delayed_work(&db->rx_work, delay);  // slower ,
		return;
	}
	schedule_delayed_work(&db->rx_work, 0); 
	#endif
}
#endif
