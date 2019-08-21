/*----------------------------------------
  Pantallas de calibracion de jeringa
  ----------------------------------------*/
void loopCal() {
  lcd.blink();
  // maximo 6 unidades
  lcd.setCursor(6 - cursorPosition, 1);
  // convertir posicion de cifra en quanto de suma
  unsigned long quanto;
  // si usamos pow(10, cursorPosition - 1) tiene problemas de redondeo
  switch (cursorPosition) {
    case 1: quanto = 1; break;
    case 2: quanto = 10; break;
    case 3: quanto = 100; break;
    case 4: quanto = 1000; break;
    case 5: quanto = 10000; break;
    case 6: quanto = 100000; break;
  }

  unsigned long auxValue;
  // elije con cual de las dos variables trabajar
  switch (screen) {
    case scrCALLENGTH: auxValue = syringeLength; break;
    case scrCALVOLUME: auxValue = syringeVolume; break;
  }
  unsigned long oldValue = auxValue;

  switch (lcd_key) {
    case btnSELECT:
      lcd.clear();
      lcd.noBlink();
      switch (screen) {
        // si esta en largo pasa a volumen
        case scrCALLENGTH:
          screen = scrCALVOLUME;
          lcd.setCursor(0, 0);
          lcd.print("Length Selected");
          delay(msgDELAY);
          return;
          break;
        //si esta en volumen vuelve a main
        case scrCALVOLUME:
          lcd.setCursor(0, 0);
          lcd.print("Volume Selected");
          delay(msgDELAY);
          recalculateLimits();
          screen = scrMAIN;
          break;
      }
      break;
    case btnRIGHT:
      cursorPosition--;
      if (cursorPosition == 0) cursorPosition = 6;
      break;
    case btnLEFT:
      cursorPosition++;
      if (cursorPosition == 7) cursorPosition = 1;
      break;
    case btnUP:
      auxValue += quanto;
      break;
    case btnDOWN:
      auxValue -= quanto;
      break;
  }
  if (auxValue > 999999 or auxValue < 1)
    auxValue = oldValue;

  switch (screen) {
    case scrCALLENGTH: syringeLength = auxValue; break;
    case scrCALVOLUME: syringeVolume = auxValue; break;
  }
}

void printCalLength() {
  lcd.setCursor(0, 0);
  lcd.print("Syringe Length: ");
  lcd.setCursor(0, 1);
  if (syringeLength < 10) lcd.print("_");
  if (syringeLength < 100) lcd.print("_");
  if (syringeLength < 1000) lcd.print("_");
  if (syringeLength < 10000) lcd.print("_");
  if (syringeLength < 100000) lcd.print("_");

  lcd.print(syringeLength);
  lcd.setCursor(7, 1);
  lcd.write(228);
  lcd.print("m");
}

void printCalVolume() {
  lcd.setCursor(0, 0);
  lcd.print("Syringe Volume: ");
  lcd.setCursor(0, 1);
  if (syringeVolume < 10) lcd.print("_");
  if (syringeVolume < 100) lcd.print("_");
  if (syringeVolume < 1000) lcd.print("_");
  if (syringeVolume < 10000) lcd.print("_");
  if (syringeVolume < 100000) lcd.print("_");

  lcd.print(syringeVolume);
  lcd.setCursor(7, 1);
  lcd.write(228);
  lcd.print("L");
}

// calcula los limites de caudal en base a los limites de frecuencia
void recalculateLimits(){
  minFlowRate=MIN_FREQ*1000./calibration*3600.*syringeVolume/syringeLength;
  maxFlowRate=MAX_FREQ*1000./calibration*3600.*syringeVolume/syringeLength;
}
