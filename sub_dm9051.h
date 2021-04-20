// --- struct declaration ---
// ---  const definition  ---

#define WR_ISR_ENDOF_RXWORK_ONLY //JJ_20190813
#ifdef MORE_DM9051_INT_BACK_TO_STANDBY 
// (TO BE TESTED) #undef WR_ISR_ENDOF_RXWORK_ONLY  //JJ_20191206
#endif

#define IIRQ_TYPE_NONE			0
#define IIRQ_TYPE_EDGE_RISING	1
#define IIRQ_TYPE_EDGE_FALLING	2
#define IIRQ_TYPE_LEVEL_HIGH	4
#define IIRQ_TYPE_LEVEL_LOW		8

//[SPI_BUF_LEN]
#define SPI_SYNC_TRANSFER_BUF_LEN (4 + DM9051_PKT_MAX)
//[CUSTOM_BUF_LEN]
#define CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES (0x400)

#define DS_NUL							0
#define DS_POLL							1
#define DS_IDLE							2
#define CCS_NUL							0
//#define CCS_PROBE						1

#define	R_SCH_INIT		1
#define R_SCH_XMIT		2
#define R_SCH_INT		3
#define R_SCH_INFINI	4
#define R_SCH_INT_GLUE	5  // vs.R_SCH_.INT
//#define R_SCH_LINK	6
#define R_SCH_PHYPOLL	6  // extended

/*void dm_schedule_phy(board_info_t *db);*/
/*static void dm9051_INTPschedule_isr(board_info_t *db, int sch_cause);*/
/*static void dm9051_fifo_reset(u8 state, u8 *hstr, board_info_t *db);*/
/*int dm9051_fifo_ERRO(int ana_test, u8 rxbyte, board_info_t *db);*/
/*static u16 check_cntStop(board_info_t *db);*/

u8 DM9051_fifo_reset_flg = 0;
#ifdef DM_CONF_POLLALL_INTFLAG // no-'DRV_POLL_1'
//u8 DM9051_int_flag = 0; //INT
u8 DM9051_int_en = 1;
u32 DM9051_int_token= 0;
#else
//u8 DM9051_int_en_OF_poll = 0; //NO used!
#endif

/* 3p6s.s */
asmlinkage __visible int printkr(const char *fmt, ...){
  return 0; 
}
EXPORT_SYMBOL(printkr);
/* 3p6s.e */
 
