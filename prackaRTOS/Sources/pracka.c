#include "derivative.h"

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
        case 2://predpirka
        break;
        case 3://hl prani
        break;
        case 4://machani
        break;
        case 5://zdimani
        break;
        default : 
          gpio_led_off(0xf);
          rtm_stop_p(pProgram);
        break;
      }
    } else{
      stav++;
    }
  }

}