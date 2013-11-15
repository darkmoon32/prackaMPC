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

unsigned long cas = 0,spozdeni = 0, zacatekPrani = 0,celkovyCas = 0;
char buffer[32];
int stav = 0, prog = 0, kurzor = 0, uroven = 0;
int stats[] = {0,0,0,0,0};
int programy[] = {0b01001110,0b10001111,0b00010010,0b00101010,0b00101011};
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
#if 0  
  dinit();
  setcursor(1,1);
  dtext("Praèka");
#endif
  gpio_hard_init();
  gpio_button_init(0xf0);
  gpio_led_init(0xf);
  
  ATD1C = 0xC4;   	// zapnuti prevodniku, 8 bit vysledek    
  ATD1PE = 1;     	// pin PTB0 prepneme do rezimu 			  	// vstupu A/D prevodniku  
  PTADD_PTADD5 =0;      	// PTA4 vstupni rezim
  PTAPE_PTAPE5 = 1;	// pull-up pro PTA4 zapnut
  
  rtm_init(&init);
  rtm_create_p("proc2", 100, sci, 0x100, &pSCI );	 
  rtm_create_p("proc3", 2, keyboard, 32, &pKeyboard ); 
  rtm_create_p("proc5", 200, program, 0x100, &pProgram ); 
  rtm_create_p("proc6", 1, casPracky, 32, &pCas ); 
  rtm_create_p("proc7", 254, menu, 0x100, &pMenu );
  
  rtm_start_p(pCas, 0, 20);
  rtm_start_p(pKeyboard, 0, 1);
  rtm_start_p(pMenu,0,0);
  rtm_delay_p(init, 0);

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
  while(1){
    ATD1SC = 0;                                      // spusteni prevodu
    while(ATD1SC_CCF == 0);
    dcls();
    if(ATD1R == 0){//potenctiometr je na 0, rezervováno pro položku zpìt nebo konec
      if(uroven == 0)
        dtext("Konec");
      else
        dtext("Zpìt");
      kurzor = 0;
    } else if(uroven == 0 && ATD1R <= 255){//hlavní nabídka a výbìr programu
      dtext("Vyber programu");
      kurzor = 1;
    } else if(uroven == 0 && ATD1R <= 510){
      dtext("Odlozeny start");
      kurzor = 2;
    } else if(uroven == 0 && ATD1R <= 765){
      dtext("Cas");
      kurzor = 3;
    } else if(uroven == 0 && ATD1R <= 1023){
      dtext("Start");
      kurzor = 4;
    } else if(uroven == 1 && ATD1R <= 204){//podmenu pro výber programu
      dtext("Program 1");
      kurzor = 1;
    } else if(uroven == 1 && ATD1R <= 408){
      dtext("Program 2");
      kurzor = 2;
    } else if(uroven == 1 && ATD1R <= 612){
      dtext("Program 3");
      kurzor = 3;
    } else if(uroven == 1 && ATD1R <= 816){
      dtext("Program 4");
      kurzor = 4;
    } else if(uroven == 1 && ATD1R <= 1023){
      dtext("Program 5");
      kurzor = 5;
    } else if(uroven == 2){
      spozdeni = ATD1R;
      sprintf(buffer,"Spozdeni %d:%d",(ATD1R / 60), ((ATD1R % 60)*60));
      dtext(buffer);
    }
  }
  rtm_stop_p(pMenu);
}

void sci( void )
{
  
  rtm_stop_p(pSCI);
}

void keyboard( void )
{
  __RESET_WATCHDOG();
  if(gpio_button_test(0x10) && stav == 0){
    if(uroven == 0 && kurzor == 0){//zastavení praèky
      rtm_stop_p(pKeyboard);//pravdìpodobnì bude tøeba zmìnit i periody spouštìní na 0
      rtm_stop_p(pCas);
    } else if(uroven == 0 && kurzor == 1){
      uroven = 1;
    } else if(uroven == 0 && kurzor == 2){
      uroven = 2;
    } else if(uroven == 0 && kurzor == 3){
      dcls();
      sprintf(buffer,"Cas %d:%d",(cas / 60),((cas % 60) * 60));
      dtext(buffer);
    } else if(uroven == 0 && kurzor == 4){
      //spuštìní programu
      if(spozdeni != 0)
        spozdeni += cas;
      stats[prog]++;      
      zacatekPrani = cas;
      rtm_start_p(pProgram,0,0);
    } else if(uroven == 1 && kurzor == 0){
      uroven = 0;
    } else if(uroven == 1 && kurzor == 1){//program 1
      uroven = 0;
      prog = 0;
      if(spozdeni != 0){
        prog = (prog << 1) | 1;
        stav = 1;
      } else
        stav = 3;
      gpio_led_off(0xf);
      gpio_led_on(0x1);
    } else if(uroven == 1 && kurzor == 2){//program 2
      uroven = 0;
      prog = 1;
      if(spozdeni != 0){
        prog = (prog << 1) | 1;
        stav = 1;
      } else
        stav = 2;
      gpio_led_off(0xf);
      gpio_led_on(0x2);
    } else if(uroven == 1 && kurzor == 3){//program 3
      uroven = 0;
      prog = 2;
      if(spozdeni != 0){
        prog = (prog << 1) | 1;
        stav = 1;
      } else
        stav = 3;
      gpio_led_off(0xf);
      gpio_led_on(0x3);
    } else if(uroven == 1 && kurzor == 4){//program 4
      uroven = 0;
      prog = 3;
      if(spozdeni != 0){
        prog = (prog << 1) | 1;
        stav = 1;
      } else
        stav = 3;
      gpio_led_off(0xf);
      gpio_led_on(0x4);
    } else if(uroven == 1 && kurzor == 1){//program 5
      uroven = 0;
      prog = 4;
      if(spozdeni != 0){
        prog = (prog << 1) | 1;
        stav = 1;
      } else
        stav = 2;
      gpio_led_off(0xf);
      gpio_led_on(0x5);
    } else if(uroven == 2){
      uroven = 0;
    }
  }else if(gpio_button_test(0x80) && stav != 0){
    stav = 6;
    driver_pracka_vypust(1);
    driver_pracka_napust(0);
    driver_pracka_topeni(0);
    driver_buben_rychlost(0);
    driver_buben_smer(0);
    prog = 0;
    rtm_stop_p(pProgram);
  }
  rtm_stop_p(pKeyboard);
}

void program( void )
{
  setcursor(1,1);
  dtext("Program");
  programSequence();
  rtm_stop_p(pProgram);
}
  
void programSequence(){
  while(1){
      if(((prog >> (stav - 1)) & 1) != 0){
        
      switch(stav){
        case 1://spozdìné praní 
          if(cas >= spozdeni){
            stav++;
          } else{
            dcls();
            dtext("Program zacne za");
            setcursor(2,1);
            sprintf(buffer,"%d:%d",((spozdeni - cas) / 60),((spozdeni - cas) / 60) * 60);
            dtext(buffer);
          }
        break;
        case 2://pøedpírka
          dcls();
          dtext("Predpirka");
          predpirka();
          stav = (stav != 6 ? stav++ : 6);
        break;
        case 3://hlavní praní
          dcls();
          dtext("Hlavni prani");          
          hlavniPrani();
          stav = (stav != 6 ? stav++ : 6);
        break;
        case 4://machani
          dcls();
          dtext("Máchání");
          machani();
          stav = (stav != 6 ? stav++ : 6);
        break;
        case 5://zdimani
          dcls();
          dtext("Zdimani");
          zdimani();
          stav = 6;
        break;
        default : //konec praní
          dtext("Ukoncovani");
          ukonci();
          gpio_led_off(0xf);
          celkovyCas += (cas - zacatekPrani);
          prog = 0;
          return;
        break;
      }
    } else{
      stav++;
    }
  }
}  

void ukonci(){
  int casPom = cas + 5;
  driver_pracka_napust(0);
  driver_pracka_vypust(1);
  driver_pracka_topeni(0);
  driver_buben_rychlost(0);
  driver_buben_smer(0);
  while(driver_pracka_hladina() != 0 && driver_pracka_teplota() != 0);
  while(cas <= casPom);
  driver_pracka_vypust(0);
}

void predpirka(){
  driver_pracka_napust(1);
  while(driver_pracka_hladina() != 50);
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
  while(driver_pracka_hladina() != 50 && driver_pracka_teplota() != teplota){
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
  while(driver_pracka_hladina() != 100);
  driver_pracka_napust(0);
  tocBubnem(20);
  dokonciCinnost();
}

void zdimani(){
  int casPom = cas + 15;
  driver_buben_smer(1);
  driver_buben_rychlost(1);
  driver_pracka_vypust(1);
  while(cas <= casPom);
  driver_pracka_vypust(0);  
}

void dokonciCinnost(){
  int casPom;
  driver_pracka_vypust(1);
  while(driver_pracka_hladina() != 0);
  casPom = cas + 5;
  while(cas <= casPom);
}

void tocBubnem(int doba){
  int casPom = cas;
  int smer = 1;
  doba += cas;
  driver_buben_smer(smer);
  while(cas <= doba){
    casPom = cas + 2;
    while(cas <= casPom);
    smer *= -1;
    driver_buben_smer(0);
    while(cas <= casPom + 1);
    driver_buben_smer(smer);
  }
  driver_buben_smer(0);
}