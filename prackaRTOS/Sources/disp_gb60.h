// --------------------------------------------------------------------------------
// Hlavickovy soubor pro ovladac displeje dispgb60.asm
// --------------------------------------------------------------------------------


void dinit(void);                       // prototypy funkci pro obsluhu displeje
void dcls(void);
void douta(char znak);
void dtext(char* string);
void setcursor(char radek,char sloupec);
