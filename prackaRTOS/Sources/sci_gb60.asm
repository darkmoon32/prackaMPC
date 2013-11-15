;****************************************************************************
;* Ovladac seriove linky pro vyvojovy kit M68EVB08GB60
;* UTB ve Zline, 2006
;* Implementovane podprogramy:
;*        sci1_init     - inicializace SCI1 rozhrani, v reg.A ocekava kod prenosove rychlosti
;*        sci1_out      - odeslani znaku na SCI1 rozhrani (ceka na uvolneni vysilace)
;*        sci1_in       - prijem znaku z SCI1 rozhrani (ceka na prijaty znak)
;*        sci1_str_out  - vystup textoveho retezce na SCI1, vstup H:X adr. retezce zakonceneho null
;****************************************************************************


          XDEF      sci1_init         ; zpristupneni funkci ovladace okoli
          XDEF      sci1_out
          XDEF      sci1_in
          XDEF      sci1_str_out

          INCLUDE   'derivative.inc'


; variable/data section
MY_ZEROPAGE: SECTION  SHORT             ; Insert here your data definition


; code section
MyCode:     SECTION



;*************************************************************************************
;*
;* podprogram sci1_init - inicializace rozhrani SCI1
;*
;*        - provede inicializaci SCI1 rozhrani na pozadovanou prenosovou rychlost
;*        - predpoklada se fbus = 20MHz (program je spusten z debuggeru)
;*        - kod prenosove rychlosti se predava v reg.A
;*        - 4=4800Bd, 9=9600Bd, 19=19200Bd, 38=38400Bd, 57=57600Bd, 115=115200Bd
;*        - funkce vraci v reg.A status operace. A=0 indikuje neuspesnou inicializaci
;*
;*************************************************************************************

sci1_init pshx
          pshh
          cmpa      #4
          beq       sc4bd
          cmpa      #9
          beq       sc9bd
          cmpa      #19
          beq       sc19bd
          cmpa      #38
          beq       sc38bd
          cmpa      #57
          beq       sc57bd
          cmpa      #115
          beq       sc115bd
          pulh
          pulx
          clra                          ; chybne zadana prenosova rychlost
          rts                           ; navrat s chybou
          
sc4bd     ldhx      #260
          sthx      SCI1BDH             ; nastaveni baudrate
          bra       scend
sc9bd     ldhx      #130                
          sthx      SCI1BDH             ; nastaveni baudrate
          bra       scend
sc19bd    ldhx      #65                
          sthx      SCI1BDH             ; nastaveni baudrate
          bra       scend
sc38bd    ldhx      #33                
          sthx      SCI1BDH             ; nastaveni baudrate
          bra       scend
sc57bd    ldhx      #22                
          sthx      SCI1BDH             ; nastaveni baudrate
          bra       scend
sc115bd   ldhx      #11                
          sthx      SCI1BDH             ; nastaveni baudrate

scend     mov       #0,SCI1C1
          mov       #%00001100,SCI1C2
          mov       #0,SCI1C3
          pulh
          pulx
          lda       #1                  ; nastaveni SCI1 uspesne
          rts



;*************************************************************************************
;*
;* podprogram sci1_in - prijem znaku z SCI1
;*
;*        - prijaty znak je ulozen do reg.A
;*
;*************************************************************************************

sci1_in   nop
si1       feed_watchdog
          lda       SCI1S1              ; precteme stavovy reg.1
          and       #%00100000          ; je nastaven RDRF?
          beq       si1                 ; pokud ne, cekej na prijem znaku
          lda       SCI1D               ; preceteme z datoveho registru prijaty znak
          rts



;*************************************************************************************
;*
;* podprogram sci1_out - vyslani znaku na SCI1
;*
;*        - vysilany znak se ocekava v reg.A
;*
;*************************************************************************************

sci1_out  nop
          psha
so1       feed_watchdog
          lda       SCI1S1              ; precteme stavovy reg.1
          and       #%10000000          ; je nastaven TDRE?
          beq       so1                 ; pokud ne, cekej na uvolneni vysilaciho registru
          pula
          sta       SCI1D               ; zapiseme vysilany znak
          rts



;*************************************************************************************
;*
;* podprogram sci1_str_out - vyslani retezce znaku na SCI1
;*
;*        - adresa retezce se ocekava v reg. H:X
;*        - konec retezce indikuje znak NULL
;*
;*************************************************************************************

sci1_str_out  
          nop
          pshh                          ; ulozime stav registru A,H,X
          pshx
          psha
scs1      lda       ,x                  ; znak z retezce do A
          cmp       #0                  ; jsme na konci retezce?
          beq       scs_end             ; pokud ano, skoc na konec
          jsr       sci1_out            ; vysli znak na SCI1
          aix       #1                  ; inkrementace H:X
          bra       scs1
scs_end   pula
          pulx
          pulh
          rts          