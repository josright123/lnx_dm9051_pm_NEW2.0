// delayed_wrok_rx.c <----- [appsrc_user.c]

//#if DM_CONF_APPSRC

// -------------- rx operation skel --------------------

//static void dm9051_disp_hdr_s(board_info_t *db)
static void dm9051_disp_hdr_s_new(board_info_t *db)
{
	u16 calc= dm9051_rx_cap(db);
	//printk("dm9.hdrRd.s.%02x/%02x(RO %d.%d%c)\n", db->rwregs[0], db->rwregs[1], calc>>8, calc&0xFF, '%'); //hdrWrRd
	db->DERFER_rwregs[RXM_WrtPTR]= db->rwregs[0];
	db->DERFER_rwregs[MD_ReadPTR]= db->rwregs[1];
	db->DERFER_calc= calc;
}
//static void dm9051_disp_hdr_e(board_info_t *db, int rxlen)
static void dm9051_disp_hdr_e_new(board_info_t *db)
{
	//u16 calc= dm9051_rx_cap(db);
	//printk("hdrWrRd.e.%02x/%02x(RO %d.%d%c) rxLen(0x%x)= %d\n", db->rwregs[0], db->rwregs[1], calc>>8, calc&0xFF, '%', 
	//  rxlen, rxlen);
	u16 calc= dm9051_rx_cap(db);
	db->DERFER_rwregs1[RXM_WrtPTR]= db->rwregs[0];
	db->DERFER_rwregs1[MD_ReadPTR]= db->rwregs[1];
	db->DERFER_calc1= calc;
} 

static int /*irqreturn_t */
dm9051_isr_ext2(void *dev_id) //dm9051_isr_ext1(int irq, void *dev_id, int flag)
{
	struct net_device *dev = dev_id;
	board_info_t *db = netdev_priv(dev);
	unsigned int_status = 0;
	int nRx= 0;
	u8 flg_rx_cap =  0;
	static int nRx_counter0= 0;
	//static u8 eqEnter_count = 0;
	//static u8 neqEnter_count = 0;
	static int nNT_section_count = 0;
	//static u16 rwreg_equ = 0;

	#if 1
	//Add TX cap. (to be tested.)
	//dm9051_tx_irx(db);
	//dm9051_tx_work_disp(db);
	#endif

	// Received the coming packet 
#if 1
	//-----------------------------------
	//needed especially, for polling mode
	//-----------------------------------
	#if DRV_POLL_1
	u16 calc;
	flg_rx_cap = 1;
	calc= dm9051_rx_cap(db);
	if (db->rwregs[0]==db->rwregs[1])
	{
	  // [disp] [todo] Will make it only re-cycle is happen
	}
	else
	{
		//[trick.extra]
		if (flg_rx_cap )
			db->rwregs_enter = db->rwregs[0];
		else
			read_rwr(db, &db->rwregs_enter);
		//[trick.extra1]
		db->rwregs1_enter = db->rwregs1;
	#endif
#endif
		//u16 regs[2];
		//regs[0]= db->rwregs[0];
		//regs[1]= db->rwregs[1];
		//if (int_status & ISR_PRS)
		//	dm9000_rx(dev);
		//=
		#if 0
		u8 rxbyte;
		rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */
		rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */	
		if ( rxbyte == DM9051_PKT_RDY)
		{
		#endif

		// Get DM9051 interrupt status 
#if DEF_SPIRW
		db->bC.isbyte= ior(db, DM9051_ISR);	// Got ISR
		#if !defined WR_ISR_ENDOF_RXWORK_ONLY	// need written at end of dm9051 _rx _work
			iiow(db, DM9051._ISR, db->bC.isbyte); //..tgbd,. // Clear ISR status
		#endif
#endif
			int_status = db->bC.isbyte;

			if (int_status & ISR_PRS)
			{
				nRx= dm9000_rx(dev);
				if (nRx==0) 
				{

				  nRx_counter0++;
				  if (!(nRx_counter0 % 25) ) {

				  	nNT_section_count++;
				  	if (nNT_section_count <= 5)
				  	{
						//printk("\n[@dm9.RxbErr start-with rxb= 0x%0x]\n", db->bC.last_ornext_pad);
						dev->stats.rx_errors++;
						printk("\n[@dm9.RX errors (rxb= 0x%0x)] %lu\n", db->bC.last_ornext_pad, dev->stats.rx_errors);
						#if MSG_DBG
						  printk("dm9.Tell-Last.ScanEnd %02x\n", db->scan_end_reg0);
						  printk("dm9.Tell-Last.SkbingEnd %02x/%02x (tod RO %d.%d%c), Last.nRx %d\n", 
							db->last_rwregs[0], db->last_rwregs[1], db->last_calc>>8, db->last_calc&0xFF, '%',
							db->last_nRx);
						  printk("Warn-(nRx 0), rxb_pad %02x, NT %d: When traffic, No dm9051-fifo-reset(11) nRx_counter0= %d\n", 
							db->bC.last_ornext_pad , nNT_section_count, nRx_counter0);
						#endif
					}
				  	if (nNT_section_count == 5)
						printk("Warn-(nRx 0), rxb_pad %02x ........... and more ............\n", db->bC.last_ornext_pad);

					#if 1
					do {       
					//[simple NCR reset]   
						//"[ERRO.found.s]"
						char hstr[82];
						db->bC.RXBErr_counter++; //db->bC.ERRO_counter++;
						
						if (!flg_rx_cap)
							dm9051_rx_cap(db);
						
						#ifdef ON_RELEASE    
						#if MSG_DBG
						  printk("[dm9] %02x/%02x (tod RO %d.%d%c), now.dm9051_rx.return.nRx %d\n", // since (nRx==0)
							db->rwregs[0], db->rwregs[1], db->calc>>8, db->calc&0xFF, '%', nRx); 
						#endif
						#endif
						
						sprintf(hstr, "dmfifo_reset( 02 RxbErr ) ISR %02x %04x/%04x (at outside-caller nRx 0)", 
							db->bC.isbyte, 
							db->rwregs[0], 
							db->rwregs[1]);
						dm9051_fifo_reset(2, hstr, db);
						dm9051_fifo_reset_statistic(db);
						
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
					} while(0);
					#endif
			
					  //"[ERRO.found.s]"
					 //..dm9051._fifo_reset(11, "dmfifo._reset( 11 )", db); //~= dm9051._fifo_reset(11, ...)
					  //.dm9051._fifo_reset_statistic(db);
					  //.int._status= ISR_CLR_STATUS;
				  }

				  goto end_of_dm9051_isr_ext;
				}
			}
			else
			{
				nRx= dm9000_rx(dev);
				#if 0
				//<No need>
				if (nRx==0) {
				  //"[ERRO.found.s]"
				  if (int_status & ISR_PRS)
				   printk("[NOT DO RST.. (!ISR_PRS)] ISR 0x%02x and-Zero-pkt ( nRx %d, totalRx %lu)\n", 
				     int_status, nRx, dev->stats.rx_packets);
				  //.int_status= ISR_CLR_STATUS; //return ISR_CLR_STATUS;
				  //.goto end_of_dm9051_isr_ext;
				}
				#endif
				#if 0
				//"[Special.found.s]", NT= NoteTemp
				//<Many case that get 1 frame below disp.>
				if (nRx) {
				   printk("[NT: SPECIAL.FND.. ISR 0x%02x (!ISR_PRS)] nRx= %d ( totalRx %lu)\n", 
				     int_status, nRx, dev->stats.rx_packets);
				}
				#endif
			}

			if (nRx) {
				nRx_counter0= 0;
				nNT_section_count = 0;
			}

		#if 0
		}
		#endif
		//.else
		//.{
		//.	if (int_status & ISR_PRS)
		//.	{
		//.		dm9000_rx(dev);
		//.		printk(ISRByte 0x%02x warnning rxbyt=0x%02x (add %d to %lu)\n", 
		//.		  int_status, rxbyte, nRx, dev->stats.rx_packets);
		//.	}
		//.}
#if 1
	#if DRV_POLL_1
	}
	#endif
#endif

	/* Receive Overflow */
	if (int_status & ISR_ROS) {

		if (nRx) 
				; //printk(" [ERR-ISR] ISR_ROS(rx-overflow), nRX= %d, NOT DO RST..\n", nRx);
		else {		
				char hstr[60];
				db->bC.OvrFlw_counter++;
		#if 0
				printk("( Rxb %d ) %d %d ++ \n", db->bC.isbyte, db->bC.OvrFlw_counter, db->bC.rxbyte._counter0_to_prt);
				
				if (!(db->bC.OvrFlw_counter%5))
				 {
				 //driver_dtxt_disp(db);
				 //driver_dloop_disp(db);
				 }
		#endif
		#if 0
				printk(" db_isbyte 0x%02x (%d ++)\n", db->bC.isbyte, db->bC.rxbyte._counter0_to_prt);
				printk(" int_status 0x%02x", int_status);
		#endif
				sprintf(hstr, "dmfifo_reset( rcv ovflw ) ERR-ISR= %02x", int_status);
				dm9051_fifo_reset(1, hstr, db);
				//printk(" [ERR-ISR= %02x] (overflow)", int_status); //dm9051_fifo_show_flatrx(" [ERR-ISR] (recieve overflow)"); // (--, db)
				//dm9051._fifo_reset(1, "dmfifo._reset(recieve overflow )", db); //early fifo_reset
				dm9051_fifo_reset_statistic(db);
		}

		//int_status= ISR_CLR_STATUS;
		goto end_of_dm9051_isr_ext;
	}
end_of_dm9051_isr_ext:

	//return int_status; //[Here! 'void' is OK]
	return nRx;
}
//#endif
