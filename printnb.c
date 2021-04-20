

#define printnb(format, args...)	printnb_process(format, ##args)

//printnb.s //[nb: new buffer]
struct {
  int enab;
  int n;
  char bff[100];
} nb;

/* message print [0: disable, 1: enable] */
void printnb_init(int enab)
{
  nb.enab = enab;
  nb.n = 0; //printnb's initialization-reset.
}

void printnb_process(const char *format, ...)
{
  struct va_format vaf;
  va_list args;
  if (!nb.enab)
    return;
  
  va_start(args, format);
  vaf.fmt= format;
  vaf.va= &args;
  nb.n += sprintf(&nb.bff[nb.n], "%pV", &vaf); 
  va_end(args);
  
  if (nb.bff[nb.n -1]=='\n') {
    printk(nb.bff);
    nb.n = 0;
  }
}
//printnb.e
#define DISP_PER_LINE		16
#define DISP_HALF_LINE	8 
#define DISP_PER_MLINE	32
#define DISP_HALF_MLINE	16
void printnb_packet(u8 *mdat, int n) //u8 * //char *
{
	int i;
	 for (i=0; i<n; i++)
	 {
		 if (i && (!(i%DISP_PER_MLINE)) ) printnb("\n");
		 else if (i && (!(i%DISP_HALF_MLINE)) ) printnb(" ");
		 printnb(" %02x", mdat[i]);
	 }
	 printnb("\n");
}
