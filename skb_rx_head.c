				     
#define RWREG1_START  0x0c00
#define RWREG1_END    0x3FFF
#define RWREG1_OVSTART  0x4000

struct dm9051_rxhdr0 { //old
	u8	RxPktReady;
	u8	RxStatus;
	__le16	RxLen;
} __packed;

struct spi_rxhdr { //new
	u8	padb;
	u8	spiwb;
	struct dm9051_rxhdr {
	  u8	RxPktReady;
	  u8	RxStatus;
	  __le16	RxLen;
	} rxhdr;
} __packed;          

/* Common cap calc usage */

void read_rwr(board_info_t *db, u16 *ptrwr)
{
#if DEF_SPIRW	
	*ptrwr= ior(db, 0x24); //v.s. 'DM9051_MRRL'
	*ptrwr |= (u16)ior(db, 0x25) << 8;  //v.s. 'DM9051_MRRH'
#endif	
}
void read_mrr(board_info_t *db, u16 *ptrmrr)
{
#if DEF_SPIRW	
	*ptrmrr= ior(db, DM9051_MRRL);
	*ptrmrr |= (u16)ior(db, DM9051_MRRH) << 8; 
#endif	
}

u16 dm9051_calc(u16 rwregs0, u16 rwregs1)
{
	u32 digiCalc;
	u32 digi, dotdigi;
	u16 calc;
	
	if (rwregs0>=rwregs1)
		digiCalc= rwregs0 - rwregs1;
	else
		digiCalc= 0x3400 + rwregs0 - rwregs1; //= 0x4000 - rwregs[1] + (rwregs[0] - 0xc00)
		
	digiCalc *= 100;
	digi= digiCalc / 0x3400;
	
	dotdigi= 0;
	digiCalc -= digi * 0x3400;
	if (digiCalc>=0x1a00) dotdigi= 5;
	
	calc= ((digi << 8) + dotdigi);
	return calc;
}

u16 dm9051_rx_cap(board_info_t *db)
{
	u16 rwregs[2];
	read_rwr(db, &rwregs[0]);
	read_mrr(db, &rwregs[1]);
	db->rwregs[0]= rwregs[0]; // save in 'rx_cap'
	db->rwregs[1]= rwregs[1];
	db->calc= dm9051_calc(rwregs[0], rwregs[1]);
	return db->calc;   
}

