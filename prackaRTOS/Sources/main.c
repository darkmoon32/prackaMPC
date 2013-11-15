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
char buffer[32],znak;
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
  sci1_init(BD9600);  
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
  rtm_create_p("proc3", 2, keyboard, 32, &pKeyboard ); 
  rtm_create_p("proc5", 200, program, 0x100, &pProgram ); 
  rtm_create_p("proc6", 1, casPracky, 32, &pCas ); 
  rtm_create_p("proc7", 254, menu, 0x100, &pMenu );
  
  rtm_start_p(pCas, 0, 20);
  rtm_start_p(pKeyboard, 0, 2);
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
    __RESET_WATCHDOG();
    ATD1SC = 0;                                      // spusteni prevodu
    while(ATD1SC_CCF == 0);
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
  int povoleno = 0;
  char *ukChar;
  sci1_str_out("Zadejte heslo\r\n");
  pozice = 0;
    
  while(1){
    do{
      znak = sci1_in();
      buffer[pozice++] = znak;
    }while(znak != '\0' || pozice != 30);
    if(strstr(buffer,"1234") != NULL || povoleno == 1){
      if(strstr(buffer,"setTime") != NULL){
        ukChar = strstr(buffer,"setTime") + 8;
        cas = (10 * (*ukChar + '0') + *(ukChar + 1) + '0') * 60;
        cas += (10 * (*(ukChar + 3) + '0') + *(ukChar + 4) + '0');
      } else if(strstr(buffer,"clean") != NULL){
        stats[0] = 0;
        stats[1] = 0;
        stats[2] = 0;
        stats[3] = 0;
        stats[4] = 0;
        celkovyCas = 0;
      } else if(strstr(buffer,"getTime") != NULL){
        sprintf(buffer,"%d:%d\r\n",(cas / 60),(cas % 60) * 60);
        sci1_str_out(buffer);
      } else if(strstr(buffer,"getActiveTime") != NULL){
        sprintf(buffer,"%d:%d\r\n",(celkovyCas / 60),(celkovyCas % 60) * 60);
        sci1_str_out(buffer);
      } else if(strstr(buffer,"getFactory") != NULL){
        sprintf(buffer,"Výrobní èíslo - %d\r\nDatum výroby - %s\r\n",factoryNum,factoryDate);
        sci1_str_out(buffer);
      } else if(strstr(buffer,"getStats") != NULL){
        sprintf(buffer,"Program 1 - %d\r\nProgram 2 - %d\r\nProgram 3 - %d\r\nProgram 4 - %d\r\nProgram 5 - %d\r\n",stats[0],stats[1],stats[2],stats[3],stats[4]);
        sci1_str_out(buffer);
      } else if(strstr(buffer,"konec") != NULL){
        return;
      }
      povoleno = 1;
      pozice = 0;
    } else
      break;
  }
  pozice = 0;
  rtm_stop_p(pSCI);
}

void keyboard( void )
{
  __RESET_WATCHDOG();
  if(gpio_button_test(0x10) && stav == 0){
  douta(uroven + '0');
  rtm_delay_p(pKeyboard,1);
  if(gpio_button_test(0x10) == 0 && stav != 0)
    return;
    if(uroven == 1 && kurzor == 0){
      uroven = 0;
    } else if(uroven == 1 && kurzor == 1){//program 1
      uroven = 0;
      prog = programy[0] << 1;
      if(spozdeni != 0){
        prog |= 1;
        initState = 1;
      } else
        initState = 3;
      gpio_led_off(0xf);
      gpio_led_on(0x1);
    } else if(uroven == 1 && kurzor == 2){//program 2
      uroven = 0;
      prog = programy[1] << 1;
      if(spozdeni != 0){
        prog |= 1;
        initState = 1;
      } else
        initState = 2;
      gpio_led_off(0xf);
      gpio_led_on(0x2);
    } else if(uroven == 1 && kurzor == 3){//program 3
      uroven = 0;
      prog = programy[2] << 1;
      if(spozdeni != 0){
        prog |= 1;
        initState = 1;
      } else
        initState = 3;
      gpio_led_off(0xf);
      gpio_led_on(0x3);
    } else if(uroven == 1 && kurzor == 4){//program 4
      uroven = 0;
      prog = programy[3] << 1;
      if(spozdeni != 0){
        prog |= 1;
        initState = 1;
      } else
        initState = 3;
      gpio_led_off(0xf);
      gpio_led_on(0x4);
    } else if(uroven == 1 && kurzor == 5){//program 5
      uroven = 0;
      prog = programy[4] << 1;
      if(spozdeni != 0){
        prog |= 1;
        initState = 1;
      } else
        initState = 2;
      gpio_led_off(0xf);
      gpio_led_on(0x5);
    } else if(uroven == 2){
      uroven = 0;
    } else if(uroven == 0 && kurzor == 0){//zastavení praèky
      rtm_ch_period_p(pKeyboard, 0);
      rtm_stop_p(pKeyboard);//pravdìpodobnì bude tøeba zmìnit i periody spouštìní na 0
      rtm_stop_p(pCas);
    } else if(uroven == 0 && kurzor == 1){
      uroven = 1;
    } else if(uroven == 0 && kurzor == 2){
      uroven = 2;
    } else if(uroven == 0 && kurzor == 3){
      dcls();
      sprintf(buffer,"%d:%d",(cas / 60),(cas % 60) * 60);
      dtext(buffer);
    } else if(uroven == 0 && kurzor == 4){
      dcls();
      dtext("Diagnostika");
      rtm_ch_period_p(pKeyboard,0);
      rtm_start_p(pSCI,0,0);
    } else if(uroven == 0 && kurzor == 5){
      //spuštìní programu
      if(spozdeni != 0)
        spozdeni += cas;
      stats[prog]++;      
      zacatekPrani = cas;
      rtm_start_p(pProgram,0,0);
    } 
  } else if(gpio_button_test(0x20) && stav == 0){
    rtm_ch_period_p(pKeyboard,0);
    while(gpio_button_test(0x20)){
      ATD1SC = 0;                                      // spusteni prevodu
      while(ATD1SC_CCF == 0);
      cas = 3600 * ATD1R / 1023;
      sprintf(buffer, "%d:%d",(cas / 60),(cas % 60) * 60);
    }
    rtm_ch_period_p(pKeyboard,2);
  } else if(gpio_button_test(0x40) && stav == 0){
    rtm_ch_period_p(pKeyboard,0);
    while(gpio_button_test(0x20)){
      ATD1SC = 0;                                      // spusteni prevodu
      while(ATD1SC_CCF == 0);
      cas = 60 * ATD1R / 1023;
      sprintf(buffer, "%d:%d",(cas / 60),(cas % 60) * 60);
    }
    rtm_ch_period_p(pKeyboard,2);
  } else if(gpio_button_test(0x80) && stav != 0){
    if(stav == 1)
      rtm_stop_p(pProgram);
    stav = 6;
    driver_pracka_vypust(1);
    driver_pracka_napust(0);
    driver_pracka_topeni(0);
    driver_buben_rychlost(0);
    driver_buben_smer(0);
    prog = 0;
    initState = 0;
  }
  rtm_stop_p(pKeyboard);
}

void program( void )
{
  dcls();
  setcursor(1,1);
  dtext("Program");
  stav = initState;
  programSequence();
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
            sprintf(buffer,"%d:%d",((spozdeni - cas) / 60),((spozdeni - cas) / 60) * 60);
            dtext(buffer);
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
          dtext("Ukoncovani");
          ukonci();
          gpio_led_off(0xf);
          celkovyCas += (cas - zacatekPrani);
          prog = 0;
          stav = 0;
          initState = 0;
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
  while(cas <= casPom)
    __RESET_WATCHDOG();
  driver_pracka_vypust(0);  
}

void dokonciCinnost(){
  int casPom;
  driver_pracka_vypust(1);
  while(driver_pracka_hladina() != 0);
  casPom = cas + 5;
  while(cas <= casPom)
    __RESET_WATCHDOG();
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
    while(cas <= casPom + 1)
        __RESET_WATCHDOG();
    driver_buben_smer(smer);
  }
  driver_buben_smer(0);
}