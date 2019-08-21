/*
// Variables para el manejo de la botonera
// del LCD Keypad Shield

#define btnRIGHT   0
#define btnUP      1
#define btnDOWN    2
#define btnLEFT    3
#define btnSELECT  4
#define btnNONE    5
#define btnPRESSED 6
int lcd_key     = btnNONE;
int pressed_key = btnNONE;
int adc_key_in  = 0;
bool btnFlag = false;
unsigned long btnMillis = 0;
*/
int read_LCD_buttons()
{
  /* Mis botones dan: en dos placas
     right  0     0
     none   1023  1023
     up     99    100
     down   256   257
     left   409   412
     select 639   641
  */

  // Leemos A0
  adc_key_in = analogRead(0);
  // chequea si no hay ninguno apretado
  if (adc_key_in > 1020) {
    // baja el flag solo si paso cierto tiempo
    if (millis() - btnMillis > 200)
      btnFlag = false;
    lcd_key = btnNONE;
  }
  else {
    // si hay algun boton presionado
    if (adc_key_in < 1000) lcd_key = btnSELECT;
    if (adc_key_in < 630)  lcd_key = btnLEFT;
    if (adc_key_in < 400)  lcd_key = btnDOWN;
    if (adc_key_in < 250)  lcd_key = btnUP;
    if (adc_key_in < 90)  lcd_key = btnRIGHT;

    // verifica si el flag estaba arriba
    if (btnFlag) {
      // y cambia la lcd_key para indicar que sigue presionado
      pressed_key = lcd_key;
      lcd_key = btnPRESSED;
    }
    else {
      btnFlag = true;
      btnMillis = millis();
    }
  }
}
