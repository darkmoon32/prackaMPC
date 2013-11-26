#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

#include "main_asm.h" /* interface to the assembly module */
#include <stdio.h>
#include "gpio.h"
#include "disp_gb60.h"
#include "sci_gb60.h"
#include "rtmon_08.h"
#include "driver.h"

#define Pin0               1
#define Pin1               2
#define Pin2               4
#define Pin3               8
#define Pin4               16
#define Pin5               32
#define Pin6               64
#define Pin7               128
#define vystupMode         0 
#define vstupMode          1


IDPROC *init, *pSCI, *pKeyboard, *pProgram, *pCas, *pMenu;
IDQUEUE* s1;

void sci( void );
void keyboard( void );
void program( void );
void casPracky( void );
void menu( void );
void programSequence(void);
void tocBubnem(int doba);
void predpirka(void);
void hlavniPrani(void);
void machani(void);
void zdimani(void);
void dokonciCinnost(void);
void ukonci(void);
void intToTime(unsigned long time, char *out);

unsigned long cas = 123,spozdeni = 0, zacatekPrani = 0,celkovyCas = 0;
char buffer[32];
int stav = 0, initState = 0, prog = 0, kurzor = -1, uroven = 0, pozice = 0;
int stats[] = {0,0,0,0,0};
int programy[] = {0b01001110,0b10001111,0b00010010,0b00101010,0b00101011};
int factoryNum = 1111;
char *factoryDate = "1.1.1970";
//prog význam bitù
//1 - odložený start
//2 - pøedpírka
//3 - hlavní praní
//4 - máchání
//5 - ždímání
//6-9 požadovaná teplota hl praní
//char menuText[][20] = {"Start","Vyber programu",","Diag","Cas","Spozdeni","Stop"};

void main(void) {
  unsigned long t = 0;
  EnableInterrupts; /* enable interrupts */
  /* include your code here */


  asm_main(); /* call the assembly function */
#if 1
  //sci1_init(BD9600);  
  dinit();
  dcls();
  dtext("Pracka");
#endif
  gpio_hard_init();
  gpio_button_init(0xf0);
  gpio_led_init(0xf);
  gpio_led_off(0xf);
  
  ATD1C = 0xC4;   	// zapnuti prevodniku, 8 bit vysledek    
  ATD1PE = 1;     	// pin PTB0 prepneme do rezimu 			  	// vstupu A/D prevodniku  
  PTADD_PTADD5 =0;      	// PTA4 vstupni rezim
  PTAPE_PTAPE5 = 1;	// pull-up pro PTA4 zapnut
  
  rtm_init(&init);
  rtm_create_p("proc2", 100, sci, 0x100, &pSCI );	 
  rtm_create_p("proc3", 2, keyboard, 100, &pKeyboard ); 
  rtm_create_p("proc5", 200, program, 100, &pProgram ); 
  rtm_create_p("proc6", 1, casPracky, 32, &pCas ); 
  rtm_create_p("proc7", 254, menu, 100, &pMenu );  
  rtm_create_q("no", sizeof(int), 1, &s1);  

  
  rtm_start_p(pCas, 0, 20);
  rtm_start_p(pKeyboard, 0, 3);
  rtm_start_p(pMenu,0,0);
  rtm_delay_p(init, 0);

  dcls();
  setcursor(1,1);  
  dtext("Nyni muzete");
  setcursor(2,1);
  dtext("pracku vypnout");          

  for(;;) {
    __RESET_WATCHDOG(); /* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}

void casPracky( void )
{
    cas++;
    rtm_stop_p(pCas);
}

void menu( void ) {
long t = 0xffff;
  while(1){
  t = 0xffff;
    __RESET_WATCHDOG();
    ATD1SC = 0;                                      // spusteni prevodu
    while(ATD1SC_CCF == 0 || t-- > 0)
        break;
    if(ATD1R < 10){//potenctiometr je na 0, rezervováno pro položku zpìt nebo konec
      if(kurzor != 0)
      {
        dcls();
        if(uroven == 0)        
            dtext("Konec");
        else
            dtext("Zpet");
        kurzor = 0;
      }
    } else if(uroven == 0 && ATD1R <= 204){//hlavní nabídka a výbìr programu
      if(kurzor != 1){
        dcls();
        dtext("Vyber programu");
        kurzor = 1;
      }
    } else if(uroven == 0 && ATD1R <= 408){
        if(kurzor != 2){
            dcls();
            dtext("Odlozeny start");
            kurzor = 2;
        }
    } else if(uroven == 0 && ATD1R <= 612){
        if(kurzor != 3){
            dcls();
            dtext("Cas");
            kurzor = 3;
        }
    } else if(uroven == 0 && ATD1R <= 816){
        if(kurzor != 4){
            dcls();
            dtext("Diagnostika");
            kurzor = 4;
        }
    } else if(uroven == 0 && ATD1R <= 1023){
        if(kurzor != 5)
        {   dcls();
            dtext("Start");
            kurzor = 5;
        }
    } else if(uroven == 1 && ATD1R <= 204){//podmenu pro výber programu
      if(kurzor != 1){
        dcls();
        dtext("Program 1");
        kurzor = 1;
      }
    } else if(uroven == 1 && ATD1R <= 408){
        if(kurzor != 2){
            dcls();
            dtext("Program 2");
            kurzor = 2;
        }
    } else if(uroven == 1 && ATD1R <= 612){
      if(kurzor != 3){
            dcls();
            dtext("Program 3");
            kurzor = 3;
        }
    } else if(uroven == 1 && ATD1R <= 816){
      if(kurzor != 4){
            dcls();
            dtext("Program 4");
            kurzor = 4;
        }
    } else if(uroven == 1 && ATD1R <= 1023){
      if(kurzor != 5){
            dcls();
            dtext("Program 5");
            kurzor = 5;
        }
    } else if(uroven == 3 && ATD1R <= 341){
      if(kurzor != 1){
            dcls();
            dtext("Vypsani casu");
            kurzor = 1;
        }
    } else if(uroven == 3 && ATD1R <= 682){
      if(kurzor != 2){
            dcls();
            dtext("Nastaveni hodin");
            kurzor = 2;
        }
    } else if(uroven == 3 && ATD1R <= 1023){
      if(kurzor != 3){
            dcls();
            dtext("Nastaveni minut");
            kurzor = 3;
        }
    } else if(uroven == 4){  //nastaveni hodin
      spozdeni = ATD1R;
      dcls();
      dtext("Cas ");
      t = 60 * ((int)(ATD1R / 42));
      intToTime(t,buffer);
      dtext(buffer);
      rtm_delay_p(pMenu,2);
    } else if(uroven == 5){  //nastaveni minut
      spozdeni = ATD1R;
      dcls();
      dtext("Cas ");
      t = ((int)(ATD1R / 17));
      intToTime(t,buffer);
      dtext(buffer);
      rtm_delay_p(pMenu,2);
    } else if(uroven == 2){  //nastaveni spozdeni
      spozdeni = ATD1R;
      dcls();
      dtext("Spozdeni ");
      intToTime(ATD1R,buffer);
      dtext(buffer);
      rtm_delay_p(pMenu,2);
    }
  }
  rtm_stop_p(pMenu);
}

void sci( void )
{
  int povoleno = 0;
  char *ukChar;
  char znak;
  sci1_str_out("Zadejte heslo\r\n");
  pozice = 0;
    
  while(1){
    do{
      znak = sci1_in();
      buffer[pozice++] = znak;
    }while(znak != '#' && pozice != 30);
    buffer[--pozice] = '\0';
    
    if(strstr(buffer,"1234") != NULL && povoleno == 0)
    {
        sci1_str_out("Prihlasen\r\n");
        povoleno = 1;
    }
    if(povoleno == 1){
      if(strstr(buffer,"setTime") != NULL){
        ukChar = strstr(buffer,"setTime") + 8;
        cas = (10 * ((*ukChar) - '0') + (*(ukChar + 1) - '0')) * 60;
        cas += (10 * ((*(ukChar + 3)) - '0') + (*(ukChar + 4) - '0'));
      } else if(strstr(buffer,"clean") != NULL){
        stats[0] = 0;
        stats[1] = 0;
        stats[2] = 0;
        stats[3] = 0;
        stats[4] = 0;
        celkovyCas = 0;
      } else if(strstr(buffer,"getTime") != NULL){
        intToTime(cas,buffer);
        sci1_str_out(buffer);
        sci1_str_out("\r\n");
      } else if(strstr(buffer,"getActiveTime") != NULL){
        intToTime(celkovyCas,buffer);
        sci1_str_out(buffer);
        sci1_str_out("\r\n");
      } else if(strstr(buffer,"getFactory") != NULL){
        sprintf(buffer,"Vyrobni cislo - %d\r\n",factoryNum);
        sci1_str_out(buffer);
        sprintf(buffer,"Datum vyroby - %s\r\n",factoryDate);
        sci1_str_out(buffer);
      } else if(strstr(buffer,"getStats") != NULL){
        sprintf(buffer,"Program 1 - %d\r\n",stats[0]);
        sci1_str_out(buffer);
        sprintf(buffer,"Program 2 - %d\r\n",stats[1]);
        sci1_str_out(buffer);
        sprintf(buffer,"Program 3 - %d\r\n",stats[2]);
        sci1_str_out(buffer);
        sprintf(buffer,"Program 4 - %d\r\n",stats[3]);
        sci1_str_out(buffer);
        sprintf(buffer,"Program 5 - %d\r\n",stats[4]);
        sci1_str_out(buffer);
      } else if(strstr(buffer,"konec") != NULL){
        sci1_str_out("Odhlasen\r\n");
        rtm_ch_period_p(pKeyboard,2);
        rtm_start_p(pKeyboard,0,2);
        rtm_stop_p(pSCI);
        return;
      }
    } else
    {   
      rtm_ch_period_p(pKeyboard,2);
      rtm_start_p(pKeyboard,0,2);
      rtm_stop_p(pSCI);
      return;
    }
    pozice = 0;
  }
  pozice = 0;
  rtm_stop_p(pSCI);
}

void keyboard( void )
{
  int pomInt = 1;
  __RESET_WATCHDOG();
  if(gpio_button_test(0x10) && stav == 0){
  if(!rtm_read_q(s1,&pomInt))
  {
    rtm_write_q(s1,&pomInt);
    rtm_stop_p(pKeyboard);
  }
  rtm_write_q(s1,&pomInt);
  rtm_delay_p(pKeyboard,1);
  if(gpio_button_test(0x10) != 1 || stav != 0)
    rtm_stop_p(pKeyboard);
    if((uroven == 1 || uroven == 3) && kurzor == 0){
      uroven = 0;
      kurzor = -1;
    } else if(uroven == 1 && kurzor == 1){//program 1
      pozice = 0;
      uroven = 0;
      prog = programy[0] << 1;
      if(spozdeni != 0){
        prog |= 1;
        initState = 1;
      } else
        initState = 3;
      gpio_led_off(0xf);
      gpio_led_on(0x1);
      dcls();
      dtext("Zvoleno");
      setcursor(2,1);
      dtext("Program 1");
    } else if(uroven == 1 && kurzor == 2){//program 2
      pozice = 1;
      uroven = 0;
      prog = programy[1] << 1;
      if(spozdeni != 0){
        prog |= 1;
        initState = 1;
      } else
        initState = 2;
      gpio_led_off(0xf);
      gpio_led_on(0x2);
      dcls();
      dtext("Zvoleno");
      setcursor(2,1);
      dtext("Program 2");
    } else if(uroven == 1 && kurzor == 3){//program 3
      pozice = 2;
      uroven = 0;
      prog = programy[2] << 1;
      if(spozdeni != 0){
        prog |= 1;
        initState = 1;
      } else
        initState = 3;
      gpio_led_off(0xf);
      gpio_led_on(0x3);
      dcls();
      dtext("Zvoleno");
      setcursor(2,1);
      dtext("Program 3");
    } else if(uroven == 1 && kurzor == 4){//program 4
      pozice = 3;
      uroven = 0;
      prog = programy[3] << 1;
      if(spozdeni != 0){
        prog |= 1;
        initState = 1;
      } else
        initState = 3;
      gpio_led_off(0xf);
      gpio_led_on(0x4);
      dcls();
      dtext("Zvoleno");
      setcursor(2,1);
      dtext("Program 4");
    } else if(uroven == 1 && kurzor == 5){//program 5
      pozice = 4;
      uroven = 0;
      prog = programy[4] << 1;
      if(spozdeni != 0){
        prog |= 1;
        initState = 1;
      } else
        initState = 2;
      gpio_led_off(0xf);
      gpio_led_on(0x5);
      dcls();
      dtext("Zvoleno");
      setcursor(2,1);
      dtext("Program 5");
    } else if(uroven == 2){
      ATD1SC = 0;                                      // spusteni prevodu
      while(ATD1SC_CCF == 0);
      spozdeni = ATD1R;
      uroven = 0;
      dcls();
      dtext("Zvoleno spozdeni");
      setcursor(2,1);
      intToTime(spozdeni,buffer);
      dtext(buffer);
    } else if(uroven == 4){     //zvoleni hodin
      ATD1SC = 0;                                      // spusteni prevodu
      while(ATD1SC_CCF == 0);
      cas = 60 * ((int)(ATD1R / 42));
      uroven = 0;
      dcls();
      dtext("Cas nastaven");
      setcursor(2,1);
      intToTime(cas,buffer);
      dtext(buffer);
    } else if(uroven == 5){      //zvoleni minut
      ATD1SC = 0;                                      // spusteni prevodu
      while(ATD1SC_CCF == 0);
      cas = (int)(ATD1R / 17);
      uroven = 0;
      dcls();
      dtext("Cas nastaven");
      setcursor(2,1);
      intToTime(cas,buffer);
      dtext(buffer);
    } else if(uroven == 3 && kurzor == 0){
      uroven = 0;
      kurzor = -1;
    } else if(uroven == 3 && kurzor == 1){ //vypsání èasu
      dcls();
      dtext("Cas ");
      intToTime(cas,buffer);
      dtext(buffer);
    } else if(uroven == 3 && kurzor == 2){ //nastavení hodin
      uroven = 4;
    } else if(uroven == 3 && kurzor == 3){ //nastavení minut
      uroven = 5;
    }
    else if(uroven == 0 && kurzor == 0){//zastavení praèky      
      rtm_ch_period_p(pCas, 0);      
      rtm_stop_p(pMenu);
      rtm_stop_p(pCas);
      rtm_ch_period_p(pKeyboard, 0);
      rtm_continue_p(init);
      rtm_stop_p(pKeyboard);//pravdìpodobnì bude tøeba zmìnit i periody spouštìní na 0
    } else if(uroven == 0 && kurzor == 1){
      uroven = 1;
      kurzor = -1;
    } else if(uroven == 0 && kurzor == 2){
      uroven = 2;
      kurzor = -1;
    } else if(uroven == 0 && kurzor == 3){
      uroven = 3;
      kurzor = -1;
    } else if(uroven == 0 && kurzor == 4){
      dcls();
      dtext("Diagnostika");
      rtm_ch_period_p(pKeyboard,0);
      rtm_start_p(pSCI,0,0);
    } else if(uroven == 0 && kurzor == 5){
      //spuštìní programu
      if(spozdeni != 0)
        spozdeni += cas;
      stats[pozice]++;      
      zacatekPrani = cas;
      rtm_start_p(pProgram,0,0);
    }
     
  } else if(gpio_button_test(0x80) && stav != 0){
    if(stav == 1)
    {
      gpio_led_off(0xf);   
      rtm_stop_p(pProgram);
    }
    stav = 6;
    driver_pracka_vypust(1);
    driver_pracka_napust(0);
    driver_pracka_topeni(0);
    driver_buben_rychlost(0);
    driver_buben_smer(0);
    prog = 0xff;
    initState = 0;
    pozice = 0;
    spozdeni = 0;
  } else
  {
    rtm_read_q(s1,&pomInt);
  }
  rtm_stop_p(pKeyboard);
}

void program( void )
{
  dcls();
  setcursor(1,1);
  dtext("Program");
  stav = initState;
  if(prog != 0)
    programSequence();
  else
  {
    dcls(); 
    dtext("Program nezvolen");
  }
  rtm_stop_p(pProgram);
}
  
void programSequence(){
  while(1){
      if(((prog >> (stav - 1)) & 1) != 0){
        
      switch(stav){
        case 1 ://spozdìné praní 
          if(cas >= spozdeni){
            ++stav;
          } else{
            dcls();
            dtext("Program zacne za");
            setcursor(2,1);
            intToTime((spozdeni - cas),buffer);
            dtext(buffer);
            rtm_delay_p(pProgram,2);
          }
        break;
        case 2 : //pøedpírka
          dcls();
          dtext("Predpirka");
          predpirka();
          stav = (stav != 6 ? ++stav : 6);
        break;
        case 3 : //hlavní praní
          dcls();
          dtext("Hlavni prani");          
          hlavniPrani();
          stav = (stav != 6 ? ++stav : 6);
        break;
        case 4 : //machani
          dcls();
          dtext("Machani");
          machani();
          stav = (stav != 6 ? ++stav : 6);
        break;
        case 5 : //zdimani
          dcls();
          dtext("Zdimani");
          zdimani();
          stav = 6;
        break;
        default : //konec praní
          dcls();
          dtext("Ukoncovani");
          ukonci();
          gpio_led_off(0xf);
          celkovyCas += (cas - zacatekPrani);
          prog = 0;
          stav = 0;
          initState = 0;
          dcls();
          dtext("Ukonceno");
          return;
        break;
      }
    } else{
      stav++;
    }
  }
}  

void ukonci(){
  int casPom;
  driver_pracka_napust(0);
  driver_pracka_vypust(1);
  driver_pracka_topeni(0);
  driver_buben_rychlost(0);
  driver_buben_smer(0);
  while(driver_pracka_hladina() != 0 || driver_pracka_teplota() != 0)__RESET_WATCHDOG();
  casPom = cas + 5;
  while(cas <= casPom)
    __RESET_WATCHDOG();
  driver_pracka_vypust(0);
}

void predpirka(){
  driver_pracka_napust(1);
  while(driver_pracka_hladina() != 50 && stav != 6)__RESET_WATCHDOG();
  driver_pracka_napust(0);
  tocBubnem(10);
  dokonciCinnost();  
}

void hlavniPrani(){
  int teplota = (prog & 0xffff) >> 5;
  switch(teplota){
    case 1 :
      teplota = 30;
    break;
    case 2 :
      teplota = 40;
    break;
    case 4 :
      teplota = 60;
    break;
    case 8 :
      teplota = 90;
    break;
  }
  
  driver_pracka_napust(1);
  driver_pracka_topeni(1);
  while((driver_pracka_hladina() != 50 || driver_pracka_teplota() != teplota) && stav != 6){
    __RESET_WATCHDOG();
    if(driver_pracka_hladina() == 50)
      driver_pracka_napust(0);
    if(driver_pracka_teplota() == teplota)
      driver_pracka_topeni(0);
  }
  driver_pracka_topeni(0);
  driver_pracka_napust(0);
  tocBubnem(20);
  dokonciCinnost();
}

void machani(){
  driver_pracka_napust(1);
  while(driver_pracka_hladina() != 100 && stav != 6)__RESET_WATCHDOG();
  driver_pracka_napust(0);
  tocBubnem(20);
  dokonciCinnost();
}

void zdimani(){
  int casPom = cas + 15;
  driver_buben_smer(1);
  driver_buben_rychlost(1);
  driver_pracka_vypust(1);
  while(cas <= casPom && stav != 6)
    __RESET_WATCHDOG();
  driver_pracka_vypust(0);  
}

void dokonciCinnost(){
  int casPom;
  driver_pracka_vypust(1);
  while((driver_pracka_hladina() != 0 || driver_pracka_teplota() != 0) && stav != 6);
  casPom = cas + 5;
  while(cas <= casPom && stav != 6)
    __RESET_WATCHDOG();
  driver_pracka_vypust(0);
}

void tocBubnem(int doba){
  int casPom = cas;
  int smer = 1;
  doba += cas;
  driver_buben_smer(smer);
  while(cas <= doba  && stav != 6){
    casPom = cas + 2;
    while(cas <= casPom);
    smer *= -1;
    driver_buben_smer(0);
    while(cas <= casPom + 1)
        __RESET_WATCHDOG();
    driver_buben_smer(smer);
  }
  driver_buben_smer(0);
}

void intToTime(unsigned long time, char *out)
{
    int pom = time / 60;
    if(pom >= 10)
    {
        out[0] = ((int)(pom / 10)) + '0';
        out[1] = (pom % 10) + '0';
    } else
    {
        out[0] = '0';
        out[1] = (int) pom + '0';
    }
    out[2] = ':';
    pom = time % 60;
    if(pom >= 10)
    {
        out[3] = ((int)(pom / 10)) + '0';
        out[4] = (pom % 10) + '0';
    } else
    {
        out[3] = '0';
        out[4] = (int) pom + '0';
    }
    out[5] = '\0';
}
#if 0
void hours()
{
    long t;
    SettingHours = 1;
    while(gpio_button_test(0x20) == 1){
      t = 0xfff;
      __RESET_WATCHDOG();
      ATD1SC = 0;                                      // spusteni prevodu
      while(ATD1SC_CCF == 0);
      cas = 3600 * ATD1R / 42;
      intToTime(cas,buffer);
      dcls();
      dtext(buffer);
      while(t-- > 0);
    }
    SettingHours = 0;
    rtm_stop_p(pHours);
}

void minutes()
{
    long t;
    SettingMinutes = 1;
    while(gpio_button_test(0x40) == 1){
      t = 0xfff;
      __RESET_WATCHDOG();
      ATD1SC = 0;                                      // spusteni prevodu
      while(ATD1SC_CCF == 0);
      cas = ATD1R / 17;
      intToTime(cas,buffer);
      dcls();
      dtext(buffer);
      while(t-- > 0);
    }
    SettingMinutes = 0;
    rtm_stop_p(pMinutes);
}
#endif