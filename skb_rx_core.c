//#if DM_CONF_APPSRC
//#endif 

 #define DM_TYPE_ARPH	0x08
 #define DM_TYPE_ARPL	0x06
 #define DM_TYPE_IPH	0x08
 #define DM_TYPE_IPL	0x00
static bool chk_data(board_info_t *db, struct dm9051_rxhdr *prxhdr, u8 *rdptr)
{
	struct net_device *dev = db->ndev;
	u8 *phd= (u8 *) prxhdr;
	u8 flg_disp = 0;

	if ((phd[1] & 0x40) && (!(rdptr[0] &1)))
	{
		flg_disp = 1;
		//printk("\n[@dm9.multiErr start-with rxb= 0x%0x]\n", prxhdr->RxPktReady);
		dev->stats.rx_length_errors= 3;
		dev->stats.rx_crc_errors=              6;
		dev->stats.rx_fifo_errors=                 9;
		dev->stats.rx_missed_errors=                 12;
		dev->stats.rx_over_errors++;
		printk("\n[@dm9.multiErr (rxhdr %02x %02x %02x %02x)] mac %02x %02x %02x, %lu\n", phd[0], phd[1], phd[2], phd[3], rdptr[0], rdptr[1], rdptr[2],
			dev->stats.rx_over_errors); //dev->stats
		#if 0
		printk("dm9.dmfifo_reset( 10 multiErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x {multi before rst RST_c= %d}\n", 
				rdptr[0], rdptr[1], rdptr[2],
				phd[0], phd[1], phd[2], phd[3],
				db->bC.DO_FIFO_RST_counter);
		#endif
	}
	else if (rdptr[0]!=dev->dev_addr[0] || rdptr[1]!=dev->dev_addr[1] || rdptr[2]!=dev->dev_addr[2])
	{                                  
		if ((rdptr[4]==DM_TYPE_ARPH && rdptr[5]==DM_TYPE_ARPL) && (rdptr[12]!=DM_TYPE_ARPH || rdptr[13]!=DM_TYPE_ARPL)) // special in data-skip
			; // error=fail //;;[current has rdptr[12]/rdptr[13]]
		if ((rdptr[4]==DM_TYPE_IPH && rdptr[5]==DM_TYPE_IPL) && (rdptr[12]!=DM_TYPE_IPH || rdptr[13]!=DM_TYPE_IPL))  // special in data-skip
			; // error=fail //;;[current has rdptr[12]/rdptr[13]]
		else if (rdptr[0]&1) //'skb->data[0]'
			return true;
			
		#if 1
		//[01 00 9e 00] unicast and len is less 256 ;"Custom report 20210204/'lan_aging_error3.log'"
		if (phd[0]==0x01 && phd[1]==0x00 && phd[3]==0x00){
			printk("\n[@dm9.[warn] unknow uni-cast frame (hdr %02x %02x %02x %02x)] %02x %02x %02x %02x %02x %02x\n", 
				phd[0], phd[1], phd[2], phd[3], rdptr[0], rdptr[1], rdptr[2], rdptr[3], rdptr[4], rdptr[5]);
			return true;
		}
			
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
			//printk("\n[@dm9.macErr start-with rxb= 0x%0x]\n", prxhdr->RxPktReady);
			dev->stats.rx_frame_errors++;
			printk("\n[@dm9.frame error (hdr %02x %02x %02x %02x)] [%02x %02x %02x %02x %02x %02x] %lu\n", 
				phd[0], phd[1], phd[2], phd[3], rdptr[0], rdptr[1], rdptr[2], rdptr[3], rdptr[4], rdptr[5],
				dev->stats.rx_frame_errors);
			#if 0
			printk("dm9.dmfifo_reset( 11 macErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x {before rst RST_c= %d}\n", 
				rdptr[0], rdptr[1], rdptr[2],
				phd[0], phd[1], phd[2], phd[3],
				db->bC.DO_FIFO_RST_counter); //(quick)
			#endif
			//"(This got return true!!)"    
			//db->mac_process = 1 - db->mac_process;
		//}
		#endif    
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
		#if MSG_DBG
		u16 calc;
		calc= dm9051_rx_cap(db);   
		printk("[dm9] %02x/%02x (scan_enter)\n", db->rwregs_enter, db->rwregs1_enter); //[trick.extra]
		printk("[dm9] %02x/(wrp scan.end)\n", db->rwregs_scanend ); //[trick.extra]
		printk("[dm9] %02x/%02x (scan.end RO %d.%d%c)\n", db->rwregs[0], db->rwregs[1], calc>>8, calc&0xFF, '%');
		#endif
		//  printk("[dm9] ior-mac %02X %02X %02X %02X %02X %02X\n",
		//	    ior(db, 0x10), ior(db, 0x11), 
		//	    ior(db, 0x12), ior(db, 0x13), 
		//	    ior(db, 0x14), ior(db, 0x15));
		//  printk("[dm9] ior-RCR 0x%02X\n", ior(db, DM9051_RCR));
		#if 0
		printnb_packet(rdptr, prxhdr->RxLen-4);         
		printnb_packet(&rdptr[prxhdr->RxLen-4], 4);
		#endif
		return false;
	}
	//if (db->flg_rxhold_evt)
	//	printk("[dm9].encounter NN-cast, dm9051__chk_data, ...End...\n");
	
	db->bC.rx_unicst_counter++;
	db->nSCH_UniRX++;
	return true;
}

struct sk_buff *SKB_trans(struct net_device *dev, char * buffp, int RxLen, int SKB_size)
{
	board_info_t *db = netdev_priv(dev);     
	struct sk_buff *skb;
	u8 *rdptr;
	u8 rxbyte = db->bC.dummy_pad;
	static int asr_rsrv_test = 0;
#ifdef ASL_RSRV_RX_HDR
	if ((skb = dev_alloc_skb(SKB_size)) == NULL)  {
			printk("dm9051 [!ALLOC skb size %d fail]\n", SKB_size);
			return NULL;
	}
#else
	if ((skb = dev_alloc_skb(SKB_size - ASL_RSRV_RX_HDR_LEN)) == NULL)  {
			printk("dm9051 [!ALLOC skb size %d fail]\n", SKB_size - ASL_RSRV_RX_HDR_LEN);
			return NULL;
	}
#endif
	if (asr_rsrv_test==0) {
		//asr_rsrv_test = 1;
		printk("[dm9].peek ------------ RxLen 4 RSRV.s= %d %d %d---------\n", RxLen,
			 4, ASL_RSRV_RX_HDR_LEN);
		printk("[dm9].peek ------------ skb_len.s= %d -------------------\n", skb->len);
	}   
	skb_reserve(skb, 2);
	
	if (asr_rsrv_test==0) {
		printk("[dm9].peek ------------ skb_put.i(x), x= %d -------------------\n", RxLen - 4);
		printk("[dm9].peek ------------ skb_len.s(put)= %d -------------------\n", skb->len);
	}
	
	/* A pointer to the first byte of the packet */
	rdptr = (u8 *) skb_put(skb,  RxLen - 4);  // [save 4] SKB_size - 4 - ASL_RSRV_RX_HDR_LEN - 4
	memcpy(rdptr, buffp, RxLen - 4); // [save 4] &sbufp[p]
	
	if (asr_rsrv_test==0) {
		printk("[dm9].peek ------------ skb_len.e(put)= %d -------------------\n", skb->len);
	}                   
	// ?? if (!dm9051_chk_data(db, rdptr, RxLen))
	//	return nRx; ? rwregs1; ?          
	dev->stats.rx_bytes += RxLen;
	skb->protocol = eth_type_trans(skb, dev);  // [JJ found: skb->len -= 14]
		 
	if (asr_rsrv_test==0) {
		asr_rsrv_test = 1;
		printk("[dm9].peek --- due to eth_type_trans(skb, dev), skb->len -= 14 ---\n");
		printk("[dm9].peek ------------ skb_len.e= %d -------------------\n", skb->len);
		printk("[dm9].peek ------------ skb_alloc.is= %d -------------------\n", RxLen + 4 + ASL_RSRV_RX_HDR_LEN);
	}
	
	if (dev->features & NETIF_F_RXCSUM) {
		if ((((rxbyte & 0x1c) << 3) & rxbyte) == 0)
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb_checksum_none_assert(skb);
	}
	
	rel_printk6("[DM9].netif_rx: skb->len %d\n", skb->len);
	printnb_packet6(skb->data, 32); 
	
	if (in_interrupt())
		netif_rx(skb);
	else
		netif_rx_ni(skb);

	dev->stats.rx_packets++;  
	return skb;
}

u8 sb_scan_enter = 0;
u8 sb_scan_enter1 = 0;
u16 SB_skbing(struct net_device *dev, char *sbufp, u16  rwregs1)
{
	board_info_t *db = netdev_priv(dev);     
	struct dm9051_rxhdr rxhdr;   
	//[u8 rxbyte;]
	int prevl = 0, RxLen;
	int prevp = 0, p = 1;
	static int diff = 0;

	db->validlen_for_prebuf = 0; //init.
	while(sbufp[p] == DM9051_PKT_RDY)
	{                                                                     
		db->bC.dummy_pad = sbufp[p];
		memcpy((char *)&rxhdr, &sbufp[p], sizeof(rxhdr));
		RxLen = rxhdr.RxLen;
		
		//[read.mem.buf's]
		if ((p+sizeof(rxhdr)) <= (SCAN_LEN_HALF+1)) //RWREG1_END
		{
			p += sizeof(rxhdr);
			db->validlen_for_prebuf = p;
		} 
		else {
			rel_printk6("[trace.end1 reg1 %x]\n", rwregs1);
			goto skbing_end; //return rwregs1;
		}

		//[read.mem.buf's]
		if ((p+RxLen ) <= (SCAN_LEN_HALF+1)) //RWREG1_END
			;  //[go skb-trans data and then move pointer 'p']
		else {
			rel_printk6("[trace.end2 reg1 %x]\n", rwregs1);
			goto skbing_end; //return rwregs1;
		}

#if 0
		//_check_data
#else
		if (!chk_data(db, (struct dm9051_rxhdr *) &sbufp[p-sizeof(rxhdr)], &sbufp[p])) //&sbufp[p] = skb->data
		{
			//do {       
			//[simple NCR reset]   
				//"[ERRO.found.s]"
				#if MSG_DBG
				  struct dm9051_rxhdr *prxhdr = (struct dm9051_rxhdr *) &sbufp[p-sizeof(rxhdr)];
				#endif
				
				#ifdef ON_RELEASE
				#if MSG_DBG
				  dm9051_rx_cap(db);
				  printk("[dm9] %02x/%02x (again scan.end RO %d.%d%c)\n", db->rwregs[0], db->rwregs[1], db->calc>>8, db->calc&0xFF, '%'); 
				  printk("[dm9] skbing.on %02x/%02x while.acc.nRx %d macErr.RxLen %d\n", 
					db->rwregs[0], rwregs1,
					db->nRx, prxhdr->RxLen);
				#endif
				#endif
				
				do {         
					  char hstr[72];
					  sprintf(hstr, "dmfifo_reset( 11 macErr ) mac %02x %02x %02x %02x rxhdr %02x %02x %02x %02x (quick)", 
						sbufp[p], sbufp[p+1], sbufp[p+2], sbufp[p+3],
						sbufp[p-4], sbufp[p-3], sbufp[p-2], sbufp[p-1]);
					//.printk("dm9.s-%s Len %03d RST_c %d\n", hstr, db->bC.RxLen, db->bC.DO_FIFO_RST_counter);
					db->bC.ERRO_counter++;
					dm9051_fifo_reset(11, hstr, db);
					dm9051_fifo_reset_statistic(db);
				} while (0);
				
				#ifdef ON_RELEASE
				  dm9051_rx_cap(db);
				#if MSG_DBG
				  printk("dm9.e-%s Len %03d RST_c %d\n", hstr, db->bC.RxLen, db->bC.DO_FIFO_RST_counter);
				  printk("[dm9] %02x/%02x (end RO %d.%d%c)\n", db->rwregs[0], db->rwregs[1], 0, 0, '%'); 
				#endif
				#if MSG_REL
				  printk("[dm9] %02x/%02x (r%lu o%lu f%lu RST_c %d)\n", db->rwregs[0], db->rwregs[1], 
					dev->stats.rx_errors, dev->stats.rx_over_errors, dev->stats.rx_frame_errors, db->bC.DO_FIFO_RST_counter);  //"end RO %d.%d%c", 0, 0, '%', 
				#endif
				#endif
				return 0x0c00;
			//} while(0);
		}
#endif
		prevp = p;
		prevl= RxLen;
		if (!SKB_trans(dev, &sbufp[p], RxLen, RxLen + 4 + ASL_RSRV_RX_HDR_LEN)) {  //[or compact as (RxLen - 4 + ASL_RSRV_RX_HDR_LEN)]
				printk("[trace.end3 reg1 %x]\n", rwregs1); // not 'rel_printk6', wait happen to do analysis
				goto skbing_end; //return rwregs1;
		}
		p += RxLen ;
		db->validlen_for_prebuf = p;
		db->nRx++;    
		//[db->rx_count++;]
		//[copying skb..]

		//{chip's}
		rwregs1 += sizeof(rxhdr) + RxLen;
		rel_printk6("[trace.on reg1 %x]\n", rwregs1);
		if (rwregs1 > RWREG1_END)
		{
			rwregs1 -= RWREG1_OVSTART;
			rwregs1 += RWREG1_START;
		}

		//[read.mem.buf's]
		if (p > (SCAN_LEN_HALF+1)) //'RWREG1_END'
		{
			printk("[trace.end4 reg1 %x]\n", rwregs1); // not 'rel_printk6', wait happen to do analysis
			goto skbing_end; //return rwregs1;
		}
	}
	rel_printk_last("[trace.end5 reg1 %x]\n", rwregs1); // not 'rel_printk6', wait happen to do analysis
skbing_end:
	if (db->nRx != diff)
	{
		if (sb_scan_enter && db->nRx > 32)
		{
			sb_scan_enter++;
			if (sb_scan_enter > 1) //2
				sb_scan_enter = 0;
			printk("        ---------------------------------- [%d PKs dm9LastRxb] %02x\n", db->nRx, db->bC.last_ornext_pad);
		}
		diff = db->nRx;
	}
	return rwregs1;
}         

char *SB_scan0(board_info_t *db, char *sbuff)
{
	u8 rxbyte;        
	int RxLen;
	struct dm9051_rxhdr *prxhdr;
	char bf1;
	//char *inblkSTARTP = NULL;
	u16 calc;
	int p = 1;

	db->validlen_for_prebuf = 0; //init.
	/*
	 * If this spi read xfer, over-read than rx_sram_writte_in is not allowed~
	 * MOdify to be spi-read-xfer by sizeof(rxhdr) and then rxhdr.RxLen,
	 * Repeat untill no rxbyte(0x01) or the todo spi-read-xfer will 
	 * over-than totally SCAN_LEN_HALF bytes.
	 */

	db->bC.isbyte = ior(db, DM9051_ISR);
	calc= dm9051_rx_cap(db);
db->bC.isr_clear = 0;	// 20210201.RX_Pointer_Protect

	rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */
	rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */
	if (rxbyte != DM9051_PKT_RDY)
	{                                                                                                                                                                                                                                                                                                                                                           
		db->bC.last_ornext_pad = rxbyte; // We also think below line, that sbuff[p] = rxbyte, is also OK.
		sbuff[p] = 0xcc; // We like to make completely, NOT 0x01, so everything after this could not accident find the dummy-0x01 head-rxb
		db->rwregs_scanend = db->rwregs[0]; //[trick.extra]
		if (!db->bC.isr_clear)
			iow(db, DM9051_ISR, 0xff);
		return NULL; //inblkSTARTP;
	//inblkSTARTP = sbuff;
	}

	rel_printk5("(dm9.rxbyte [step %02d]) %02x, rxptrs %x/%x isr %02x\n", 101 , rxbyte, db->rwregs[0], db->rwregs[1], db->bC.isbyte);

	do 
	{
		rel_printk5("(dm9.rxbyte [step %02d]) p= %d {%d, %d}\n", 111 , p, p+sizeof(struct dm9051_rxhdr), RWREG1_OVSTART);

		if ((p+sizeof(struct dm9051_rxhdr)) > (SCAN_LEN_HALF+1)) //[not 'RWREG1_OVSTART']
		{                   
			if (sb_scan_enter1)
			{
				sb_scan_enter1 = 0;
				printk("        ---------------------------------- [scan.rxhdr exaust-sbuf %d + %u] > %d\n", p, (unsigned int) sizeof(struct dm9051_rxhdr), (SCAN_LEN_HALF+1));
			}
			break;
		}

		dm9051_inblk_noncpy(db, &sbuff[p-1], sizeof(struct dm9051_rxhdr)); /*rdptr-1*/    /* destroy previous pkt's crc's 4th byte.*/
		sbuff[p-1] = 0xcc; // Be Destroied, mark to be 0xcc */
	if (!db->bC.isr_clear){
		iow(db, DM9051_ISR, 0xff);
		db->bC.isr_clear = 1;
	}

		rel_printk5("(dm9.rxbyte [step %02d]) %02x, %02x %02x %02x %02x\n", 112 , sbuff[p-1], 
			sbuff[p], sbuff[p+1], sbuff[p+2], sbuff[p+3]);
		
		prxhdr = (struct dm9051_rxhdr*) &sbuff[p];
		RxLen = prxhdr->RxLen;
		p += sizeof(struct dm9051_rxhdr);  
		db->validlen_for_prebuf = p; //[scan0]

		if ((p+RxLen) > (SCAN_LEN_HALF+1)) //[not 'RWREG1_OVSTART']
		{
			if (sb_scan_enter1)
			{
				sb_scan_enter1 = 0;
				printk("        ---------------------------------- [scan.payld exaust-sbuf %d + %d] > %d\n", p, RxLen, (SCAN_LEN_HALF+1));
			}
			break;
		}

		db->bC.isr_clear = 0;
		#ifdef DM_CONF_ANY_BUF_CASE_SKB_RX_CORE
		  do {
			  int n = RxLen;
			  while (n > ANY_BUF_NUM){
				  bf1 = sbuff[p-1];
				  dm9051_inblk_noncpy(db, &sbuff[p-1], ANY_BUF_NUM); /*rdptr-1*/
				  sbuff[p-1] = bf1;
				  p += ANY_BUF_NUM;
				  n -= ANY_BUF_NUM;
			  }
			  if (n){
				  bf1 = sbuff[p-1];
				  dm9051_inblk_noncpy(db, &sbuff[p-1], n); /*rdptr-1*/
				  sbuff[p-1] = bf1;
				  p += n;
			  }
		  } while (0);
		#else
		  bf1 = sbuff[p-1];
		  dm9051_inblk_noncpy(db, &sbuff[p-1], RxLen); /*rdptr-1*/
		  sbuff[p-1] = bf1;
		  p += RxLen;
		#endif
	//.if (!db->bC.isr_clear){
	iow(db, DM9051_ISR, 0xff);
	db->bC.isr_clear = 1;
	//.}

db->bC.isbyte = ior(db, DM9051_ISR); //calc= dm9051_rx_cap(db);
db->bC.isr_clear = 0;	

		db->validlen_for_prebuf = p; //[scan0]

		rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */
		rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */
		if (rxbyte != DM9051_PKT_RDY)
		{
			db->bC.last_ornext_pad = rxbyte; // We also think below line, that sbuff[p] = rxbyte, is also OK.
			sbuff[p] = 0xcc; // We like to make completely, NOT 0x01, so everything after this could not accident find the dummy-0x01 head-rxb
			break;
		}
	} while (1);
	sbuff[p] = 0; // in case, to not 0x01.
if (!db->bC.isr_clear)
	iow(db, DM9051_ISR, 0xff);

	/*
	//.db->bC.isbyte = ior(db, DM9051_ISR);
	*/

	calc= dm9051_rx_cap(db);
	db->rwregs_scanend = db->rwregs[0]; //[trick.extra]
	
	/*
	//.rel_printk5("(dm9.rxbyte [step %02d]) %02x, rxptrs %x/%x isr %02x\n", 102 , rxbyte, db->rwregs[0], db->rwregs[1], db->bC.isbyte);
	*/

	#if 0
	/*
	 * This spi read xfer, EVER to large length, encounter fail result! 
	 */
	dm9051_inblk_noncpy(db, sbuff, SCAN_LEN_HALF); /*rdptr-1*/
	#endif

	return sbuff; //'inblkSTARTP'
}           

void SB_scan0_enter(board_info_t *db, u16 rwregs1)
{
	static u32 disp_tot_quan = 0;
	u32 m, sm;
	int mf, smf;
	if (db->rwtrace1 < rwregs1)
	{
		if (db->rwtrace1)
		{                                                             
			db->rx_rst_quan += 13;
			db->rx_tot_quan += 13;
		}
		
		if (db->rx_tot_quan > disp_tot_quan)
		{
			sb_scan_enter = 1;
			sb_scan_enter1 = 1;
			disp_tot_quan += 35840; // 35 m
				//102400; // 100 M
				//35840; // 35 m
				//512; // 0.5 M
			m = db->rx_tot_quan / 1024;
			sm = db->rx_rst_quan / 1024;
			mf = smf= 0;
			if ((db->rx_tot_quan - (m * 1024)) >= 512)  mf= 5;
			if ((db->rx_rst_quan - (sm * 1024)) >= 512)  smf= 5;
			
			#ifdef DM_CONF_ANY_BUF_CASE_SKB_RX_CORE
			printk("\n");
			printk("               <dm9 note: Testing %d bytes ANY_BUF_CASE_SKB_RX_CORE> \n", ANY_BUF_NUM);
			#endif
			
			printk("        ---------------------------------- <dm9 %04x [%d.%d M]/ %d.%d M> \n", 
				rwregs1, sm, smf, m, mf);		// "(rst %d)", db->bC.DO_FIFO_RST_counter
		}
	}
	db->rwtrace1 = rwregs1;
}

static int dm9000_rx(struct net_device *dev)  //.....
{
	board_info_t *db = netdev_priv(dev);
	char *sbufp;   

//static int dbg_nstrip = 6; 	
	//if (dbg_nstrip==0)  //.
	//	return 0; //.
	//dbg_nstrip--; //.
	
	db->nRx = 0;
	SB_scan0_enter(db, db->rwregs1);
	sbufp = SB_scan0(db, db->sbuf); //= SB_scan(db, db->sbuf, db->scanmem_sta);
	if (sbufp)
	{
		db->scan_end_reg0= db->rwregs[0]; // Save wr on (Last.ScanEnd)
		rel_printk5("[Scan Len : 0x%x (%d)]\n", db->validlen_for_prebuf, db->validlen_for_prebuf);
		rel_printk_last(" wt.s rwregs1 0x%x\n", db->rwregs1); //"%02x %02x" db->rwregs1 & 0xff, (db->rwregs1 >> 8) & 0xff
		printnb_packet5(sbufp, 17); // 32, min(db->validlen_for_prebuf - 4, 32)
		printnb_packet0(&sbufp[db->validlen_for_prebuf-4], 4);
		db->rwregs1 = SB_skbing(dev, sbufp, db->rwregs1);
		db->last_nRx = db->nRx;
		db->nSCH_GoodRX += db->nRx;
		
		read_rwr(db, &db->last_rwregs[0]); // Save every wr (Last.SkbingEnd)
		db->last_rwregs[1] = db->rwregs1;     // Save every rd
		db->last_calc = dm9051_calc(db->last_rwregs[0], db->last_rwregs[1]);
		
		if (db->rwregs1==0x0c00)
		{
			return db->nRx;
		}
	#ifndef FREE_NO_DOUBLE_MEM_MAX	// [do not copy for better performance, also with saving memory!]
		memcpy(db->prebuf, sbufp, db->validlen_for_prebuf); // not 'SCAN_LEN_HALF+1'
	#endif
		rel_printk_last(" [skbing : 0x%x (%d)]\n", db->validlen_for_prebuf, db->validlen_for_prebuf);
		
		iow(db, DM9051_MRRL, db->rwregs1 & 0xff);
		iow(db, DM9051_MRRH, (db->rwregs1 >> 8) & 0xff);
		rel_printk_last(" wt.e rwregs1 0x%x\n\n", db->rwregs1); //"%02x %02x" db->rwregs1 & 0xff, (db->rwregs1 >> 8) & 0xff
	}
	return db->nRx;
}
