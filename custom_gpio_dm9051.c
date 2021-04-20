
#ifdef QCOM_CONF_BOARD_YES
static irqreturn_t realtek_plug_irq_l(int irq, void *dev_id)
{
/*davicom add begin*/	
	
/*davicom add end*/		
	return IRQ_HANDLED;
}

static irqreturn_t realtek_plug_irq_p(int irq, void *dev_id)
{
/*davicom add begin*/	
	
/*davicom add end*/		
	return IRQ_HANDLED;
}
#endif

void Custom_Board_Init(struct spi_device *spi)
{
#ifdef QCOM_CONF_BOARD_YES
	struct pinctrl *phandle=NULL;
   // struct device_node * np=NULL;
	unsigned int gpio_rst;
	unsigned int gpio_power; 
	unsigned int gpio_realtek_irq_l; 
	unsigned int gpio_realtek_irq_p; 
	unsigned int realtek_irq_no_l= 0;
	unsigned int realtek_irq_no_p= 0;
	////gary add interrupt for realtek begin 
	
		//gpio_rst = of_get_named_gpio(spi->dev.of_node, "reset-gpio-dm9051", 0);
   //printk("[ *dm9051 probe of_get_named_gpio gpio_rst is %d\n",gpio_rst);
		//gpio_power= of_get_named_gpio(spi->dev.of_node, "en-vdd-lan", 0);
	// printk("[ *dm9051 probe of_get_named_gpio gpio_power is %d\n",gpio_power);	
		//gpio_power= of_get_named_gpio(spi->dev.of_node, "reset-gpio-rtl8305", 0);
	// printk("[ *dm9051 probe of_get_named_gpio 8305reset is %d\n",gpio_power);	
	gpio_realtek_irq_l= of_get_named_gpio(spi->dev.of_node, "8305-irq1-gpio ", 0);
	gpio_realtek_irq_p= of_get_named_gpio(spi->dev.of_node, "8305-irq2-gpio ", 0);
	if (gpio_is_valid(gpio_realtek_irq_l)) {			
			ret = gpio_request(gpio_realtek_irq_l,"realtek_plug1_irq");	
			if (ret) {
			printk("[ *dm9051 realtek irq1 request failed \n");  }	
			realtek_irq_no_l = gpio_to_irq(gpio_realtek_irq_l);
			if (gpio_realtek_irq_l) {
			ret = request_irq(realtek_irq_no_l, realtek_plug_irq_l, IRQF_TRIGGER_RISING, "realtekirql", db->spidev);}
			if (ret) {
			printk("[ *dm9051 realtek irq1 request_irq failed \n"); }
		}

	if (gpio_is_valid(gpio_realtek_irq_p)) {			
			ret = gpio_request(gpio_realtek_irq_p,"realtek_plugp_irq");	
			if (ret) {
			printk("[ *dm9051 realtek irqp request failed \n");  }	
			realtek_irq_no_p = gpio_to_irq(gpio_realtek_irq_p);
			if (gpio_realtek_irq_p) {
			ret = request_irq(realtek_irq_no_p, realtek_plug_irq_p, IRQF_TRIGGER_RISING, "realtekirq2", db->spidev);}
			if (ret) {
			printk("[ *dm9051 realtek irq2 request_irq failed \n"); }
		}		
	//////gary add interrupt for realtek end 
		phandle=devm_pinctrl_get(spi);
		if(IS_ERR_OR_NULL(phandle))
			  printk("[ *dm9051 probe devm_pinctrl_get failed\n");
		  else
			 printk("[ *dm9051 probe devm_pinctrl_get sucess\n");  
		struct pinctrl_state *turnon_reset=pinctrl_lookup_state(phandle,"dm9051_active");
		struct pinctrl_state *turnoff_reset=pinctrl_lookup_state(phandle,"dm9051_sleep");
		struct pinctrl_state *turnon_power=pinctrl_lookup_state(phandle,"lan_active");
		struct pinctrl_state *turnon_reset_8305=pinctrl_lookup_state(phandle,"rtl8305_active");
		if(IS_ERR_OR_NULL(turnon_reset))
				printk("[ *dm9051 probe pinctrl_lookup_state reset failed\n");
			else
				printk("[ *dm9051 probe pinctrl_lookup_state reset sucess\n");
			
			if(IS_ERR_OR_NULL(turnon_power))
				printk("[ *dm9051 probe pinctrl_lookup_state power failed\n");
			else
				printk("[ *dm9051 probe pinctrl_lookup_state power sucess\n");
			pinctrl_select_state(phandle,turnon_power);
			//pinctrl_select_state(phandle,turnon_reset_8305);
			 //msleep(100);
			 // pinctrl_select_state(phandle,turnon_reset);
		 msleep(100);

	 // pinctrl_select_state(phandle,turnoff_reset);
		msleep(10);
	  
	//if (!dts_pin->gpio_rst)
			  //printk("[ *dm9051 READ-MTK dts WARN ] {%s} is found in the dts data-grouping-set\n", "gpio-rst-dm9051");
	//if (!dts_pin->gpio_rst_rtl)
			 //printk("[ *dm9051 READ-MTK dts WARN ] {%s} is found in the dts data-grouping-set00\n", "gpio-rst-rtl8305");
	//dts_pin->gpio_power = of_get_named_gpio(spi->dev.of_node, "en-vdd-lan ", 0);
		// if (!dts_pin->gpio_power)
			 //printk("[ *dm9051 READ-MTK dts WARN ] {%s} is not found in the dts data-grouping-set\n", "gpio-power");
//yangguangfu add
#endif
}

#ifdef QCOM_CONF_BOARD_YES

#define ETH_NODE_NAME "davicom,dm9051"

struct pin_ctl{
	unsigned int gpio_power;
	//unsigned int gpio_SPI_CS;
	unsigned int gpio_rst;
unsigned int gpio_rst_rtl;		
};

static struct pin_ctl *dts_pin;

static int ctl_pwr_gpio(void) {  //xx_gpio(int power, int power_en ,int cs, int cs_en)
	int ret = 0;
	//if(power == POWER){
		if(/*power_en &&*/ dts_pin->gpio_power != 0){
			ret = gpio_request(dts_pin->gpio_power,"power");
			if (ret) {
				printk("error: dm9051 gpio_request\n");
				return -1;
			}
			ret = gpio_direction_output(dts_pin->gpio_power, 1);
			if (ret) {
				printk("error %s : dm9051 reset gpio_direction_output failed\n",__FILE__);
				ret = -1;
			}
			mdelay(5);
		}//else if((!power_en) && dts_pin->gpio_power != 0){
			//ret = gpio_request(dts_pin->gpio_power,"power");
			//if (ret) {
			//	printk("error: dm9051 gpio_request\n");
			//	return -1;
			//}
			//ret = gpio_direction_output(dts_pin->gpio_power, 0);
			//if (ret) {
			//	printk("error %s : dm9051 reset gpio_direction_output failed\n",__FILE__);
			//	ret = -1;
			//}
			//mdelay(5);
		//}
	//}

	return 0;
} 

static int ctl_rst_gpio(void)
{
	int ret;
	if(dts_pin->gpio_rst != 0){
	  ret = gpio_request(dts_pin->gpio_rst,"spi_rst");
	  ret = gpio_direction_output(dts_pin->gpio_rst, 0);
	  msleep(100);
	  ret = gpio_direction_output(dts_pin->gpio_rst, 1);
	  msleep(10);		
	}
	  msleep(100);
	if(dts_pin->gpio_rst_rtl != 0){
	  ret = gpio_request(dts_pin->gpio_rst_rtl,"spi_rst_rtl");
	  ret = gpio_direction_output(dts_pin->gpio_rst_rtl, 0);
	  msleep(100);
	  ret = gpio_direction_output(dts_pin->gpio_rst_rtl, 1);
	  msleep(10);		
	}
	return ret;
}

int SPI_GPIO_Set(int enable)
{
	 struct device_node *node;
		printk("SPI_GPIO_Set\n");
	 node = of_find_compatible_node(NULL, NULL, ETH_NODE_NAME);
	if (!node) {
		printk("[ *dm9051 READ-quacom dts WARN ] {%s} is not found in the dts data-grouping-set\n", ETH_NODE_NAME);
		return -1;
	}
	 
     dts_pin->gpio_power = of_get_named_gpio(node, "en-vdd-lan ", 0);
     dts_pin->gpio_rst = of_get_named_gpio(node, "reset-gpio-dm9051", 0);
	 dts_pin->gpio_rst_rtl = of_get_named_gpio(node, "reset-gpio-rtl8305", 0);
			  
	if (!dts_pin->gpio_power)
			  printk("[ *dm9051 READ-MTK dts WARN ] {%s} is not found in the dts data-grouping-set\n", "gpio-power");
	if (!dts_pin->gpio_rst)
			  printk("[ *dm9051 READ-MTK dts WARN ] {%s} is found in the dts data-grouping-set\n", "gpio-rst-dm9051");
	if (!dts_pin->gpio_rst_rtl)
			  printk("[ *dm9051 READ-MTK dts WARN ] {%s} is found in the dts data-grouping-set00\n", "gpio-rst-rtl8305");
			  
	if (dts_pin->gpio_power)
	   printk("[ *dm9051 READ-MTK dts INFO ] {%s} is not found in the dts, Do ctl_pwr_gpio()\n", "gpio-power");
	ctl_pwr_gpio(); /*ret =*/  //xx_gpio(POWER,HIGH,CS,HIGH);
	if (dts_pin->gpio_rst)
		printk("[ *dm9051 READ-MTK dts INFO ] {%s} is found in the dts, Do ctl_rst_qpio()\n", "gpio-rst-dm9051");
	ctl_rst_gpio(); /*ret =*/ 
	return 0;
}

static
void SPI_GPIO_SetupPwrOn(struct board_info *db)
{
	 dts_pin = kmalloc(sizeof(struct pin_ctl), GFP_KERNEL);
	 if(NULL == dts_pin){
			  printk("error : dm9051 malloc pin_ctl failed\n");
			  return;
	 }
	SPI_GPIO_Set(1);
//#ifdef DM_CONF_INTERRUPT	
//	mt_dm9051_irq_no(NULL);
//#endif 
}
#endif //QCOM_CONF_BOARD_YES
