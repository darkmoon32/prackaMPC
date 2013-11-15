;****************************************************************************
;* Ovladac displeje pro vyvojovy kit M68EVB08GB60 rev.2
;* UTB ve Zline, 2006
;* Implementovane podprogramy:
;*        dinit     - inicializace displeje
;*        dcls      - smazani displeje, nastaveni kurzoru na prvni radek
;*        douta     - vypis znaku obsazenem v reg.A na displej
;*        dtext     - vypis textoveho retezce zakonceneho znakem NULL
;*                  - v reg.H:X se ocekava adresa retezce v pameti
;*        setcursor - nastavi kurzor na pozadovane souradnice
;*                  - reg.X - cislo radku (1 nebo 2)
;*                  - reg.A - cislo sloupce (1 az 16)
;****************************************************************************
;* rev.2 - upravena inicializace portu a prace s porty pro zajisteni kompatibility
;*         s I/O deskou pro EDUmod moduly
;****************************************************************************

rw_pin    equ       3                   ; R/W pin je pripojen na PTG3
rs_pin    equ       6                   ; RS pin je pripojen na PTE6
en_pin    equ       7                   ; EN pin je pripojen na PTE7

          XDEF      dinit, dcls, douta, dtext     ; zpristupneni funkci ovladace okoli
          XDEF      setcursor
          
DLN1      equ       $80
DLN2      equ       $c0          

          INCLUDE 'derivative.inc'


; variable/data section
MY_ZEROPAGE: SECTION  SHORT             ; Insert here your data definition


; code section
MyCode:     SECTION



;*************************************************************************************
;*
;* podprogram dinit - inicializace displeje
;*
;*************************************************************************************

dinit     nop
          clr       PTGD                ;init PTG
          lda       PTED                ;init PTE6-7
          and       #%00111111
          sta       PTED
          
          mov       #%00001000,PTGDD    ;PTG3 je vystup - signal R/W
          lda       PTEDD
          ora       #%11000000          ;PTE6-7 jsou vystupy - signaly RS a EN
          sta       PTEDD
          
          bclr      rw_pin,PTGD         ;0->rw
          bclr      rs_pin,PTED         ;0->rs, budeme zapisovat prikaz
          lda       PTGD
          ora       #$30
          sta       PTGD                ;zapis prikazu na PTG4-7
          lda       PTGDD
          ora       #%11110000
          sta       PTGDD               ;prepnuti PTG4-7 do vystupniho rezimu
          jsr       w500ns              ;pockej 500ns (Set-up time)
          bset      en_pin,PTED         ;1->en, zpracuj prikaz
          jsr       w500ns
          bclr      en_pin,PTED         ;0->en
          jsr       w5ms                ;cekej 5ms na zpracovani prikazu
          
          bset      en_pin,PTED         ;1->en, zpracuj prikaz
          jsr       w500ns
          bclr      en_pin,PTED         ;0->en
          jsr       w5ms                ;cekej 5ms na zpracovani prikazu
          
          bset      en_pin,PTED         ;1->en, zpracuj prikaz
          jsr       w500ns
          bclr      en_pin,PTED         ;0->en
          jsr       w5ms                ;cekej 5ms na zpracovani prikazu
          
          lda       PTGD
          and       #%00001111          ;vynulujem horni 4 bity
          ora       #$20                ;prikaz
          sta       PTGD                ;zapis prikazu na PTG4-7, zapnuti 4 bit rezimu
          jsr       w500ns              ;pockej 500ns (Set-up time)
          bset      en_pin,PTED         ;1->en, zpracuj prikaz
          jsr       w500ns
          bclr      en_pin,PTED         ;0->en
          jsr       w5ms                ;cekej 5ms na zpracovani prikazu

          lda       #$28
          jsr       wr_cmd
          lda       #$6
          jsr       wr_cmd
          lda       #$c
          jsr       wr_cmd
          lda       #$1
          jsr       wr_cmd
          lda       #$80
          jsr       wr_cmd          
          rts




;*************************************************************************************
;*
;* podprogram douta - vypise na pozici kurzoru znak obsazeny v reg.A
;*
;*************************************************************************************

douta     nop
          jsr       wr_data
          rts
          



;*************************************************************************************
;*
;* podprogram dtext - vypise textovy retezec, jehoz adresa je ulozena v H:X
;*                  - retezec musi byt ukoncen znakem NULL
;*
;*************************************************************************************

dtext     psha
          pshx
          pshh
dt1       lda       0,x
          cmpa      #0
          beq       dtend               ;pokud je znak NULL -> konec vypisu
          jsr       douta
          aix       #1
          bra       dt1
dtend     pulh
          pulx
          pula
          rts          



;*************************************************************************************
;*
;* podprogram dcls - smaze obsah displeje a nastavi kurzor na 1.radek
;*
;*************************************************************************************

dcls      psha
          pshh
          pshx
          lda       #1                  ;prikaz smaz displej
          jsr       wr_cmd
          lda       #DLN1               ;adresa 1.znaku na 1.radku
          ora       #%10000000          ;DB7 nastavit na 1
          jsr       wr_cmd
          pulx
          pulh
          pula
          rts



;*************************************************************************************
;*
;* podprogram setcursor - nastavi kurzor na pozadovane souradnice
;*                      - reg. X radek (1-2)
;*                      - reg. A sloupec (1-16)
;*
;*************************************************************************************

setcursor pshx
          pshh
          psha
          cpx       #1
          bne       sc1
          add       #$7f                ;pricteme adresu 1. radku v DDRAM zmensenou o 1
          bra       sc2
sc1       add       #$bf                ;pricteme adresu 2. radku v DDRAM zmensenou o 1
sc2       ora       #%10000000          ;DB7 nastavit na 1
          jsr       wr_cmd
          pula
          pulh
          pulx
          rts

          
          

;*************************************************************************************
;*
;* pomocne podprogramy
;*
;*************************************************************************************

wr_cmd    nop                           ;zapise prikaz jehoz kod je v reg.A
          psha                          ;reg.A na zasobnik
          bclr      rs_pin,PTED         ;0->rs, budeme zapisovat prikaz
          bclr      rw_pin,PTGD
          and       #%11110000          ;vynulujeme dolni 4 bity reg.A          
          sta       PTGD                ;zapisem na port
          lda       PTGDD
          ora       #%11110000
          sta       PTGDD               ;prepnuti PTG4-7 do vystupniho rezimu
          jsr       w500ns              ;pockej 500ns (Set-up time)
          bset      en_pin,PTED         ;1->en, zpracuj prikaz
          jsr       w500ns              ;pockej 500ns (Set-up time)
          bclr      en_pin,PTED         ;0->en
          pula
          rola
          rola
          rola
          rola
          and       #%11110000          ;vynulujeme dolni 4 bity reg.A          
          sta       PTGD                ;zapisem na port
          jsr       w500ns              ;pockej 500ns (Set-up time)
          bset      en_pin,PTED         ;1->en, zpracuj prikaz
          jsr       w500ns              ;pockej 500ns (Set-up time)
          bclr      en_pin,PTED         ;0->en
          jsr       waitbf              ;cekej na dokonceni internich operaci
          rts


wr_data   nop                           ;zapise data z reg.A
          psha                          ;reg.A na zasobnik
          bset      rs_pin,PTED         ;1->rs, budeme zapisovat data
          bclr      rw_pin,PTGD
          and       #%11110000          ;vynulujeme dolni 4 bity reg.A          
          sta       PTGD                ;zapisem na port
          lda       PTGDD
          ora       #%11110000
          sta       PTGDD               ;prepnuti PTG4-7 do vystupniho rezimu
          jsr       w500ns              ;pockej 500ns (Set-up time)
          bset      en_pin,PTED         ;1->en, zpracuj prikaz
          jsr       w500ns              ;pockej 500ns (Set-up time)
          bclr      en_pin,PTED         ;0->en
          pula
          rola
          rola
          rola
          rola
          and       #%11110000          ;vynulujeme dolni 4 bity reg.A          
          sta       PTGD                ;zapisem na port
          jsr       w500ns              ;pockej 500ns (Set-up time)
          bset      en_pin,PTED         ;1->en, zpracuj prikaz
          jsr       w500ns              ;pockej 500ns (Set-up time)
          bclr      en_pin,PTED         ;0->en
          jsr       waitbf              ;cekej na dokonceni internich operaci
          rts
          

w5ms      nop                           ;cekaci smycka 5ms (pro BUSCLK=20MHz)
          pshh
          pshx
w12       ldhx      #8500              
w11       aix       #-1                 ;2 clks
          feed_watchdog                 ;4 clks
          cphx      #0                  ;3 clks          
          bne       w11                 ;3 clks
          pulx
          pulh
          rts
          
          
w500ns    nop                           ;cekani 500ns (pro BUSCLK=20MHz)          
          feed_watchdog                 ;4 clks
          rts                           ;6 clks


waitbf    nop                           ;ceka na dokonceni internich operaci displeje
          psha
          pshx
wbf1      feed_watchdog
          bclr      rs_pin,PTED         ;cmd read sekvence
          mov       #%00001000,PTGDD    ;PTG4-7 vstupni rezim, PTG3 je vystup
          bset      rw_pin,PTGD         ;budeme cist
          jsr       w500ns              ;set-up time
          bset      en_pin,PTED
          bclr      en_pin,PTED
          lda       PTGD                ;precteme status reg. bity 4-7
          bclr      en_pin,PTED
          jsr       w500ns              ;set-up time
          bset      en_pin,PTED
          jsr       w500ns              ;set-up time
          ldx       PTGD                ;precteme status reg. bity 0-3
          bclr      en_pin,PTED
          jsr       w500ns              ;set-up time
          and       #%10000000          ;testujeme priznak BF
          bne       wbf1                ;pokud je 7.bit nenulovy -> skok na wbf1
          pulx
          pula
          rts
                    