/*----------------------------------------
  Pantallas de RUN modo Flow-Rate
  ----------------------------------------*/
void loopFR() {
  switch (screen) {
    case scrFRFLOWRATE:
      loopValueFR();
      break;
    case scrFRPAUSE:
      //equivalente a loopFRScreenRefresh();
      if ((millis() - millisStartScreen) > screenCHANGE) {
        millisStartScreen = millis();
        //rota la pantalla entre dos valores
        screenRotation = (screenRotation + 1) % 2;
        printScreen();
      }
      //
      switch (lcd_key) {
        case btnLEFT:
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("Operation");
          lcd.setCursor(1, 1);
          lcd.print("Cancelled");
          delay(msgDELAY);
          screen = scrMAIN;
          break;
        case btnRIGHT:
          // comandos para iniciar operacion
          // seleccionar direccion
          digitalWrite(directionPin, directionFORWARD);
          // enciende el oscilador
          tone(pulsePin, frequency);
          // habilita la corriente
          digitalWrite(enablePin, currentENABLE);
          // iniciar contador de tiempo
          millisStartRuninng = millis();
          screen = scrFRRUNNING;
          flagProgressScreen = true;
          millisStartScreen = millis();
          break;
      }
      break;
    case scrFRRUNNING:
      /* control por fin de carrera */
      checkEndstop();
      if ((millis() - millisStartScreen) > screenCHANGE) {
        millisStartScreen = millis();
        //rota la pantalla entre tres valores
        screenRotation = (screenRotation + 1) % 3;
        printScreen();
      }
      //
      switch (lcd_key) {
        case btnSELECT:
          // detiene el motor: apaga el oscilador y apaga la corriente
          noTone(pulsePin);
          digitalWrite(enablePin, currentDISABLE);
          // mensaje de salida
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("      FLOW      ");
          lcd.setCursor(0, 1);
          lcd.print("    STOPPED     ");
          delay(msgDELAY);
          // cambio de pantalla
          screen = scrMAIN;
          break;
        case btnRIGHT:
          screen = scrFRFREQ;
          cursorPosition = 3;
          break;
      }
      break;
    case scrFRFREQ:
      /* control por fin de carrera */
      checkEndstop();
      loopValueFreq();
      break;
  }
}
void loopValueFreq() {
  lcd.blink();
  // maximo 5 unidades
  lcd.setCursor(6 + (5 - cursorPosition), 1);
  // convertir posicion de cifra en quanto de suma
  unsigned long quanto;
  // si usamos pow(10, cursorPosition - 1) tiene problemas de redondeo
  switch (cursorPosition) {
    case 1: quanto = 1; break;
    case 2: quanto = 10; break;
    case 3: quanto = 100; break;
    case 4: quanto = 1000; break;
    case 5: quanto = 10000; break;
  }

  unsigned long oldValue = frequency;

  switch (lcd_key) {
    case btnSELECT:
      lcd.clear();
      lcd.noBlink();
      screen = scrFRRUNNING;
      screenRotation = 0;
      millisStartScreen = millis();
      break;
    case btnRIGHT:
      cursorPosition--;
      if (cursorPosition == 0) cursorPosition = 5;
      break;
    case btnLEFT:
      cursorPosition++;
      if (cursorPosition == 6) cursorPosition = 1;
      break;
    case btnUP:
      frequency += quanto;
      if (frequency > MAX_FREQ or frequency < MIN_FREQ)
        frequency = oldValue;
      tone(pulsePin, frequency);
      actualFlowrate = frequency * 1000. / calibration / syringeLength * syringeVolume * 3600.0;
      break;
    case btnDOWN:
      frequency -= quanto;
      if (frequency > MAX_FREQ or frequency < MIN_FREQ)
        frequency = oldValue;
      tone(pulsePin, frequency);
      actualFlowrate = frequency * 1000. / calibration / syringeLength * syringeVolume * 3600.0;
      break;
  }
}

void loopValueFR() {
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

  unsigned long oldValue = flowrate;

  switch (lcd_key) {
    case btnSELECT:
      lcd.clear();
      lcd.noBlink();
      // esta en FRFLOWRATE y pasa a FRPAUSE
      if (flowrate >= minFlowRate & flowrate <= maxFlowRate) {
        //se establece la nueva frecuencia
        frequency = (float)flowrate * syringeLength * calibration / syringeVolume / 3600. / 1000.;
        actualFlowrate = frequency * 1000. / calibration / syringeLength * syringeVolume * 3600.0;
        // se da mensaje de aceptacion
        lcd.setCursor(0, 0);
        lcd.print("FlowRateAcepted:");
        lcd.setCursor(1, 1);
        lcd.print(flowrate);
        lcd.print(" ");
        lcd.write(228);
        lcd.print("L/h");
        delay(msgDELAY);
        screen = scrFRPAUSE;
        screenRotation = 0;
        millisStartScreen = millis();
      }
      else {
        lcd.setCursor(0, 0);
        lcd.print("Must be:");
        lcd.setCursor(0, 1);
        lcd.print(minFlowRate, 0);
        lcd.print("<Q<");
        lcd.print(maxFlowRate, 0);
        delay(msgDELAY);
        screen = scrFRFLOWRATE;
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
      flowrate += quanto;
      break;
    case btnDOWN:
      flowrate -= quanto;
      break;
  }
  if (flowrate > 999999 or flowrate < 1)
    flowrate = oldValue;
}

void printFRFlowRate() {
  lcd.setCursor(0, 0);
  lcd.print("Flow Rate: ");
  lcd.setCursor(0, 1);
  if (flowrate < 10) lcd.print("_");
  if (flowrate < 100) lcd.print("_");
  if (flowrate < 1000) lcd.print("_");
  if (flowrate < 10000) lcd.print("_");
  if (flowrate < 100000) lcd.print("_");

  lcd.print(flowrate);
  lcd.print(" ");
  lcd.write(228);
  lcd.print("L/h");
}

void printFRPause() {
  switch (screenRotation) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print(" RIGHT TO START ");
      lcd.setCursor(0, 1);
      lcd.print(" LEFT TO CANCEL ");
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Set Point: ");
      lcd.setCursor(0, 1);
      lcd.print(flowrate);
      lcd.print(" ");
      lcd.write(228);
      lcd.print("L/h");
      break;
  }
}

void printFRRunning() {
  switch (screenRotation) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("     RUNNING    ");
      lcd.setCursor(0, 1);
      lcd.print(" SELECT TO STOP ");
      break;
    case 1:
      lcd.setCursor(0, 0);
      /*---------1234567890123456*/
      lcd.print("Actual FlowRate:");
      lcd.setCursor(0, 1);
      lcd.print(actualFlowrate, 1);
      lcd.print(" ");
      lcd.write(228);
      lcd.print("L/h");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print(" Press RIGHT to ");
      lcd.setCursor(0, 1);
      lcd.print("real-time adjust");
      break;
  }
}

void printFRFrequency() {
  lcd.setCursor(0, 0);
  lcd.print("F-R: ");
  lcd.print(actualFlowrate);
  lcd.write(228);
  lcd.print("L/h");
  lcd.setCursor(0, 1);
  lcd.print("Freq: ");
  if (frequency < 100) lcd.print("_");
  if (frequency < 1000) lcd.print("_");
  if (frequency < 10000) lcd.print("_");

  lcd.print(frequency);
  lcd.print(" Hz");
}
