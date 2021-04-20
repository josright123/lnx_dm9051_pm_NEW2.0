/*
 *  INT 
 */

#ifdef DM_CONF_POLLALL_INTFLAG	
void int_en(struct net_device *dev)
{
    DM9051_int_en= 1;
    enable_irq(dev->irq); // V1.69.6p2 enable INT 
	//board_info_t *db = netdev_priv(dev);
    //if (db->nSCH_INT < 8)
	//	printk("[dm951_ int_en].e ------- enable_irq ( %d ) statis nSCH_INT= %d -------\n", dev->irq, db->nSCH_INT);
}
void int_dis(struct net_device *dev)
{
	disable_irq_nosync(dev->irq); // V1.69.6p2 disable INT 
	DM9051_int_en= 0;
}

#define S_DISP_BOOTING(endN)					((db->nSCH_INT < endN))
#define	S_DISP_DISABLE_IRQ(str)  if (S_DISP_BOOTING(DM_CONF_SPI_DBG_INT_NUM))  printk("[%s][-].disable_irq\n", str  /*, db->nSCH_INT*/ ); 
void IRQ_DISABLE(char *head, board_info_t *db)
{
//#ifdef DM_CONF_POLLALL_INTFLAG	
    if (DM9051_int_en){
		int_dis(db->ndev); //disable_irq_nosync(irq);DM9051._int_en= 0;
		
		//if ((db->nSCH_INT < DM_CONF_SPI_DBG_INT_NUM))  
			//printk("[%s][%d].disable_irq\n", head, db->nSCH_INT);
		S_DISP_DISABLE_IRQ(head)
			
		//in int_dis() DM9051_int_en= 0;
		//it like a structure with one field.DM9051_int_en is as "field.DM9051_int_enter_tobe_IMR_service_etc"
		//that every thing in the following (rx_work,dm9051_rx_work) will depend on "field.DM9051_int_enter_tobe_IMR_service_etc"
		//and finally clear it -- the "field.DM9051_int_enter_tobe_IMR_service_etc".
		//  int_reg_stop(db); .To DO FUNC.
		db->has_do_disable = true;
		//return true;
    }
//#endif
	else{
		db->has_do_disable = false;
		//return false;
	}
}

#define S_DISP_IRQ(staRANG, endN)	((db->nSCH_INT < staRANG) || (db->nSCH_INT == endN)) //[0~7, endN]
#define E_DISP_IRQ(staRANG, endN)	((db->nSCH_INT < (staRANG+1)) || (db->nSCH_INT == endN+1)) //[1~8, 25]
#define S_DISP_FIRST if (S_DISP_BOOTING(DM_CONF_SPI_DBG_INT_NUM)) {  \
	printk("\n"); \
	if (db->nSCH_INT==0) \
		printk("[dm9]\n"); \
}
#define S_DISP_FUNC	if (S_DISP_IRQ(DM9_DBG_INT_ONOFF_COUNT, DM_CONF_SPI_DBG_INT_NUM-1)) printk("[dm951_irq][-].in\n" /*, db->nSCH_INT*/ );  //(8,24) ,(5,10)
#define E_DISP_FUNC	if (E_DISP_IRQ(DM9_DBG_INT_ONOFF_COUNT ,DM_CONF_SPI_DBG_INT_NUM-1)) printk("[dm951_irq][%d].out\n", db->nSCH_INT); //(8,24) ,(5,10)
#define HAS_DISABLE	db->has_do_disable
//************************************************************//
/* Not used interrupt-related function: */
/* This function make it less efficient for performance */
/* Since the ISR not RX direct, but use a schedule-work */
/* So that polling is better in using its poll schedule-work */
#ifdef DM_CONF_THREAD_IRQ

static void dm9051_rx_work_proc(board_info_t *db);

static irqreturn_t dm9051_rx_work_irq(int irq, void *pw)
{
	board_info_t *db = pw;
	db->nSCH_INT++;
  	S_DISP_FIRST
  	S_DISP_FUNC
	IRQ_DISABLE("dm951_irq", db);
	if (HAS_DISABLE) db->nSCH_INT_NUm_A++;
	db->nSCH_INT_NUm++;
	
	dm9051_rx_work_proc(db); //proc
	
    E_DISP_FUNC
	return IRQ_HANDLED;
}
#else
static irqreturn_t dm951_irq(int irq, void *pw)
{
	board_info_t *db = pw;
	
	//if ((db->nSCH_INT < DM_CONF_SPI_DBG_INT_NUM))  // || (db->nSCH_INT == 24)
		//printk("\n");
	//if ((db->nSCH_INT < 8) || (db->nSCH_INT == 24)) //[0~7, 24]
	//{
		//if (db->nSCH_INT==0) {
			//printk("[dm9]\n");
		//}
		//printk("[dm951_irq][%d].in\n", db->nSCH_INT);
  	//}
  	S_DISP_FIRST
  	S_DISP_FUNC
  	
    //.DM9051_int_flag = 1;	

	//.#ifdef DM_CONF_POLLALL_INTFLAG	
    //.if (DM9051_int_en){
	//.	db->nSCH_INT_NUm_A++;
	//.	int_dis(db->ndev); //disable_irq_nosync(irq);DM9051._int_en= 0;
		//in int_dis() DM9051_int_en= 0;
		//it like a structure with one field.DM9051_int_en is as "field.DM9051_int_enter_tobe_IMR_service_etc"
		//that every thing in the following (rx_work,dm9051_rx_work) will depend on "field.DM9051_int_enter_tobe_IMR_service_etc"
		//and finally clear it -- the "field.DM9051_int_enter_tobe_IMR_service_etc".
		//  int_reg_stop(db); .To DO FUNC.
    //.}
	//.#endif
	//=
	//if (IRQ_DISABLE("dm951_irq", db))
		//db->nSCH_INT_NUm_A++;
	IRQ_DISABLE("dm951_irq", db);
	if (HAS_DISABLE) db->nSCH_INT_NUm_A++;
  
	db->nSCH_INT_NUm++;
    dm9051_INTPschedule_isr(db, R_SCH_INT); //schedule_work(&db->rx._work); //new 'dm9051_INTPschedule_isr'
    
    E_DISP_FUNC
	//if (db->nSCH_INT && ((db->nSCH_INT <= 8) || (db->nSCH_INT == 25))) //[1~8, 25]
		//printk("[dm951_irq][%d].out\n", db->nSCH_INT);
		
	return IRQ_HANDLED;
}
#endif

#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
u8 read_DTS_IrqType(struct device_node *nc)
{
	u8 val;
	int rc;
	u32 value;
		val = IIRQ_TYPE_NONE;
		if (nc) {
			rc = of_property_read_u32_index(nc, "interrupts", 1, &value); // read to DTS index
			if (!rc) {
				printk("[ *dm9051 dts ] %s interrupts irq-type[= %d]\n", nc->full_name, value);
				val = (int) value;
			}
		}
		else
			printk("[ *dm9051 dts ] interrupts irq-type<= %d (default)>\n", val); //=value
			
		if (val) {
			if (val & IIRQ_TYPE_EDGE_RISING)
				printk("[ *dm9051 dts ] interrupts IRQ Trigger Type[= %d] (EDGE RISING)\n", val);
			if (val & IIRQ_TYPE_EDGE_FALLING)
				printk("[ *dm9051 dts ] interrupts IRQ Trigger Type[= %d] (EDGE FALLING)\n", val);
			if (val & IIRQ_TYPE_LEVEL_HIGH)
				printk("[ *dm9051 dys ] interrupts IRQ Trigger Type[= %d] (LEVEL HIGH)\n", val);
			if (val & IIRQ_TYPE_LEVEL_LOW)
				printk("[ *dm9051 dts ] interrupts IRQ Trigger Type[= %d] (LEVEL LOW)\n", val);
		}
		else
			printk("[ *dm9051 dts ] interrupts IRQ Trigger Type{= %d} (TRIGGER NONE)\n", val);
		return val;
}
void Set_IrqType(board_info_t *db, struct device_node *nc)
{
	db->irq_type = read_DTS_IrqType(nc);
}	
u8 Get_TrqType(board_info_t *db)
{
	return db->irq_type;
}
#endif

// return: 0, ATTRI_OK 
// return: 1, ATTRI_ERR 
int int_get_attribute(struct spi_device *spi, struct net_device *ndev)
{
	int ret = 0;
#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
	board_info_t *db = netdev_priv(ndev);
#else
	char *Str[5][2]= {
		{"IRQF_TRIGGER_NONE", "LEVEL_FLOAT"},
		{"IRQF_TRIGGER_RISING", "LEVEL_HIGH"},
		{"IRQF_TRIGGER_FALLING", "LEVEL_LOW"},
		{"IRQF_TRIGGER_HIGH", "LEVEL_HIGH"},  // domain-field
		{"IRQF_TRIGGER_LOW", "LEVEL_LOW"},  // domain-field
	};  //(DRV_IRQF_TRIGGER == IRQF_TRIGGER_LOW)? "(LEVEL LOW)": "(LEVEL HIGH)"
	int str_idx;
	u16 bbit= 0x01;
	str_idx= 0;
	while(bbit<=0x08) {
	 str_idx++;
	 if (DRV_IRQF_TRIGGER & bbit) {
		break;
	 }
	 bbit<<= 1;
   }
	if (bbit > 0x08) 
		str_idx= 0;
#endif	
	printk("dm9.[int_get_attri].s\n");
//#ifdef DM_CONF_POLLALL_INTFLAG
	//---------------------------------------------------------
	//[Raspberry Pi 2, GPIO_17 (pin 11) on Pi connection slot.]
	//[Raspberry Pi 2, GPIO_26 (pin 37) on Pi connection slot.]
	//---------------------------------------------------------
//#endif

//#ifdef DM_CONF_POLLALL_INTFLAG
#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
	//Jerry add. and. if. spi->irq = dm9051_get_irq_num();
	do {
		int rc;
		struct device_node *nc;
		u32 value;
		int gpio_int_pin; //Get gpio pin from DTS.
		/* DTS, this GPIO-INT-PIN setting is in the dts file, not in the driver ! */
		gpio_int_pin = 0; // 0 means null (empty) //DM_CONF_INTERRUPT_IRQ; //=value
		nc = of_find_compatible_node(NULL, NULL, DM_DM_CONF_DTS_COMPATIBLE_USAGE);
		//read-interrupt-pin-num
		if (nc) {
			//rc = of_property_read_u32(nc, "interrupts", &value); // read to DTS 
			rc = of_property_read_u32_index(nc, "interrupts", 0, &value); // read to DTS index
			if (!rc) {
				printk("[ *dm9051 dts ] %s interrupts gpio-pin[= %d]\n", nc->full_name, value);
				gpio_int_pin = (int) value;
			}
		}
		else
			printk("[ *dm9051 dts ] interrupts gpio-pin= %d (read fail)>\n", gpio_int_pin); //=value
			
		printk("[ *dm9051 spi_device * ] %s value[= %d]\n", "spi->irq", spi->irq);
		
		//read-interrupt-trigger-irq-type	
		Set_IrqType(db, nc); //Or only call: read_DTS_IrqType(nc), is most pure simple.

		//dm9051 gpio_request.
		/* DTS, this request is of the dts's domination, not in the driver ! 	
		if (gpio_request(gpio_int_pin, GPIO_ANY_GPIO_DESC)) {
			printk("--------------------------------------------------------\n");
			printk("ERROR! dm9051 Mapped gpio_to_irq() IRQ no : %d\n", spi->irq);
			printk("--------------------------------------------------------\n");
			printk("dm951 gpio_request ERROR! pin %d, desc %s\n", DM_CONF_INTERRUPT_IRQ, GPIO_ANY_GPIO_DESC);
			printk("--------------------------------------------------------\n");
			goto err_irq;
		}*/ 
	} while(0);
#else
	printk("[ *dm9051 CONST ] dm9051 gpio_request: pin %d, desc %s\n", DM_CONF_INTERRUPT_IRQ, GPIO_ANY_GPIO_DESC);
	if (gpio_request(DM_CONF_INTERRUPT_IRQ, GPIO_ANY_GPIO_DESC)) {
		printk("---------------------------------------------\n");
		printk("Error! dm9051 Mapped gpio_to_irq() IRQ no : %d\n", spi->irq);
		printk("---------------------------------------------\n");
		printk("dm9051 gpio_request Error! pin %d, desc %s\n", DM_CONF_INTERRUPT_IRQ, GPIO_ANY_GPIO_DESC);
		printk("---------------------------------------------\n");
		ret = 1; 
		goto err_attri;
	}
	printk("[ *dm9051 CONST ] interrupts gpio-pin= %d\n", DM_CONF_INTERRUPT_IRQ);
	gpio_direction_input(DM_CONF_INTERRUPT_IRQ);
	spi->irq = gpio_to_irq(DM_CONF_INTERRUPT_IRQ);  //exp: enum gpio_to_irq( 17) = 187
	if (spi->irq <= 0) { // jj:enum
		printk("dm9051 failed to get irq_no, %d\n", spi->irq);
		ret = 1;
		goto err_attri;
	}
	printk("[ *dm9051 spi_device * ] gpio_to_irq()= %d\n", spi->irq);	
	
	printk("[ *dm9051 CONST ] interrupts IRQ Trigger Type[=%s] %s\n", 
		Str[str_idx][0],  Str[str_idx][1]); 
err_attri:		
#endif
//#endif
	printk("dm9.[int_get_attri].e\n");
	return ret;
}

void int_get_begin(struct spi_device *spi, struct net_device *ndev)   
{
	board_info_t *db = netdev_priv(ndev);
	int ret; // = 0;
#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE	
	#ifdef DTS_AUTO_TRIGGER 
	#else
	unsigned long val_trigger;
	u8 val_type;
	#endif
#endif	
	
	printk("dm9.[int_get_begin].s\n");

//#ifdef DM_CONF_POLLALL_INTFLAG

	//"int_get_attribute()"

	ndev->irq = spi->irq;	
	//ndev->if_port = IF_PORT_100BASET;
				
#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE	
	printk("dm9051 Mapped DTS's IRQ no : %d\n", spi->irq);
	
	//db->req_irq_flags = IRQF_TRIGGER_NONE;
	#ifdef DTS_AUTO_TRIGGER //(if 1)
	printk("dm9051 request irq for DTS's IRQ Trigger Type with IRQF_TRIGGER_NONE : %d (TRIGGER NONE)\n", IRQF_TRIGGER_NONE);
	printk("Hence registering with IRQF_TRIGGER_NONE does NOT modify the existing configuration of the IRQ and IRQ no.\n");
	#ifdef DM_CONF_THREAD_IRQ
	ret = request_threaded_irq(spi->irq, NULL, dm9051_rx_work_irq,
				IRQF_TRIGGER_NONE, ndev->name, db);
	#else
	ret = request_irq(spi->irq, dm951_irq, IRQF_TRIGGER_NONE, /*db->req_irq_flags*/
				ndev->name, db);
	#endif
	#else			
	//if (Get_TrqType(db) & (IIRQ_TYPE_EDGE_RISING | IIRQ_TYPE_LEVEL_HIGH)) {
		//printk("dm9051 request irq for DTS's IRQ Trigger Type with IRQF_TRIGGER_HIGH : 0x%d (IRQF_TRIGGER_HIGH)\n", IRQF_TRIGGER_HIGH);
		//ret = request_irq(spi->irq, dm951_irq, IRQF_TRIGGER_HIGH,
		//		ndev->name, db);
	//} else {
		//printk("dm9051 request irq for DTS's IRQ Trigger Type with IRQF_TRIGGER_LOW : 0x%d (IRQF_TRIGGER_LOW)\n", IRQF_TRIGGER_LOW);
		//ret = request_irq(spi->irq, dm951_irq, IRQF_TRIGGER_LOW,
		//		ndev->name, db);
	//}
	val_type = Get_TrqType(db);
	if (val_type & IIRQ_TYPE_EDGE_RISING) {
		printk("dm9051 do request irq, IRQ Trigger Type[= %d] (EDGE RISING)\n", val_type);
		val_trigger = IRQF_TRIGGER_RISING;
	} else if (val_type & IIRQ_TYPE_EDGE_FALLING) {
		printk("dm9051 do request irq, IRQ Trigger Type[= %d] (EDGE FALLING)\n", val_type);
		val_trigger = IRQF_TRIGGER_FALLING;
	} else if (val_type & IIRQ_TYPE_LEVEL_HIGH){
		printk("dm9051 do request irq, IRQ Trigger Type[= %d] (LEVEL HIGH)\n", val_type);
		val_trigger = IRQF_TRIGGER_HIGH;
	} else if (val_type & IIRQ_TYPE_LEVEL_LOW) {
		printk("dm9051 do request irq, IRQ Trigger Type[= %d] (LEVEL LOW)\n", val_type);
		val_trigger = IRQF_TRIGGER_LOW;
	} else {
		printk("dm9051 do request irq, IRQ Trigger Type[= %d] Unknow to (LEVEL LOW)\n", val_type);
		val_trigger = IRQF_TRIGGER_LOW; // or 'IRQF_TRIGGER_NONE'
	}
	#ifdef DM_CONF_THREAD_IRQ
	ret = request_threaded_irq(spi->irq, NULL, dm9051_rx_work_irq,
			val_trigger, ndev->name, db);
	#else
	ret = request_irq(spi->irq, dm951_irq, val_trigger,
			ndev->name, db);
	#endif
	#endif
#else		
	printk("(dm9051 Mapped gpio_to_irq() IRQ no : %d)\n", spi->irq);
	#ifdef DM_CONF_THREAD_IRQ
	ret = request_threaded_irq(spi->irq, NULL, dm9051_rx_work_irq,
				DRV_IRQF_TRIGGER | IRQF_ONESHOT,
				ndev->name, db);
	#else
	ret = request_irq(spi->irq, dm951_irq, DRV_IRQF_TRIGGER | IRQF_ONESHOT, //DRV_IRQF_TRIGGER for IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW
				ndev->name, db); //or IRQF_TRIGGER_NONE, //or IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING 
	#endif
#endif			

	disable_irq_nosync(/*spi->irq*/ndev->irq);//int_d.i.s
	DM9051_int_en= 0;//int_d.i.s
	printk("[DBG] dm9051 request irq then immediately, disable_irq_nosync()\n");
			
	if (!ret)
		printk("[DBG] dm9051 request irq (%d), ret= %d (must be zero to succeed)\n", spi->irq, ret);
	else
		printk("[DBG] dm9051 request irq (%d), ret= %d (must be zero to succeed, BUT not) ERROR!\n", spi->irq, ret);
	if (ret < 0) {
		printk("dm9051 failed to (get irq) ERROR!\n");
		goto err_irq;
	}
//#endif

	printk("%s: dm9051spi at isNO_IRQ %d MAC: %pM\n", // (%s)
		   ndev->name,
		   ndev->irq,
		   ndev->dev_addr);
		   
//#ifdef DM_CONF_POLLALL_INTFLAG
err_irq:
//#endif

	//#ifdef DM_CONF_POLLALL_INTFLAG
	//free_irq(spi->irq, db); 
	//#endif
	
	printk("dm9.[int_get_begin].e\n");
	
	return;
}

void int_end(struct spi_device *spi, board_info_t *db)    
{
  //#ifdef DM_CONF_POLLALL_INTFLAG
  
	#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
	/* DTS, this free is of the dts's domination, not in the driver ! */ //Jerry add
	#else
	printk("[dm9051 gpio_free: pin %d (desc %s)]\n", DM_CONF_INTERRUPT_IRQ, GPIO_ANY_GPIO_DESC);
	gpio_free(DM_CONF_INTERRUPT_IRQ);
	#endif
	
  //#endif
  
  //#ifdef DM_CONF_POLLALL_INTFLAG
	printk("[dm9051 free_irq: irq no %d]\n", spi->irq);
	free_irq(spi->irq, db);
  //#endif
} 
#endif
