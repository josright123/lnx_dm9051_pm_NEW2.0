
//[Header definitions]

#ifdef RPI_CONF_SPI_DMA_YES
#define dm9051_space_alloc  dma_space_request  //#define dm9051_space_request dma_space_request
#define dm9051_space_free    dma_space_free
#endif
#ifndef RPI_CONF_SPI_DMA_YES
#define dm9051_space_alloc  std_space_request  //#define dm9051_space_request std_space_request
#define dm9051_space_free    std_space_free
#endif

//dm9051_dbg_alloc(d); 
//dm9051_dbg_free(d); 
#if DEF_SPIRW
#define dm9051_spirw_begin(d) dm9051_space_alloc(d)
#define dm9051_spirw_end(d)      dm9051_space_free(d)
#else
#define dm9051_spirw_begin(d)  // Never called, only called while define _DEF_SPIRW in above if-condition.
#define dm9051_spirw_end(d)  // Essentially called.
#endif

//[Usage definitions]
//Usage
#define dmaXFER  dma_spi_xfer_buf
#define stdXFER  std_spi_xfer_buf
//Opt
#if DMA3_P2_RSEL_1024F
#define dmaRX 	dma_read_rx_buf_1024 // reserve 1 byte in the head. // dma_ with_ ncpy_
#else
#define dmaRX 	dma_read_rx_buf_cpy // reserve 1 byte in the head. // dma_ with_ ncpy_
#endif
#if DMA3_P2_TSEL_1024F
#define dmaTX 	dma_write_tx_buf_1024
#else
#define dmaTX 	dma_write_tx_buf
#endif
#if DMA3_P2_RSEL_1024F
#define stdRX	std_read_rx_buf_1024
#else
#define stdRX	std_read_rx_buf_ncpy
#endif
#if DMA3_P2_TSEL_1024F
#define stdTX 	std_write_tx_buf_1024
#else
 #ifdef QCOM_TX_DWORD_BOUNDARY
 #define stdTX 	std_write_tx_buf_dword_boundary
 #else
 #define stdTX 	std_write_tx_buf
 #endif
#endif

#if 0 // no used.
//[Functions cast definitions]
#define dm9051_read_rx_buf  dm9.inblk_defcpy_or_dm9.inblk_noncpy
#define dm9051_write_tx_buf  dm9.outblk
//[Functions cast definitions]
#define dm9051_spi_read_reg     dm9.iorb
#define dm9051_spi_write_reg     dm9.iowb

//#ifdef RPI_CONF_SPI_DMA_YES
//#define dm9051_spi_xfer_buf  dma_spi_xfer_buf
//#define dm9051_read_rx_buf  dma_read_rx_buf
//#define dm9051_write_tx_buf  dma_write_tx_buf
//#else
//#define dm9051_spi_xfer_buf  std_spi_xfer_buf
//#define dm9051_read_rx_buf  std_read_rx_buf
//#define dm9051_write_tx_buf  std_write_tx_buf
//#endif
//#ifdef RPI_CONF_SPI_DMA_YES
//#define dm9051_spi_read_reg     dma_spi_read_reg 
//#define dm9051_spi_write_reg     dma_spi_write_reg
//#else
//#define dm9051_spi_read_reg     std_spi_read_reg
//#define dm9051_spi_write_reg     std_spi_write_reg
//#endif
#endif // no used.

//[APIs definitions]
#if DEF_SPICORE
#define ior					dm9.iorb
#define iior					dm9.iorb
#define iow					dm9.iowb
#define iiow					dm9.iowb
#define dm9051_outblk				dm9.outblk
#define dm9051_inblk_rxhead(d,b,l)		dm9.inblk_defcpy(d,b,l,true)
#define dm9051_inblk_noncpy(d,b,l)		dm9.inblk_noncpy(d,b,l)
#define dm9051_inblk_dump(d,l)			dm9.inblk_defcpy(d,NULL,l,false)
#endif

//Temp
#if DEF_SPIRW
typedef struct cb_info_t {
	/*[int (*xfer)(board_info_t *db unsigned len);]*/ //by.= /'dmaXFER'
        /*[int (*xfer)(board_info_t *db, u8 *txb, u8 *rxb, unsigned len);]*/ //by.= /'stdXFER'
         u8 (*iorb)(board_info_t *db, unsigned reg);
         void (*iowb)(board_info_t *db, unsigned reg, unsigned val);
         void (*inblk_defcpy)(board_info_t *db, u8 *buff, unsigned len, bool need_read); // 1st byte is the rx data.
         void (*inblk_noncpy)(board_info_t *db, u8 *buff, unsigned len); // reserve 1 byte in the head.
         int (*outblk)(board_info_t *db, u8 *buff, unsigned len);
	 /*struct spi_transfer Tfer;
	 struct spi_message Tmsg; 
	 //struct spi_transfer *fer;
	 //struct spi_message *msg;
	 */
} cb_info;

static cb_info dm9;
#endif

void callback_setup(int dma_bff);  // Setup the call back functions.

//[Implemant code]
#if DEF_PRO & DEF_SPIRW
static int  std_alloc(struct board_info *db)
{
        #ifdef RPI_CONF_SPI_DMA_YES
        printk("[ *dm9051 DRV ] spi mode[= std] using 'enable_dma' is 0\n");
        printk("[ *dm9051 DRV ] spi mode[= dma] But using kmalloc, TxDatBuf[] or std_alloc TxDatBuf\n"); //ADD.JJ
        #else
        printk("[ *dm9051 DRV ] spi mode[= std] using kmalloc, TxDatBuf[] or std_alloc TxDatBuf\n"); //ADD.JJ
        #endif

	#if DEF_SPIRW
        db->spi_sypc_buf = kmalloc(SPI_SYNC_TRANSFER_BUF_LEN, GFP_ATOMIC);
        #if 0
        printk("[ *dm9051 DRV ] spi mode[= std] using devm_kzalloc, TxDatBuf[] or std_alloc TxDatBuf\n"); //ADD.JJ
        db->spi_sypc_buf = devm_kzalloc(&spi->dev, SPI_SYNC_TRANSFER_BUF_LEN, GFP_KERNEL);
        #endif
        
        if (!db->spi_sypc_buf)
                return -ENOMEM;
	#endif
		
        return 0; // no-Err
}

//[EXTRA]
static int dm9051_dbg_alloc(struct board_info *db)
{
#ifdef DM_CONF_SPI_TEST_BLKIN_SPLIT
	db->blkin = kmalloc(DM_CONF_SPI_TEST_BLKLEN, GFP_ATOMIC);
        if (!db->blkin)
                return -ENOMEM;
#endif

#ifdef FREE_NO_DOUBLE_MEM_MAX
	db->prebuf = kmalloc((SCAN_LEN_HALF+1)*1, GFP_ATOMIC); 
#else
	db->prebuf = kmalloc((SCAN_LEN_HALF+1)*2, GFP_ATOMIC);   //or 'SCAN_LEN'
#endif

        if (!db->prebuf) { 
#ifdef DM_CONF_SPI_TEST_BLKIN_SPLIT
		kfree(db->blkin);
#endif		
                return -ENOMEM;
	}                             
	//[db->scanmem= 0;]
#ifdef FREE_NO_DOUBLE_MEM_MAX
	db->sbuf = db->prebuf;
#else
	db->sbuf = db->prebuf + (SCAN_LEN_HALF+1);
#endif
	return 0;
}
static void  dm9051_dbg_free(struct board_info *db)
{
#ifdef DM_CONF_SPI_TEST_BLKIN_SPLIT
	kfree(db->blkin);
#endif
	kfree(db->prebuf);
}
//#ifndef RPI_CONF_SPI_DMA_YES
//#endif
static int std_space_request(struct board_info *db)
{
        /* Alloc non-DMA buffers */
        callback_setup(0); // assign 0 to 'enable_dma'
        return std_alloc(db);
}
#endif

//[Implemant code]
#if DEF_REM & DEF_SPIRW
static void  std_free(struct board_info *db)
{
                printk("[dm951_u-probe].s ------- Finsih using kfree, param (db->spi_sypc_buf) -------\n");  //ADD.JJ //'TxDatBuf'
		
		#if DEF_SPIRW
                kfree(db->spi_sypc_buf);
                #if 0
                printk("[dm951_u-probe].s ------- Finsih using devm_kfree, param (&db->spidev->dev, db->spi_sypc_buf) -------\n");  //ADD.JJ //'TxDatBuf'
                devm_kfree(&db->spidev->dev, db->spi_sypc_buf); //'TxDatBuf'
                #endif
		#endif
}

//#ifndef RPI_CONF_SPI_DMA_YES
//#endif
static void std_space_free(struct board_info *db)
{
        /* Free non-DMA buffers */
         std_free(db);
}
#endif

#if DEF_SPIRW
static int std_spi_xfer_buf(board_info_t *db, u8 *txb, u8 *rxb, unsigned len)
{
        int ret;
#if 0
        //static struct spi_transfer Txfer;
        //static struct spi_message Tmsg; 
        //struct spi_transfer *xfer = &Txfer;
        //struct spi_message *msg =  &Tmsg;
        //spi_message_init(msg);
        xfer->tx_buf = txb;
        xfer->rx_buf = rxb;
        xfer->len = len + 1;
        xfer->cs_change = 0;
        //spi_message_add_tail(xfer, msg);
        ret = spi_sync(db->spidev, msg);
#else //'DEF_SPICORE_IMPL1'
	db->fer->tx_buf = txb;
	db->fer->rx_buf = rxb;
	db->fer->len = len + 1;
	db->fer->cs_change = 0;
	ret = spi_sync(db->spidev, db->msg);
#endif
        if (ret < 0) {
                dbg_log("spi communication fail! ret=%d\n", ret);
        }
        return ret;
}
#endif

#if DEF_SPIRW

static int disp_std_spi_xfer_Reg(board_info_t *db, unsigned reg)
{
        int ret = 0;
        if (reg == DM9051_PIDL || reg == DM9051_PIDH ) {
                printk("dm905.MOSI.p.[%02x][..]\n",reg); 
        }
        if (reg == DM9051_PIDL || reg == DM9051_PIDH ) {
                printk("dm905.MISO.e.[..][%02x]\n", db->spi_sypc_buf[1]);  //'TxDatBuf'
        }
        return ret;
}

static u8 std_spi_read_reg(board_info_t *db, unsigned reg)
{
        u8 txb[2] = {0};
        u8 rxb[2] = {0};

        txb[0] = (DM_SPI_RD | reg);
        stdXFER(db, (u8 *)txb, rxb, 1); //cb.xfer_buf_cb(db, (u8 *)txb, rxb, 1); //std_spi_xfer_buf(db, (u8 *)txb, rxb, 1); //'dm9051_spi_xfer_buf'
        
  db->spi_sypc_buf[1] = rxb[1]; //.std.read_reg //'TxDatBuf'
  disp_std_spi_xfer_Reg(db, reg);
        return rxb[1];
}
#endif

#if DEF_SPIRW
static void std_spi_write_reg(board_info_t *db, unsigned reg, unsigned val)
{
        u8 txb[2] = {0};
        //if (!enable._dma) {
        //}
        txb[0] = (DM_SPI_WR | reg);
        txb[1] = val;
        stdXFER(db, (u8 *)txb, NULL, 1); //cb.xfer_buf_cb(db, (u8 *)txb, NULL, 1); //std_spi_xfer_buf(db, (u8 *)txb, NULL, 1); //'dm9051_spi_xfer_buf'
}
#endif

#if DEF_SPIRW
#if DEF_PRO 
  //&& DEF_SPIRW
  //&& DM_CONF_APPSRC
static void std_read_rx_buf(board_info_t *db, u8 *buff, unsigned len, bool need_read)
{
        //[this is for the (SPI_SYNC_TRANSFER_BUF_LEN - 1)_buf application.]
        unsigned one_pkg_len;
        unsigned remain_len = len, offset = 0;
        u8 txb[1];
        txb[0] = DM_SPI_RD | DM_SPI_MRCMD;
        do {
                // 1 byte for cmd
                if (remain_len <= (SPI_SYNC_TRANSFER_BUF_LEN - 1)) {
                        one_pkg_len = remain_len;
                        remain_len = 0;
                } else {
                        one_pkg_len = (SPI_SYNC_TRANSFER_BUF_LEN - 1);
                        remain_len -= (SPI_SYNC_TRANSFER_BUF_LEN - 1);
                }

                stdXFER(db, txb, db->spi_sypc_buf, one_pkg_len); //cb.xfer_buf_cb(db, txb, db->TxDatBuf, one_pkg_len); //std_spi_xfer_buf(db, txb, db->TxDatBuf, one_pkg_len); //'dm9051_spi_xfer_buf'
                if (need_read) {
                        #if 0
                        //test.ok.
                        if (one_pkg_len==4)
                        {
                                printk("Head %02x %02x %02x %02x\n", db->spi_sypc_buf[1], db->spi_sypc_buf[2], db->spi_sypc_buf[3], db->spi_sypc_buf[4]);
                        }
                        #endif
                        memcpy(buff + offset, &db->spi_sypc_buf[1], one_pkg_len); //if (!enable._dma)//.read_rx_buf //'TxDatBuf'
                        offset += one_pkg_len;
                }
        } while (remain_len > 0);
}

#if DMA3_P2_RSEL_1024F
static void std_read_rx_buf_1024(board_info_t *db, u8 *buff, unsigned len)
{
        //[this is for the 1024_buf application.(with copy operations)][It's better no-copy]
	u8 txb[1];
	int const pkt_count = (len + 1) / CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	int const remainder = (len + 1) % CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	//.if((len + 1)>CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES){	
		txb[0] = DM_SPI_RD | DM_SPI_MRCMD;
		if (pkt_count) {
			int blkLen;
			//(1)
			blkLen= CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1; // minus 1, so real all is 1024 * n
			stdXFER(db, txb, db->spi_sypc_buf, /*RD_LEN_ONE +*/ blkLen);
			memcpy(&buff[1], &db->spi_sypc_buf[1], /*RD_LEN_ONE +*/ blkLen);
	        //.printk("dm9rx_EvenPar_OvLimit(%d ... \n", blkLen);
			//(1P)
			if (remainder) {
			  //.blkLen= remainder;
			  stdXFER(db, txb, db->spi_sypc_buf, /*RD_LEN_ONE +*/ remainder);
			  memcpy(buff + (CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count), &db->spi_sypc_buf[1], remainder);
		//.printk("dm9rx_EvenPar_OvRemainder(%d ... \n", blkLen);
			}
			return;
		}
		//(2)	
		if (remainder) {
			//stdXFER(db, txb, db->spi_sypc_buf, remainder-1);
			//memcpy(&buff[1], &db->spi_sypc_buf[1], remainder-1);
			//note: len= remainder-1
			stdXFER(db, txb, buff, len);
		}
		return;
	//.}
}
#else
static void std_read_rx_buf_ncpy(board_info_t *db, u8 *buff, unsigned len)
{
        //[this is for the 0_buf application.][It's no-copy]
        u8 txb[1];
        txb[0] = DM_SPI_RD | DM_SPI_MRCMD;
        stdXFER(db, txb, buff, len);
}
#endif

#endif
#endif

#if DEF_SPIRW
#if DEF_PRO 
  //&& DEF_SPIRW
  //&& DM_CONF_APPSRC
  
#if DMA3_P2_TSEL_1024F
static int std_write_tx_buf_1024(board_info_t *db, u8 *buff, unsigned len)
{
	int blkLen;
	int const pkt_count = (len + 1)/ CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	int const remainder = (len + 1)% CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	unsigned offset = 0;
	
	if((len + 1)>CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES){
		//(1)
		blkLen= CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1;
		db->spi_sypc_buf[0] = DM_SPI_WR | DM_SPI_MWCMD;
		memcpy(&db->spi_sypc_buf[1], &buff[offset], blkLen);
                offset += blkLen;
		stdXFER(db, db->spi_sypc_buf, NULL, blkLen);
        //.printk("dm9tx_std_EvenPar_OvLimit(%d ... \n", blkLen);
		
		/*xfer->tx_buf = db->spi_sypc_buf;
		xfer->rx_buf = NULL;
		xfer->len = RD_LEN_ONE + blkLen; // minus 1, so real all is 1024 * n
		if(spi_sync(db->spidev, &db->spi_msg1))
			printk("[dm95_spi]INNO txERR1: len= %d of %d, txbuf=0x%p,rxbuf=0x%p",blkLen,len,xfer->tx_buf,xfer->rx_buf);*/

		//(2)	
		blkLen= remainder;
		memcpy(&db->spi_sypc_buf[1], &buff[offset], blkLen);
                //offset += blkLen;
		stdXFER(db, db->spi_sypc_buf, NULL, blkLen);
        //.printk("dm9tx_std_EvenPar_OvRemainder(%d ... \n", blkLen);
		
		/*xfer->tx_buf = db->spi_sypc_buf;
		xfer->rx_buf = NULL; 
		xfer->len = RD_LEN_ONE + remainder; 
		if (spi_sync(db->spidev, &db->spi_msg1)) //(INNODev_sync(db))
			printk("[dm95_spi]INNO txERR2: len=%d of %d, txbuf=0x%p,rxbuf=0x%p",blkLen,len,xfer->tx_buf,xfer->rx_buf);*/
	} else {
		db->spi_sypc_buf[0] = DM_SPI_WR | DM_SPI_MWCMD;
		memcpy(&db->spi_sypc_buf[1], buff, len);
		stdXFER(db, db->spi_sypc_buf, NULL, len);
		
		/*xfer->tx_buf = db->spi_sypc_buf;
		xfer->rx_buf = NULL; 
		xfer->len = RD_LEN_ONE + len;
		if (spi_sync(db->spidev, &db->spi_msg1))
			printk("[dm95_spi]INNO ERROR: len=%d, txbuf=0x%p,rxbuf=0x%p",len,xfer->tx_buf,xfer->rx_buf);*/
	}
        return 0;
}
#else
#ifdef QCOM_TX_DWORD_BOUNDARY
static int std_write_tx_buf_dword_boundary(board_info_t *db, u8 *buff, unsigned len)
{
        unsigned remain_len = len;
        unsigned pkg_len, offset = 0;
	
//.	printk("[dm9][tx %d, dword-bound] %02x %02x %02x %02x %02x %02x", 
		//remain_len, buff[0], buff[1], buff[2], buff[3], buff[4], buff[5]);
	
        do {
                // 1 byte for cmd
                if (remain_len <= (SPI_SYNC_TRANSFER_BUF_LEN - 1)) {
			
                        pkg_len = remain_len;
			
                } else {
			
                        pkg_len = (SPI_SYNC_TRANSFER_BUF_LEN - 1);
			
                }
		
		if ((pkg_len+1) < 4) {
			pkg_len = 1;
		} else {

			pkg_len = ((pkg_len + 1) >> 2) << 2; //[new for dword boundary]
			pkg_len--;
			//pkg_len = ((((pkg_len + 1) + 3) /4 )*4) - 4; //[new for dword boundary]
			//pkg_len  -= 1;
		}
		
		remain_len -= pkg_len;

                db->spi_sypc_buf[0] = DM_SPI_WR | DM_SPI_MWCMD; //if (!enable._dma) { } //'TxDatBuf'
                memcpy(&db->spi_sypc_buf[1], buff + offset, pkg_len); //'TxDatBuf'
                
    //.if (!remain_len && (pkg_len!=len)){
    	//.switch (pkg_len){
    	//.	case 3:
    	//.		printk(" %02x %02x %02x", buff[offset], buff[offset+1], buff[offset+2]);
    	//.		break;
    	//.	case 1:
    	//.		printk(" %02x", buff[offset]);
    	//.		break;
    	//.}
    	//.printk(" [end.t.xfer %d]", pkg_len);
    //.} else {
    	//.switch (pkg_len){
    	//.	case 1:
    	//.		printk(" %02x", buff[offset]);
    	//.		break;
    	//.}
			//.printk(" [t.xfer %d]", pkg_len);
		//.}
                
                offset += pkg_len;
                stdXFER(db, db->spi_sypc_buf, NULL, pkg_len); //cb.xfer_buf_cb(db, db->TxDatBuf, NULL, pkg_len); //std_spi_xfer_buf(db, db->TxDatBuf, NULL, pkg_len); //'dm9051_spi_xfer_buf'
        } while (remain_len > 0);
	//.printk("\n");
        return 0;
}
#endif
#ifndef QCOM_TX_DWORD_BOUNDARY
static int std_write_tx_buf(board_info_t *db, u8 *buff, unsigned len)
{
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

                db->spi_sypc_buf[0] = DM_SPI_WR | DM_SPI_MWCMD; //if (!enable._dma) { } //'TxDatBuf'
                memcpy(&db->spi_sypc_buf[1], buff + offset, pkg_len); //'TxDatBuf'
                
                offset += pkg_len;
                stdXFER(db, db->spi_sypc_buf, NULL, pkg_len); //cb.xfer_buf_cb(db, db->TxDatBuf, NULL, pkg_len); //std_spi_xfer_buf(db, db->TxDatBuf, NULL, pkg_len); //'dm9051_spi_xfer_buf'

        } while (remain_len > 0);
        return 0;
}
#endif
#endif

#endif
#endif

#if 0
//static void dm9051_read_rx_buf(board_info_t *db, u8 *buff, unsigned len, bool need_read)
//{
//        if (need_read) { // need data but no buff, return
//                if (!buff) {
//                        dbg_log("rx U8* buff fail\r\n");
//                        return;
//                }
//        }
//        if (len <= 0) {
//                dbg_log("rx length fail\r\n");
//                return ;
//        }
//        do {
//        } while (remain_len > 0);
//}
#endif

#if 0
//static int dm9051_write_tx_buf(board_info_t *db, u8 *buff, unsigned len)
//{
//        if (len > DM9051_PKT_MAX) {
//                dbg_log("warning: send buffer overflow!!!\n");
//                return -1;
//        }
//        do {
//        } while (remain_len > 0);
//        return 0;
//}
#endif

#ifdef MUST_STATIC_DECL
/*static int dm9051_spi_xfer_bufXXReg(struct spi_device *spi)
{
        board_info_t *db = spi_get_drvdata(spi);
        static struct spi_transfer xfer;
        static struct spi_message msg;
        int ret;
        u8 rxb[4] = {0};

        spi_message_init(&msg);

        xfer.tx_buf = db->TxDatBuf;
        xfer.rx_buf = rxb; //[db->TxDatBuf;]
        xfer.len = 2;

        //xfer.cs_change = 0;
    
        //struct spi_transfer *xfer = &db->spi_xfer1;
        //struct spi_message *msg = &db->spi_msg1;
        //xfer->tx_buf = txb;
        //xfer->rx_buf = rxb;
        //xfer->len = len + 1;
        //xfer->cs_change = 0;
        //if (enable._dma) {
        //  xfer->tx_dma = db->spi_tx_dma;
        //  xfer->rx_dma = db->spi_rx_dma;
        //  msg->is_dma_mapped = 1;
        //}

        spi_message_add_tail(&xfer, &msg);
        ret = spi._sync(spi, &msg);
        
        db->TxDatBuf[1] = rxb[1]; //when read.
        if (ret) {
                dbg_log("spi transfer failed: ret=%d\n", ret);
        }
        return ret;
}*/
#endif //0

//static u8 dm9051_spi_read_reg(board_info_t *db, unsigned reg) {
// #if 0
        //u8 txb[4] = {0};
        //u8 rxb[4] = {0};
// db->TxDatBuf[0] = (DM_SPI_RD | reg);
// dm9051_spi_xfer_bufXXReg(db->spidev);
// return db->TxDatBuf[1];
// #endif        
//}

//static void dm9051_spi_write_reg(board_info_t *db, unsigned reg, unsigned val) {
// #if 0        
// db->TxDatBuf[0] = (DM_SPI_WR | reg);
// db->TxDatBuf[1] = val;
// dm9051_spi_xfer_bufXXReg(db->spidev);
// #endif        
//}

#if 0 //PHASE-out
//static u8 dm9051_spi_read_regXX(board_info_t *db, /*int*/ unsigned reg)
//{
//        u8 txb[4] = {0};
//        u8 rxb[4] = {0};
//        txb[0] = (DM_SPI_RD | reg);
//        dm9051._spi_xfer_buf(db, (u8 *)txb, (u8 *)rxb, 1);
//        return rxb[1];
//}
//static void dm9051_spi_write_regXX(board_info_t *db, /*int*/ unsigned reg, /*int*/ unsigned val)
//{
//        u8 txb[2] = {0};
//        txb[0] = (DM_SPI_WR | reg);
//        txb[1] = val;
//        dm9051._spi_xfer_buf(db, (u8 *)txb, NULL, 1);
//}
#endif

#if DEF_SPICORE_IMPL0
#define RD_LEN_ONE	1
//-----------------------------------------------------
// ---   io operate  (all spi read/write dm9051) ---
//-----------------------------------------------------
void wbuff(unsigned op, __le16 *txb)
{
  //op= DM_SPI_WR | reg | val
	txb[0] = cpu_to_le16(op);
}

void wbuff_u8(u8 op, u8 *txb)
{
	txb[0]= op;
}

void xrdbyte(board_info_t * db, __le16 *txb, u8 *trxb)
{
	struct spi_transfer *xfer= &db->spi_xfer1;
	struct spi_message *msg= &db->spi_msg1;
	int ret;
	
	xfer->tx_buf = txb;
	xfer->rx_buf = trxb;
	xfer->len = 2;

	ret = spi_sync(db->spidev, msg);
	if (ret < 0)
		netdev_err(db->ndev, "spi_.sync()fail (xrd.byte 0x%04x) ret= %d\n", txb[0], ret);
}

void xwrbyte(board_info_t * db, __le16 *txb)
{
	struct spi_transfer *xfer = &db->spi_xfer1;
	struct spi_message *msg = &db->spi_msg1;
	int ret;
	
  #ifdef DM_CONF_VSPI
	return;
  #endif	
	
	xfer->tx_buf = txb;
	xfer->rx_buf = NULL;
	xfer->len = 2;

	ret = spi_sync(db->spidev, msg);
	if (ret < 0)
		netdev_err(db->ndev, "spi_.sync()failed (xwrbyte 0x%04x)\n", txb[0]);
}

void xrdbuff_u8(board_info_t *db, u8 *txb, u8 *trxb, unsigned len) //xwrbuff
{
	struct spi_transfer *xfer = &db->spi_xfer1;
	struct spi_message *msg = &db->spi_msg1;
	int ret;
	
#ifdef DM_CONF_VSPI
		trxb[1]= 0;
        return;
#endif	
		//(One byte)
        xfer->tx_buf = txb;
        xfer->rx_buf = trxb;
        xfer->len = RD_LEN_ONE + len;
		ret = spi_sync(db->spidev, msg);
		if (ret < 0){
	    	printk("9051().e out.in.dump_fifo4, %d BYTES, ret=%d\n", len, ret); //"%dth byte", i
			printk(" <failed.e>\n");
		}
//u8	return trxb[1];
}

//-----------------------------------------------------
/* --- custom model --- */
/* routines for sending block to chip */
/*static int INNODev_sync(board_info_t *db)
{
	return spi_sync(db->spidev, &db->spi_msg1); //'msg'
	
	//int ret;
	//mutex_lock(&db->sublcd_mtkspi_mutex);
	//ret= spi_sync(db->spidev, &db->spi_dmsg1);
	//mutex_unlock(&db->sublcd_mtkspi_mutex);
	//if(ret)
		//printk("[dm95_spi] spi.sync fail ret= %d, should check", ret);
	//return ret;
}*/
#endif

#if DEF_SPICORE_IMPL0
void dwrite_1024_Limitation(board_info_t *db, u8 *txb, u8 *trxb, int len)
{
	int blkLen;
	
	//struct spi_transfer *xfer;
		//xfer= &db->spi_dxfer1;
		//xfer= &db->spi_dxfer1;
		//xfer= &db->spi_dxfer1;
	struct spi_transfer *xfer = &db->spi_xfer1;
	//[&db->spi_msg1]
	//struct spi_message *msg = &db->spi_msg1;

#if 0 //# ifdef "DM_CONF_ADVENCE_MEM_YES"
	// advance memory usage
	
	//int const pkt_count = (len )/ CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	//int const remainder = (len )% CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	int const pkt_count = (len + 1)/ CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	//int const remainder = (len + 1)% CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	  //1-0.
	  //..
	    wbuff_u8(DM_SPI_WR | DM_SPI_MWCMD, db->TxDatBuf); //'RD_LEN_ONE'
		xfer->tx_buf = db->TxDatBuf;
		xfer->rx_buf = NULL;
		//xfer->len = RD_LEN_ONE ; 
		//if(spi_sync(db->spidev, &db->spi_msg1))
		//	printk("[dm95_spi]INNO txERR1.0: len= %d of %d, txbuf=0x%p,rxbuf=0x%p",1,len,xfer->tx_buf,xfer->rx_buf); //return INNO_GENERAL_ERROR;
	  //1-1.
	  //...
	  if (pkt_count)
		blkLen= CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1;
	  else
		blkLen= len;
	  //blkLen= CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count ;
	  memcpy(db->TxDatBuf+1, txb, blkLen);
	  //xfer->tx_buf = txb;
		//xfer->rx_buf = NULL;
		//xfer->len = blkLen ; 
		xfer->len = RD_LEN_ONE + blkLen ; 
		if(spi_sync(db->spidev, &db->spi_msg1))
			printk("[dm95_spi]INNO txERR1.1-1: len= %d of %d, txbuf=0x%p,rxbuf=0x%p",blkLen,len,xfer->tx_buf,xfer->rx_buf); 
	  //2.
		//..
		txb[blkLen-1] = db->TxDatBuf[0];
		//...
		blkLen = len - blkLen;
		if (blkLen) {
			//.blkLen= remainder;
			xfer->tx_buf = &txb[blkLen-1];
			xfer->rx_buf = NULL;
			xfer->len = RD_LEN_ONE + blkLen ; 
			if(spi_sync(db->spidev, &db->spi_msg1))
				printk("[dm95_spi]INNO txERR1.2: len= %d of %d, txbuf=0x%p,rxbuf=0x%p",/*remainder*/ blkLen ,len,xfer->tx_buf,xfer->rx_buf); 
		}
#else
	
	int const pkt_count = (len + 1)/ CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	int const remainder = (len + 1)% CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;

	if((len + 1)>CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES){
		//(1)
		blkLen= CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1;
		//[TxDatBuf]
		//.printk("9Tx tbf=0x%p,rbf=%s [len, blkLen %d= %d + %d])\n", db->TxDatBuf, "NULL", len, blkLen +1, remainder - 1); // +1 for himan-read, -1 also

	    wbuff_u8(DM_SPI_WR | DM_SPI_MWCMD, db->spi_sypc_buf); //'RD_LEN_ONE'

#if 1
      //memcpy(db->tmpTxPtr, txb, RD_LEN_ONE + blkLen);
        memcpy(db->spi_sypc_buf+1, txb, blkLen);
		xfer->tx_buf = db->spi_sypc_buf; //txb;
		xfer->rx_buf = NULL; //db->tmpRxPtr; //NULL; //tmpRxPtr; //trxb; ((( When DMA 'NULL' is not good~~~
#else
#endif

		xfer->len = RD_LEN_ONE + blkLen; // minus 1, so real all is 1024 * n
		//spi_message_init(&db->spi_dmsg1);
		//spi_message_add_tail(&db->spi_dxfer1, &db->spi_dmsg1);
		if(spi_sync(db->spidev, &db->spi_msg1)) //(INNODev_sync(db))
			printk("[dm95_spi]INNO txERR1: len= %d of %d, txbuf=0x%p,rxbuf=0x%p",blkLen,len,xfer->tx_buf,xfer->rx_buf); //return INNO_GENERAL_ERROR;

		//(2)	
		blkLen= remainder;
#if 1
        memcpy(db->spi_sypc_buf+1, &txb[CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1], remainder);
		xfer->tx_buf = db->spi_sypc_buf; //&txb[CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1]; // has been minus 1
		xfer->rx_buf = NULL; //NULL; //tmpRxPtr; //trxb; (((  'NULL' is not good~~~
#else
#endif

		xfer->len = RD_LEN_ONE + remainder; // when calc, it plus 1
		//spi_message_init(&db->spi_dmsg1);
		//spi_message_add_tail(&db->spi_dxfer1, &db->spi_dmsg1);
		if (spi_sync(db->spidev, &db->spi_msg1)) //(INNODev_sync(db))
			printk("[dm95_spi]INNO txERR2: len=%d of %d, txbuf=0x%p,rxbuf=0x%p",blkLen,len,xfer->tx_buf,xfer->rx_buf); //return INNO_GENERAL_ERROR;
	} else {
		wbuff_u8(DM_SPI_WR | DM_SPI_MWCMD, db->spi_sypc_buf);
		//spi_message_init(&db->spi_dmsg1);
#if 1
      //memcpy(db->tmpTxPtr, txb, RD_LEN_ONE + len);
        memcpy(db->spi_sypc_buf+1, txb, len);
		xfer->tx_buf = db->spi_sypc_buf; //txb;
		xfer->rx_buf = NULL; //NULL; //tmpRxPtr; //trxb; ((( again When DMA 'NULL' is not good~~~
#endif
		xfer->len = RD_LEN_ONE + len;
		//spi_message_add_tail(&db->spi_dxfer1, &db->spi_dmsg1);
		if (spi_sync(db->spidev, &db->spi_msg1)) //(INNODev_sync(db))
			printk("[dm95_spi]INNO ERROR: len=%d, txbuf=0x%p,rxbuf=0x%p",len,xfer->tx_buf,xfer->rx_buf); //return INNO_GENERAL_ERROR;
	}
#endif
}

void dread_1024_Limitation(board_info_t *db, u8 *trxb, int len)
{
	struct spi_transfer *xfer = &db->spi_xfer1;
	//struct spi_transfer *xfer;
	u8 txb[1];
	int const pkt_count = (len + 1) / CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	int const remainder = (len + 1) % CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	if((len + 1)>CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES){	
		int blkLen;
		wbuff_u8(DM_SPI_RD | DM_SPI_MRCMD, txb);
		//(1)
		blkLen= CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1;

	    printkr("dm9rx_EvenPar_OvLimit(%d ... ", blkLen);
	    printkr("txbf=%s,rxbf=0x%p)\n", "&txb[0]", db->spi_sypc_buf /*db->tmpRxPtr*/);
    
		//spi_message_init(&db->spi_dmsg1);
		//xfer= &db->spi_dxfer1;
        //memcpy(db->tmpTxPtr, txb, 2);
		//xfer->tx_buf = db->tmpTxPtr; //txb;
		xfer->tx_buf = txb;
		xfer->rx_buf = db->spi_sypc_buf; //[TxDatBuf].instead-rxb. //db->tmpRxPtr; //trxb;
		xfer->len = RD_LEN_ONE + (CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count) - 1;  // minus 1, so real all is 1024 * n
		//spi_message_add_tail(&db->spi_dxfer1, &db->spi_dmsg1);
		if (spi_sync(db->spidev, &db->spi_msg1)) //(INNODev_sync(db))
			printk("[dm95_spi]INNO1 ERROR: len=%d, txbuf=0x%p,rxbuf=0x%p",len,txb,trxb); //return INNO_GENERAL_ERROR;
        memcpy(trxb, db->spi_sypc_buf, RD_LEN_ONE + (CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count) - 1);
		//(2)	
		blkLen= remainder;
		printkr("dm9rx_EvenPar_OvRemainder(%d ... ", blkLen);
		printkr("txbf=%s,rxbf=0x%p)\n", "&txb[0]", db->spi_sypc_buf /*db->tmpRxPtr*/);

		//spi_message_init(&db->spi_dmsg1);
		//xfer= &db->spi_dxfer1;
      //memcpy(db->tmpTxPtr, txb, 2);
      //memcpy(db->tmpRxPtr, db->spi_sypc_buf, RD_LEN_ONE + remainder);
		xfer->tx_buf = txb; //NULL is also OK.. //db->tmpTxPtr; //txb;
		xfer->rx_buf = db->spi_sypc_buf; //db->TxDatBuf;
		xfer->len = RD_LEN_ONE + remainder; // when calc, it plus 1
		//spi_message_add_tail(&db->spi_dxfer1, &db->spi_dmsg1);
		if (spi_sync(db->spidev, &db->spi_msg1)) //(INNODev_sync(db))
			printk("[dm95_spi]INNO2 ERROR: len=%d, txbuf=0x%p,rxbuf=0x%p",len,txb,xfer->rx_buf); //return INNO_GENERAL_ERROR;

        memcpy(trxb + (CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count), &db->spi_sypc_buf[1], remainder);
	}
	else{
		printkr("dm9rx_smal_(%d ... ", len);
		printkr("txbf=%s,rxbf=0x%p)\n", "&txb[0]", db->spi_sypc_buf);

		wbuff_u8(DM_SPI_RD | DM_SPI_MRCMD, txb);
		//spi_message_init(&db->spi_dmsg1);
		//xfer= &db->spi_dxfer1;

#if 1
	     //wbuff_u8(DM_SPI_RD | DM_SPI_MRCMD, db->tmpTxPtr);
#else
         //memcpy(db->tmpTxPtr, txb, 2);
#endif

		xfer->tx_buf = txb; //db->tmpTxPtr; //txb;
		xfer->rx_buf = db->spi_sypc_buf; //trxb;
		xfer->len = RD_LEN_ONE + len;
		//spi_message_add_tail(&db->spi_dxfer1, &db->spi_dmsg1);
		if (spi_sync(db->spidev, &db->spi_msg1)) //(INNODev_sync(db))
			printk("[dm95_spi]INNO ERROR: len=%d, txbuf=0x%p,rxbuf=0x%p",len,txb,trxb); //return INNO_GENERAL_ERROR;

		printkr("dm9rx_smal_tx_cmd(%s) ... \n", "%txb[0]");

		memcpy(trxb, db->spi_sypc_buf, RD_LEN_ONE + len);                
		//dread_32_Limitation(db, trxb, len);
	}
} 

#if DM_CONF_APPSRC
static int Custom_SPI_Write(board_info_t *db, u8 *buff, unsigned len) 
{
#if DMA3_P2_TSEL_1024F
	dwrite_1024_Limitation(db, buff, NULL, len);
	return 1;
#elif DMA3_P2_TSEL_32F
    memcpy(&db->spi_sypc_buf[1], buff, len);
    dwrite_32_Limitation(db, db->spi_sypc_buf, NULL, len);
	return 1;
#elif DMA3_P2_TSEL_1F
	memcpy(&db->spi_sypc_buf[1], buff, len);
	dwrite_1_Limitation(db, db->spi_sypc_buf, NULL, len);
	return 1;
#else
	return 0;
#endif
}

static int Custom_SPI_Read(board_info_t *db, u8 *buff, unsigned len)
{	
#if DMA3_P2_RSEL_1024F
	dread_1024_Limitation(db, buff, (int)len);
	return 1;
#elif DMA3_P2_RSEL_32F
	dread_32_Limitation(db, buff, (int)len);
	return 1;
#elif DMA3_P2_RSEL_1F
	dread_1_Limitation(db, buff, (int)len);
	return 1;
#else
	return 0;
#endif
}
#endif

#endif
