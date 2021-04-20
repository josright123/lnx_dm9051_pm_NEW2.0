
#if 0
void dm9051_fifo_show_flatrx(char *dstr) //(--, board_info_t *db)
{
	printk("%s: ", dstr);
	/*
	u16 rwregs[2];	     
	u16 calc;
	read_rwr(db, &rwregs[0]);
	read_mrr(db, &rwregs[1]);
	calc= dm9051_calc(rwregs[0], rwregs[1]);
	
	// Show rx-occupied state
	if (dstr) printk("%s: ", dstr);
	printk("rxWrRd .%04x/%04x (RO %d.%d%c)\n", rwregs[0], rwregs[1], 
		calc>>8, calc&0xff, '%');
	*/
}
#endif

static bool chk_data(board_info_t *db, struct dm9051_rxhdr *prxhdr, u8 *rdptr)
{
	struct net_device *dev = db->ndev;
	u8 *phd= (u8 *) prxhdr;
	u8 flg_disp = 0;

	if ((phd[1] & 0x40) && (!(rdptr[0] &1)))
	{
		flg_disp = 1;
		printk("dmfifo_reset( 10 multiErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x {Just rxhdr said multi-cast ..}\n", 
				rdptr[0], rdptr[1], rdptr[2],
				phd[0], phd[1], phd[2], phd[3]);
	}
	else if (rdptr[0]!=dev->dev_addr[0] || rdptr[1]!=dev->dev_addr[1] || rdptr[2]!=dev->dev_addr[2])
	{                                     
		if ((rdptr[4]==DM_TYPE_ARPH && rdptr[5]==DM_TYPE_ARPL) && (rdptr[12]!=DM_TYPE_ARPH || rdptr[13]!=DM_TYPE_ARPL))
			; // error=fail //;;[current has rdptr[12]/rdptr[13]]
		if ((rdptr[4]==DM_TYPE_IPH && rdptr[5]==DM_TYPE_IPL) && (rdptr[12]!=DM_TYPE_IPH || rdptr[13]!=DM_TYPE_IPL)) 
			; // error=fail //;;[current has rdptr[12]/rdptr[13]]
		else if (rdptr[0]&1) //'skb->data[0]'
			return true;
			
		flg_disp = 1;
		//if (db->mac_process) { //"[ERRO.found.s]"
			//char hstr[72];
			//sprintf(hstr, "dmfifo_reset( 11 macErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x", 
			//	rdptr[0], rdptr[1], rdptr[2],
			//	phd[0], phd[1], phd[2], phd[3]);
			//db->mac_process = 0;
			//db->bC.ERRO_counter++;
			//dm9051_fifo_reset(11, hstr, db);
			//dm9051_fifo_reset_statistic(db);
			//return false;
		//} else {
			//"[ERRO.found.s treat as no-error]"
			printk("dmfifo_reset( 11 macErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x {Just treat as a normal-unicast}\n", 
				rdptr[0], rdptr[1], rdptr[2],
				phd[0], phd[1], phd[2], phd[3]);
			//"(This got return true!!)"    
			//db->mac_process = 1 - db->mac_process;
		//}    
	}
	//else
	//{
	//	if (db->mac_process) printk("macError-clear {rx-unicast %02x %02x %02x rxhdr %02x %02x %02x %02x}\n", 
	//		rdptr[0], rdptr[1], rdptr[2],
	//		phd[0], phd[1], phd[2], phd[3]);
	//	db->mac_process = 0;
	//}             
	
	if (flg_disp)
	{
		//packet ...    
		u16 calc;
		calc= dm9051_rx_cap(db);   
		printk("[dm9] %02x/%02x (RO %d.%d%c)\n", db->rwregs[0], db->rwregs[1], calc>>8, calc&0xFF, '%');                                    
		printk("[dm9] ior-mac %02X %02X %02X %02X %02X %02X\n",
			ior(db, 0x10), ior(db, 0x11), ior(db, 0x12), ior(db, 0x13), ior(db, 0x14), ior(db, 0x15));
		printk("[dm9] ior-RCR 0x%02X\n", ior(db, DM9051_RCR));
		printk("[dm9] RxLen %d\n", prxhdr->RxLen);
		printnb_packet(rdptr, prxhdr->RxLen-4);         
		printnb_packet(&rdptr[prxhdr->RxLen-4], 4);
		return false;
	}
	//if (db->flg_rxhold_evt)
	//	printk("[dm9].encounter NN-cast, dm9051__chk_data, ...End...\n");
	
	db->bC.rx_unicst_counter++;
	db->nSCH_UniRX++;
	return true;
}
#if DEF_SPIRW
/* Common etc usage (Chk data, Display, ...) */
static bool dm9051_chk_data(board_info_t *db, struct dm9051_rxhdr *prxhdr, u8 *rdptr) //'int RxLen' (db->bC.dummy_pad is 'rxbyte')
{
	struct net_device *dev = db->ndev;
	u8 *phd= (u8 *) prxhdr;
	u8 flg_disp = 0;

	if ((phd[1] & 0x40) && (!(rdptr[0] &1)))
	{
		flg_disp = 1;
		printk("dmfifo_reset( 10 multiErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x {Just rxhdr said multi-cast ..}\n", 
				rdptr[0], rdptr[1], rdptr[2],
				phd[0], phd[1], phd[2], phd[3]);
	}
	if (rdptr[0]!=dev->dev_addr[0] || rdptr[1]!=dev->dev_addr[1] || rdptr[2]!=dev->dev_addr[2])
	{                                     
		if (rdptr[0]&1) //'skb->data[0]'
		{
		 	#if 0
			db->bC.rx_multi_counter++;
		 	#endif

		 	if (db->mac_process) printk("macError-clear {rx multicast %02x %02x %02x rxhdr %02x %02x %02x %02x}\n", 
				rdptr[0], rdptr[1], rdptr[2],
				phd[0], phd[1], phd[2], phd[3]); //, Get recover-clear
		 	db->mac_process = 0;
			return true;
		}
		#if 1                    
		flg_disp = 1;
		if (db->mac_process) {
			//"[ERRO.found.s]"
			char hstr[72];
			sprintf(hstr, "dmfifo_reset( 11 macErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x", 
				rdptr[0], rdptr[1], rdptr[2],
				phd[0], phd[1], phd[2], phd[3]);
			db->mac_process = 0;
			db->bC.ERRO_counter++;
			dm9051_fifo_reset(11, hstr, db);
			dm9051_fifo_reset_statistic(db);
			return false;
		} else {
			//"[ERRO.found.s treat as no-error]"
			printk("dmfifo_reset( 11 macErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x {Just treat as a normal-unicast}\n", 
				rdptr[0], rdptr[1], rdptr[2],
				phd[0], phd[1], phd[2], phd[3]);
			//"(This got return true!!)"    
			db->mac_process = 1 - db->mac_process;
		}
		#endif
	}
	else
	{
		if (db->mac_process) printk("macError-clear {rx-unicast %02x %02x %02x rxhdr %02x %02x %02x %02x}\n", 
			rdptr[0], rdptr[1], rdptr[2],
			phd[0], phd[1], phd[2], phd[3]);
		db->mac_process = 0;
	}
	
	if (flg_disp)
	{
		//packet ...    
		u16 calc;
		calc= dm9051_rx_cap(db);   
		printk("[dm9] %02x/%02x (RO %d.%d%c)\n", db->rwregs[0], db->rwregs[1], calc>>8, calc&0xFF, '%');                                    
		printk("[dm9] ior-mac %02X %02X %02X %02X %02X %02X\n",
			ior(db, 0x10), ior(db, 0x11), ior(db, 0x12), ior(db, 0x13), ior(db, 0x14), ior(db, 0x15));
		printk("[dm9] ior-RCR 0x%02X\n", ior(db, DM9051_RCR));
		printk("[dm9] RxLen %d\n", prxhdr->RxLen);
		printnb_packet(rdptr, prxhdr->RxLen-4);         
		printnb_packet(&rdptr[prxhdr->RxLen-4], 4);
	}
	if (db->flg_rxhold_evt)
		printk("[dm9].encounter NN-cast, dm9051__chk_data, ...End...\n");
	
	db->bC.rx_unicst_counter++;
	db->nSCH_UniRX++;
	return true;
}
#endif
		
static void dm9051_rd_rxhdr(board_info_t *db, u8 *buff, unsigned len)
{	
#if DEF_SPIRW	
	#if 1
	//test.ok.
	//std_read_rx_buf(db, buff, len, true); tested.
	dm9051_inblk_rxhead(db, buff, len);
	#else
	//u8 test_buf[12]; //len is 4
	//dm9051_inblk(db, test_buf, 1);
	//buff[0]= test_buf[1];
	//dm9051_inblk(db, test_buf, 1);
	//buff[1]= test_buf[1];
	//dm9051_inblk(db, test_buf, 1);
	//buff[2]= test_buf[1];
	//dm9051_inblk(db, test_buf, 1);
	//buff[3]= test_buf[1];
	#endif
#endif	
}

/*
 *  Received a packet and pass to upper layer
 */
 #define DM_TYPE_ARPH	0x08
 #define DM_TYPE_ARPL	0x06
 #define DM_TYPE_IPH	0x08
 #define DM_TYPE_IPL	0x00
 #define RXHOLD_CONTROL_LEN	8
static int dm9000_rx(struct net_device *dev)  //.....
{
	board_info_t *db = netdev_priv(dev);
	int nRx = 0;
#if 1	
	struct dm9051_rxhdr rxhdr;
#endif	
#if 0	
	struct spi_rxhdr spihdr; //new
#endif	
#ifdef DM_CONF_SPI_TEST_BLKIN_SPLIT
	int ncp , blkLen;  //u8  blkin[DM_CONF_SPI_TEST_BLKLEN];
#endif	
	struct sk_buff *skb;
	u8 rxbyte, *rdptr;
	int RxLen, RxHold;                              
	bool GoodPacket;
	u16 calc;
	u8 *phd;
	static u16 rxwp = 0x0000;
	
	static int dbg_rxbyte_StepX = 0;
	static int dbg_test = 0;
	
	static int asr_rsrv_test = 0;
	
	db->rx_count= 0;

	/* Get most updated data */
#if DEF_SPIRW	
	rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */
	rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */
#endif	
	db->bC.dummy_pad= rxbyte;
		
	do {
		db->bC.RxLen = 0; // store.s
		// if ( rxbyte & DM9000_PKT_ERR) 
		// if (!(rxbyte & DM9000_PKT_RDY))
		// if ( rxbyte != DM9000_PKT_RDY)
		if ( rxbyte != DM9051_PKT_RDY)
		{
			if ( rxbyte == 0x00 ) {
	
				calc= dm9051_rx_cap(db); // get db->rwregs[0] & db->rwregs[1]
				if ((db->rwregs[0]!=db->rwregs[1]) || (db->bC.isbyte & ISR_PRS)) // Enter conditions
				{
					db->bC.rxbyte_counter0++;
					
					if (/*db->bC.rxbyte_counter0>1 */ db->bC.rxbyte_counter0>=5 || (calc>>8)>NUM_RX_FIFO_FULL){ // '3','5','50'
						
						Disp_RunningEqu(db);
					  //.printk("RXB_00Hs_or_FIFO_pre_full \n");		     	   
					  //.driver_dtxt_disp(db);
					}

				}
				     
			} else {
				calc= dm9051_rx_cap(db);
				if ((db->rwregs[0]!=db->rwregs[1]) || (db->bC.isbyte & ISR_PRS))
				{
						 if ((dbg_rxbyte_StepX <= 2) && (rxwp != db->rwregs[0] )) {
								printk("(dm9.rxbyte [step %02d]) %02x, rxptrs %x/%x isr %02x\n", dbg_rxbyte_StepX , rxbyte, db->rwregs[0], db->rwregs[1], db->bC.isbyte);
								rxwp = db->rwregs[0];
								dbg_rxbyte_StepX++;
						 }
						 db->bC.rxbyte_counter++; // insteaded (FFH, or not_01H)
				}
			}
		    
		    /*[if (dbg_test < 3)] xxx_ERRO() */
		    dbg_test += dm9051_fifo_ERRO(dbg_test, rxbyte, db); // (00H/FFH/not_01H) // {Operate RST if continue 'n' 0x00 read.}
			return nRx;
		} /* Status check: this byte must be 0xff, 0 or 1 */
		
		/* rxbyte_counter/rxbyte_counter0 */
		bcrdy_rx_info_clear(&db->bC);
 
		/* A packet ready now  & Get status/length */
		GoodPacket = true;
		
		
//	struct net_device *dev = db->ndev;
//	u16 calc= dm9051_rx_cap(db);
//    printk("hdrWrRd.s.%02x/%02x(RO %d.%d%c)\n", db->rwregs[0], db->rwregs[1], calc>>8, calc&0xFF, '%');
//    printk("hdrWrRd.e.%02x/%02x(RO %d.%d%c) rxlen= ...\n", db->rwregs[0], db->rwregs[1], calc>>8, calc&0xFF, '%');
        //dm9051_disp_hdr_s(db);
        
		  dm9051_rd_rxhdr(db, (u8 *)&rxhdr, sizeof(rxhdr));
		
		  RxLen = le16_to_cpu(rxhdr.RxLen);
		  db->bC.RxLen = le16_to_cpu(rxhdr.RxLen); // store

        //dm9051_disp_hdr_e(db, RxLen);
        
		/*
		 * [LARGE THAN 1536 or less than 64]!"
		 */
		 if (RxLen > DM9051_PKT_MAX || RxLen < 0x40) {
			char hstr[60];
			db->bC.LARGErr_counter++;
			
			sprintf(hstr, "dmfifo_reset( Large Err ) LargLen=%d", RxLen);
			dm9051_fifo_reset(1, hstr, db);
			
			//u16 calc= dm9051_rx_cap(db);
	     	//printk("( LargEr %d / %d ) LargLen=%d (RO %d.%d%c)\n", 
	        //	db->bC.LARGErr_counter, (db->bC.FIFO_RST_counter+1), 
	     	//	RxLen, calc>>8, calc&0xFF, '%');	
	        //dm9051._fifo_reset(1, "dmfifo._reset( LargEr )", db);
	        
	        dm9051_fifo_reset_statistic(db);
		    return nRx;
		 }

		 /* Packet Status check, 'RSR_PLE' happen, but take it not error!! 20150609JJ */		 
		 /* rxhdr.RxStatus is identical to RSR register. */
		if (rxhdr.RxStatus & (RSR_FOE | RSR_CE | RSR_AE |
				      RSR_RWTO |
				      RSR_LCS | RSR_RF)) {
			char hstr[70];
			GoodPacket = false;
			
			if (rxhdr.RxStatus & RSR_FOE) 
				dev->stats.rx_fifo_errors++;
			if (rxhdr.RxStatus & RSR_CE) 
				dev->stats.rx_crc_errors++;
			if (rxhdr.RxStatus & RSR_RF) 
				dev->stats.rx_length_errors++;
			
			db->bC.StatErr_counter++;
			
	 		rdptr= (u8 *)&rxhdr;
	 		
	     	sprintf(hstr, "dmfifo_reset( StatusErr ) rxbyte&rxhdr %02x & %02x %02x %02x %02x", rxbyte, rdptr[0], rdptr[1], rdptr[2], rdptr[3]);
	     	dm9051_fifo_reset(1, hstr, db);
	     	//printk("<!GoodPacket-rxbyte&rxhdr %02x & %02x %02x %02x %02x>\n", rxbyte, rdptr[0], rdptr[1], rdptr[2], rdptr[3]);
	     	//printk("( StatEr %d / %d ) StatEr=0x%02x", db->bC.StatErr_counter, (db->bC.FIFO_RST_counter+1), rdptr[1]);
            //dm9051._fifo_reset(1, "[!GoodPacket - StatusErr]", db);
            
            dm9051_fifo_reset_statistic(db);
           
		    return nRx;
		}

		/* Move data from DM9051 */
		phd= (u8 *) &rxhdr;
#ifdef ASL_RSRV_RX_HDR
		if ((skb = dev_alloc_skb(RxLen  + RXHOLD_CONTROL_LEN+ 4 + ASL_RSRV_RX_HDR_LEN)) == NULL)  {
#else
		if ((skb = dev_alloc_skb(RxLen + RXHOLD_CONTROL_LEN + 4)) == NULL)  {
#endif
			printk("dm9051 [Warn] [!ALLOC %d rx.len space]\n", RxLen);
			//printk("\n");
			/* dump-data */
#if DEF_SPIRW            
			dm9051_inblk_dump(db, RxLen);
#endif            
			return nRx;
		}

		if (asr_rsrv_test==0) {
			//asr_rsrv_test = 1;
			printk("[dm9].peek ------------ skb_alloc(RxLen pad RSRV= %d %d %d)---------\n", RxLen, 4, ASL_RSRV_RX_HDR_LEN);
			//printk("[dm9].peek ------------ skb_len.s= %d -------------------\n", skb->len);
		}

	    /* 
	     *  We note that "#define NET_IP_ALIGN  2"
	     *
	     *	Move data from DM9051 
		 *  (Linux skb->len IS LESS than 4, because it = RxLen - 4)
		 */
		/* Increase the headroom of an empty &skb_buff by            *
		 * reducing the tail room. Only allowed for an empty buffer. */ 
#ifdef ASL_RSRV_RX_HDR
		skb_reserve(skb, 2 + RXHOLD_CONTROL_LEN); // better.[+ ASL_RSRV_RX_HDR_LEN]
#else		 
		skb_reserve(skb, 2 + RXHOLD_CONTROL_LEN);
#endif		

		/* A pointer to the first byte is returned */
		if (asr_rsrv_test==0) {
			printk("[dm9].peek ------------ skb_rsrv(2) rsrv RxLen pad RSRV= %d %d %d %d---------\n", 2, RxLen, 2, ASL_RSRV_RX_HDR_LEN);
			printk("[dm9].peek ------------ skb_put(RxLen-4) rsrv SkbLen crc pad RSRV = %d %d %d %d %d---------\n", 2, RxLen - 4, 4, 2, ASL_RSRV_RX_HDR_LEN);
			printk("[dm9].peek ------------ skb_len(BEF.put)= %d -------------------\n", skb->len);
		}
		rdptr = (u8 *) skb_put(skb, RxLen - RXHOLD_CONTROL_LEN - 4);  
		if (asr_rsrv_test==0) {
			printk("[dm9].peek ------------ skb_len(AFT.put)= %d -------------------\n", skb->len);
		}

		/* Read received packet from RX SRAM */
		//dm9051_rd_rxdata(db, rdptr-1, RxLen);
		//=
#if DEF_SPIRW
		#ifdef DM_CONF_SPI_TEST_BLKIN_SPLIT
		  //--00---
		  ncp = 0;
		  blkLen = RxLen;
		  while (blkLen) {
			  if (blkLen > DM_CONF_SPI_TEST_BLKLEN) {
				dm9051_inblk_rxhead(db, db->blkin, DM_CONF_SPI_TEST_BLKLEN);
				memcpy(rdptr+ncp, db->blkin, DM_CONF_SPI_TEST_BLKLEN);
				ncp += DM_CONF_SPI_TEST_BLKLEN;
				blkLen -= DM_CONF_SPI_TEST_BLKLEN;
			  }
			  else {
				  dm9051_inblk_rxhead(db, db->blkin, blkLen);
				  memcpy(rdptr+ncp, db->blkin, blkLen);
				  blkLen= 0;
			  }
		  }
		#elif 1
		//Rel.
		/*dm9051_inblk_noncpy(db, rdptr-1, RxLen);*/
		//Work Around.1516
		db->flg_rxhold_evt = 0;
		RxHold= RxLen - 12;
		dm9051_inblk_noncpy(db, rdptr - 1, 12);
		if (rxhdr.RxStatus&0x40)
		{
			if (!(rdptr[0]&1))
			{
				db->flg_rxhold_evt = 1;
				rel_printk2("[dm9]rxhdr %02x %02x %02x %02x multi-cast, uni-mac %02x %02x %02x %02x [%02x %02x] .check macErr.\n",
					phd[0], phd[1], phd[2], phd[3],
					rdptr[0],rdptr[1],rdptr[2],rdptr[3],
					rdptr[4],rdptr[5]);
			}
		}
		else
		{
			if (rdptr[0]!=dev->dev_addr[0] || rdptr[1]!=dev->dev_addr[1] || rdptr[2]!=dev->dev_addr[2])
			{
				db->flg_rxhold_evt = 1;
				rel_printk2("[dm9]rxhdr %02x %02x %02x %02x uni-cast, uknow-mac %02x %02x %02x %02x [%02x %02x] .check macErr.\n",
					phd[0], phd[1], phd[2], phd[3],
					rdptr[0],rdptr[1],rdptr[2],rdptr[3],
					rdptr[4],rdptr[5]);        
			}
		}
		//if ((rxhdr.RxStatus&0x40) && !(rdptr[0]&1) )
		//{
		//}
		//if (!(rxhdr.RxStatus&0x40) && (rdptr[0]!=dev->dev_addr[0] || rdptr[1]!=dev->dev_addr[1] || rdptr[2]!=dev->dev_addr[2]) )
		//{
		//}
			if (rdptr[4]==DM_TYPE_ARPH && rdptr[5]==DM_TYPE_ARPL)
			{
				//printk("[dm9].ARP peek %02x %02x %02x %02x [%02x %02x] %02x %02x %02x %02x %02x %02x (%02x %02x)\n",
					//rdptr[0],rdptr[1],rdptr[2],rdptr[3],rdptr[4],rdptr[5],
					//rdptr[6],rdptr[7],rdptr[8],rdptr[9],rdptr[10],rdptr[11],
					//rdptr[12],rdptr[13]);
				//if (rdptr[12]!=DM_TYPE_ARPH || rdptr[13]!=DM_TYPE_ARPL) ;;[current no rdptr[12]/rdptr[13]]
				//{
					RxHold -= 8; // work-around
					if (!db->flg_rxhold_evt)
					{
						rel_printk2("[dm9]rxhdr %02x %02x %02x %02x ARP-cast, ARP-mac %02x %02x %02x %02x [%02x %02x] .check macErr.\n",
							phd[0], phd[1], phd[2], phd[3],
							rdptr[0],rdptr[1],rdptr[2],rdptr[3],
							rdptr[4],rdptr[5]);
						db->flg_rxhold_evt = 1;
					}
					rel_printk("[dm9] warn- ARP-PKT macErr\n");
				//}
			}
			if (rdptr[4]==DM_TYPE_IPH && rdptr[5]==DM_TYPE_IPL)
			{
				//printk("[dm9].IP peek %02x %02x %02x %02x [%02x %02x] %02x %02x %02x %02x %02x %02x (%02x %02x)\n",
					//rdptr[0],rdptr[1],rdptr[2],rdptr[3],rdptr[4],rdptr[5],
					//rdptr[6],rdptr[7],rdptr[8],rdptr[9],rdptr[10],rdptr[11],
					//rdptr[12],rdptr[13]);
				//if (rdptr[12]!=DM_TYPE_IPH || rdptr[13]!=DM_TYPE_IPL) ;;[current no rdptr[12]/rdptr[13]]
				//{        
					RxHold -= 8; // work-around
					if (!db->flg_rxhold_evt)
					{
						rel_printk2("[dm9]rxhdr %02x %02x %02x %02x xxIP-cast, xIP-mac %02x %02x %02x %02x [%02x %02x] .check macErr.\n",
							phd[0], phd[1], phd[2], phd[3],
							rdptr[0],rdptr[1],rdptr[2],rdptr[3],
							rdptr[4],rdptr[5]);
						db->flg_rxhold_evt = 1;
					}
					rel_printk("[dm9] warn- IP-PKT macErr\n");
				//}
			}
	#if 1 //JF, [only data disappear in fisrt 8 bytes, rxwptr is still OK.]
		dm9051_inblk_noncpy(db, rdptr - 1 + 12, RxLen - 12);
	#else   //JJ, [both data disappear in fisrt 8 bytes, and rxwptr is sub 8 bytes too.]
		dm9051_inblk_noncpy(db, rdptr - 1 + 12, RxHold); // The data byte offet in 'rdptr - 1 + 12' is destroied, which is the last byte of SRC Address.
	#endif
		
		//processing.RXHOLD_CONTROL_LEN (=8)
		if (RxHold == (RxLen - 12))
			skb_put(skb, RXHOLD_CONTROL_LEN);		//or (move skb->data only foreward 8)
		else
		{
			skb_put(skb, RXHOLD_CONTROL_LEN); 	//skb->data -= RXHOLD_CONTROL_LEN;
				//skb->len also need maintaining
				//either (move skb->data back 8 only v.s. skb_reserve(RXHOLD_CONTROL_LEN) more)
			rel_printk("[dm9] warn- macErr(move skb->data back 8)\n");
		}
		
		#else
		//dm9051_inblk(db, rdptr-1, RxLen);
		#endif

		//[.Add a quick-for-give!]
		if (db->flg_rxhold_evt)
		{
			//"[ERRO.found.s]"
			char hstr[72];
			sprintf(hstr, "dmfifo_reset( 11 macErr ) mac %02x %02x %02x %02x rxhdr %02x %02x %02x %02x (quick)", 
				rdptr[0], rdptr[1], rdptr[2], rdptr[3],
				phd[0], phd[1], phd[2], phd[3]);
			db->bC.ERRO_counter++;
			dm9051_fifo_reset(11, hstr, db);
			dm9051_fifo_reset_statistic(db);
		}
		else
		{
			if (!dm9051_chk_data(db, &rxhdr, rdptr)) //'RxLen' by &rxhdr (db->bC.dummy_pad is 'rxbyte')
				return nRx;
		}
#endif

		db->nSCH_GoodRX++;
		dev->stats.rx_bytes += RxLen;
    
		/* Pass to upper layer */
		skb->protocol = eth_type_trans(skb, dev);  // [JJ found: skb->len -= 14]
		if (dev->features & NETIF_F_RXCSUM) {
			if ((((rxbyte & 0x1c) << 3) & rxbyte) == 0)
				skb->ip_summed = CHECKSUM_UNNECESSARY;
			else
				skb_checksum_none_assert(skb);
		}

	#ifdef DM_CONF_POLLALL_INTFLAG 
		if (db->nSCH_INT < DM9_DBG_INT_ONOFF_COUNT) {
			printk("[dm9rx %3d].[%02x %02x]\n", db->nSCH_GoodRX,rdptr[12], rdptr[13]);
			printk("[dm9rx %3d].skb->protocol.[%04x]\n", db->nSCH_GoodRX,skb->protocol);
		}
	#endif
		
		if (asr_rsrv_test==0) {
			asr_rsrv_test = 1;
			printk("[dm9].peek ------------ eth_type_trans(skb, dev), skb->len -= 14 ---\n");
			printk("[dm9].peek ------------ skb_len(AFT.eth_typ_trans)= %d-------------------\n", skb->len);
			printk("[dm9].peek ------------ skb_alloc %d.is.sum.of= [%d %d %d  %d %d %d] ---\n", 
				RxLen + 4 + ASL_RSRV_RX_HDR_LEN,
				2, 14, skb->len, 4, 2, ASL_RSRV_RX_HDR_LEN);
		}
		
		//netif_rx(skb);
		if (in_interrupt())
			netif_rx(skb);
		else
			netif_rx_ni(skb);

		dev->stats.rx_packets++;
		db->rx_count++;
		nRx++;

		#if 1
		//Add TX cap. [no here]
		//dm9051_tx_irx(db);
		if (check_cntStop(db)) 
			break;
		#endif

#if DEF_SPIRW
		// Get DM9051 interrupt status 
		db->bC.isbyte= ior(db, DM9051_ISR);	// Got ISR
	  #if !defined WR_ISR_ENDOF_RXWORK_ONLY	// need written at end of dm9051 _rx _work
		iiow(db, DM9051._ISR, db->bC.isbyte); //..tgbd,. // Clear ISR status	
	  #endif
		/* Get most updated data */
		rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */
		rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */ //rxbyte = 0x01; //. readb(db->io_data);
#endif
	} while (rxbyte == DM9051_PKT_RDY); // CONSTRAIN-TO: (rxbyte != XX)
	return nRx;
}
