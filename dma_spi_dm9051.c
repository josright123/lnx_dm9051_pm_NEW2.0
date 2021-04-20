
//[Classic declare definitions]
//#ifdef RPI_CONF_SPI_DMA_YES
//static int enable._dma = 1; // Default: 1 (ON)
//#else
//static int enable._dma = 0; // Default: 0 (OFF)
//#endif
//=
#ifdef RPI_CONF_SPI_DMA_YES /* [defined in "conf_ver.h"] */
static int enable_dma;
#endif

//[Implemant code]
#if DEF_PRO
#ifdef RPI_CONF_SPI_DMA_YES
//[spi_dm9051_dma.c]
static int dm9051_is_dma_param(struct board_info *db)
{
        //if (!enable._dma) return enable._dma; //int ret = 0;
        int dma_bff= 0;

#if DEF_SPIRW        
        db->spidev->dev.coherent_dma_mask = ~0;

        db->spi_tx_buf = dma_alloc_coherent(&db->spidev->dev,
                                            PAGE_SIZE,
                                            &db->spi_tx_dma,
                                            GFP_KERNEL | GFP_DMA); //GFP_KERNEL, GFP_DMA

        if (db->spi_tx_buf) {
                /* OK the buffer for DMA */
                callback_setup(1); // 1, also assigned to 'enable_dma'
                dma_bff = 1;
                
                printk("[ *dm9051 DRV ] spi mode[= dma] using db->spi_tx_buf = dma_alloc_coherent(PAGE_SIZE, &db->spi_tx_dma)\n"); //ADD.JJ
                printk("[ *dm9051 DRV ] spi mode[= dma] using 'enable_dma', enable_dma = %d\n", dma_bff); //ADD.JJ
                db->spi_rx_dma = (dma_addr_t)(db->spi_tx_dma +
                        (PAGE_SIZE / 2));
                //db->spi_rx_buf = (db->spi_tx_buf + (PAGE_SIZE / 2));
        } else {
                /* Fall back to non-DMA */
                dma_bff = 0; 
        }
#endif        
        return dma_bff; //return ret;
}

//[spi_dm9051_dma.c]
static int dma_space_request(struct board_info *db)
{
        int dma_bff;
        /* Allocate DMA buffers */
#if 0 //test 
        dma_bff = 0;
#else
        dma_bff = dm9051_is_dma_param(db);
#endif

        if (!dma_bff)
                return std_space_request(db);
        return 0; // no-Err
}
#endif
#endif

//[Implemant code]
#if DEF_REM
#ifdef RPI_CONF_SPI_DMA_YES
//[spi_dm9051_dma.c]
static void dma_space_free(struct board_info *db)
{
        /* Free DMA buffers */
        if (enable_dma) { //.space_free
                printk("[dm951_u-probe].s -Finsih using dma_free_coherent(&db->spidev->dev, PAGE_SIZE, db->spi_tx_buf, db->spi_tx_dma, ..)-\n");  //ADD.JJ
                dma_free_coherent(&db->spidev->dev, PAGE_SIZE, db->spi_tx_buf, db->spi_tx_dma);
        }
        /* Free non-DMA buffers */
        if (!enable_dma) //.space_free, ADD.JJ (JJ)
                std_space_free(db); //~std_free();
}
#endif
#endif

#if DEF_SPIRW

#ifdef RPI_CONF_SPI_DMA_YES //.(more_for_cbCode).
static int dma_spi_xfer_buf(board_info_t *db, unsigned len)
{
        int ret;
#if 0
//.static struct spi_transfer Txfer;
//.static struct spi_message Tmsg; 
//.struct spi_transfer *xfer = &Txfer;
//.struct spi_message *msg =  &Tmsg;
//.spi_message_init(msg);
        xfer->tx_buf = db->spi_tx_buf; 
        xfer->rx_buf = db->spi_tx_buf; 
        xfer->tx_dma = db->spi_tx_dma;
        xfer->rx_dma = db->spi_rx_dma;
        msg->is_dma_mapped = 1;
        xfer->len = len + 1;
        xfer->cs_change = 0;
//.spi_message_add_tail(xfer, msg);
        ret = spi_sync(db->spidev, msg);
#else
        db->fer->tx_buf = db->spi_tx_buf;
        db->fer->rx_buf = db->spi_tx_buf;
        db->fer->tx_dma = db->spi_tx_dma;
        db->fer->rx_dma = db->spi_rx_dma;
        db->msg->is_dma_mapped = 1;
        db->fer->len = len + 1;
        db->fer->cs_change = 0;
        ret = spi_sync(db->spidev, db->msg);
#endif
        if (ret < 0)
                dbg_log("spi communication fail! ret=%d\n", ret);
        return ret;
}
#endif //.(more_for_cbCode).
#endif

#if DEF_SPIRW
#ifdef RPI_CONF_SPI_DMA_YES
static int disp_dma_spi_xfer_Reg(board_info_t *db, unsigned reg)
{
        int ret = 0;
        if (reg == DM9051_PIDL || reg == DM9051_PIDH ) {
                printk("dm905.MOSI.p.[%02x][..]\n",reg); 
        }
        if (reg == DM9051_PIDL || reg == DM9051_PIDH ) {
                printk("dm905.MISO.e.[..][%02x]\n", db->spi_tx_buf[1]);  //~db->spi_sypc_buf[1], 'TxDatBuf'
        }
        return ret;
}

static u8 dma_spi_read_reg(board_info_t *db, unsigned reg)
{
        //if (!enable._dma) //.dma.read_reg
        //  return std_spi_read_reg(db, reg);
                
        db->spi_tx_buf[0] = (DM_SPI_RD | reg); //if (enable._dma) //.dma.read_reg
        dmaXFER(db, 1); //cb.xfer_buf_cb(db, NULL, NULL, 1); //dma_spi_xfer_buf(db, NULL, NULL, 1); //'dm9051_spi_xfer_buf'
        
        //db->spi_sypc_buf[1] = db->spi_tx_buf[1]; //if (enable._dma)//.dma.read_reg  //'TxDatBuf'
  disp_dma_spi_xfer_Reg(db, reg);
        return db->spi_tx_buf[1]; //'TxDatBuf'
}
#endif
#endif

#if DEF_SPIRW
#ifdef RPI_CONF_SPI_DMA_YES
static void dma_spi_write_reg(board_info_t *db, unsigned reg, unsigned val)
{
        //if (!enable._dma) //.write_reg
         //  return std_spi_write_reg(db, reg, val);
        db->spi_tx_buf[0] = (DM_SPI_WR | reg);          //if (enable._dma) {//.dma.write_reg
        db->spi_tx_buf[1] = val;                                            //if (enable._dma) {//.dma.write_reg
         dmaXFER(db, 1); //cb.xfer_buf_cb(db, NULL, NULL, 1); //dma_spi_xfer_buf(db, /*txb*/ NULL, NULL, 1); //'dm9051_spi_xfer_buf'
}
#endif
#endif

#if DEF_SPIRW
#if DEF_PRO 
#ifdef RPI_CONF_SPI_DMA_YES

static void dma_read_rx_buf(board_info_t *db, u8 *buff, unsigned len, bool need_read)
{
        
        //if (!enable._dma)//.read_rx_buf
         //       return std_read_rx_buf(db, buff, len, need_read);
        
        //[this is for the (SPI_SYNC_TRANSFER_BUF_LEN - 1)_buf application.]
        do {
                // [=dma_rdloop_tx_buf]
                unsigned one_pkg_len;
                unsigned remain_len = len, offset = 0;
                db->spi_tx_buf[0] = (DM_SPI_RD | DM_SPI_MRCMD);
                do {
                        // 1 byte for cmd
                        if (remain_len <= (SPI_SYNC_TRANSFER_BUF_LEN - 1)) {
                                one_pkg_len = remain_len;
                                remain_len = 0;
                        } else {
                                one_pkg_len = (SPI_SYNC_TRANSFER_BUF_LEN - 1);
                                remain_len -= (SPI_SYNC_TRANSFER_BUF_LEN - 1);
                        }

                        dmaXFER(db, one_pkg_len); //cb.xfer_buf_cb(db, NULL, /* NULL */ db->TxDatBuf, one_pkg_len); //dma_spi_xfer_buf(db, /* txb */ NULL,  /* NULL */ db->TxDatBuf, one_pkg_len); //'dm9051_spi_xfer_buf'
                        if (need_read) {
                                memcpy(buff + offset, &db->spi_tx_buf[1], one_pkg_len); //if (enable._dma)//.dma.read_rx_buf
                                offset += one_pkg_len;
                        }
                } while (remain_len > 0);
        } while(0);
}

#if DMA3_P2_RSEL_1024F
static void dma_read_rx_buf_1024(board_info_t *db, u8 *buff, unsigned len)
{
        //[this is for the 1024_buf application.(with copy operations)][It's better no-copy]
	int const pkt_count = (len + 1) / CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	int const remainder = (len + 1) % CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
                db->spi_tx_buf[0] = (DM_SPI_RD | DM_SPI_MRCMD);
		if (pkt_count) {
			int blkLen;
			//(1)
			blkLen= CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1; // minus 1, so real all is 1024 * n
			dmaXFER(db, blkLen);
			memcpy(&buff[1], &db->spi_tx_buf[1], blkLen);
	        //.printk("dm9rx_EvenPar_OvLimit(%d ... \n", blkLen);
			//(1P)
			if (remainder) {
			  db->spi_tx_buf[0] = (DM_SPI_RD | DM_SPI_MRCMD); // dma XFER need re-fill read_cmd_field.
			  dmaXFER(db, remainder);
			  memcpy(buff + (CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count), &db->spi_tx_buf[1], remainder);
		//.printk("dm9rx_EvenPar_OvRemainder(%d ... \n", blkLen);
                        }
                        return;
                }
		//(2)	
		if (remainder) {
                        dmaXFER(db, len);
                        memcpy(&buff[1], &db->spi_tx_buf[1], len);
			//note: len= remainder-1
		}
		return;
}
#else
static void dma_read_rx_buf_cpy(board_info_t *db, u8 *buff, unsigned len)
{
        //[this is for the 0_buf application.][It's better no-copy]
                db->spi_tx_buf[0] = (DM_SPI_RD | DM_SPI_MRCMD);
                dmaXFER(db, len);
                memcpy(&buff[1], &db->spi_tx_buf[1], len);
                        //Because from: [dm9051_inblk_noncpy(db, rdptr-1, RxLen);]
}
#endif

#endif
#endif
#endif

#if DEF_SPIRW
#if DEF_PRO 
#ifdef RPI_CONF_SPI_DMA_YES

#if DMA3_P2_TSEL_1024F
static int dma_write_tx_buf_1024(board_info_t *db, u8 *buff, unsigned len)
{
	int blkLen;
	int const pkt_count = (len + 1)/ CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	int const remainder = (len + 1)% CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	unsigned offset = 0;
	if((len + 1)>CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES){
		//(1)
		blkLen= CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1;
                db->spi_tx_buf[0] = (DM_SPI_WR | DM_SPI_MWCMD);
                memcpy(&db->spi_tx_buf[1], &buff[offset], blkLen);
                offset += blkLen;
                dmaXFER(db, blkLen);
        //printk("dm9tx_dma_EvenPar_OvLimit(%d ... \n", blkLen);
		//(2)	
		blkLen= remainder;
                db->spi_tx_buf[0] = (DM_SPI_WR | DM_SPI_MWCMD); //need re-fill again.
                memcpy(&db->spi_tx_buf[1], &buff[offset], blkLen);
                //offset += blkLen;
                dmaXFER(db, blkLen);
        //printk("dm9tx_dma_EvenPar_OvRemainder(%d ... \n", blkLen);
	} else {
		db->spi_tx_buf[0] = DM_SPI_WR | DM_SPI_MWCMD;
                memcpy(&db->spi_tx_buf[1], buff, len);
                dmaXFER(db, len);
        }
        return 0;
}
#else
static int dma_write_tx_buf(board_info_t *db, u8 *buff, unsigned len)
{
        //if (!enable._dma) //.write_tx_buf
        //        return std_write_tx_buf(db, buff, len);
                
        do {
                // [= dma_wrloop_tx_buf]
                unsigned remain_len = len;
                unsigned pkg_len, offset = 0;
                do {
                        // 1 byte for cmd
                        if (remain_len <= (SPI_SYNC_TRANSFER_BUF_LEN - 1)) {
                                pkg_len = remain_len;
                                remain_len = 0;
                        } else {
                                pkg_len = (SPI_SYNC_TRANSFER_BUF_LEN - 1);
                                remain_len -= (SPI_SYNC_TRANSFER_BUF_LEN - 1);
                        }

                        //if (enable._dma) {//.write_tx_buf
                        //}
                        db->spi_tx_buf[0] = (DM_SPI_WR | DM_SPI_MWCMD);
                        memcpy(&db->spi_tx_buf[1], buff + offset, pkg_len);
                        
                        offset += pkg_len;
                        dmaXFER(db, pkg_len); //cb.xfer_buf_cb(db, /* NULL */ db->TxDatBuf, NULL, pkg_len); //dma_spi_xfer_buf(db, /* NULL */ db->TxDatBuf, NULL, pkg_len); //'dm9051_spi_xfer_buf'
                } while (remain_len > 0);
        } while(0);
        return 0;
}
#endif

#endif
#endif
#endif

void callback_setup(int dma_bff)
{
#ifdef RPI_CONF_SPI_DMA_YES 
        enable_dma = dma_bff;
   
        #if DEF_SPIRW
        if (enable_dma) {
                dm9.iorb= dma_spi_read_reg;
                dm9.iowb= dma_spi_write_reg;
                dm9.inblk_defcpy= dma_read_rx_buf;  // 1st byte is the rx data.
                dm9.inblk_noncpy= dmaRX; // reserve 1 byte in the head. // dma_ with_ ncpy_
                dm9.outblk= dmaTX;
        } else {
                dm9.iorb= std_spi_read_reg;
                dm9.iowb= std_spi_write_reg;
                dm9.inblk_defcpy= std_read_rx_buf;  // 1st byte is the rx data.
                dm9.inblk_noncpy= stdRX;
                dm9.outblk= stdTX;
        }
        #endif
#else
        #if DEF_SPIRW
        dm9.iorb= std_spi_read_reg;
        dm9.iowb= std_spi_write_reg;
        dm9.inblk_defcpy= std_read_rx_buf;  // 1st byte is the rx data.
        dm9.inblk_noncpy= stdRX;
        dm9.outblk= stdTX;
        //#if DMA3_P2_RSEL_1024F
        //#else
        //#endif
        //#if DMA3_P2_TSEL_1024F
        //#else
        //#endif
        #endif
#endif

#if 0
#ifdef RPI_CONF_SPI_DMA_YES
        //..gXfer = (enable_dma)? dma_spi_xfer_buf :std_spi_xfer_buf; // callback.
        //..cb.xfer = (enable_dma)? dma_spi_xfer_buf :std_spi_xfer_buf; // callback.
#endif
#ifndef RPI_CONF_SPI_DMA_YES
        //.gXfer = std_spi_xfer_buf;
        //.cb.xfer = std_spi_xfer_buf;
#endif
#endif
}

void dm9051_spimsg_init(board_info_t *db)
{
        //spi_message_init(&dm9.Tmsg);
        //spi_message_add_tail(&dm9.Tfer,&dm9.Tmsg);
        #if DEF_SPICORE_IMPL1
        spi_message_init(&db->Tmsg);
        spi_message_add_tail(&db->Tfer,&db->Tmsg);
        db->fer = &db->Tfer;
        db->msg = &db->Tmsg;
        #endif
}
