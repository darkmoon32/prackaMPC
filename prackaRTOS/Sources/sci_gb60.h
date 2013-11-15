// --------------------------------------------------------------------------------
// Hlavickovy soubor pro ovladac serioveho rozhrani sci_gb60.asm
// --------------------------------------------------------------------------------

#define BD4800   4
#define BD9600   9
#define BD19200  19
#define BD38400  38
#define BD57600  57
#define BD115200 115

char sci1_init(char);                       // prototypy funkci pro obsluhu displeje
char sci1_in(void);
void sci1_out(char);
void sci1_str_out(char*);