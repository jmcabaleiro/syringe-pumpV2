/*----------------------------------------
  Pantallas de RUN modo Volume-Time
  ----------------------------------------*/
void loopVT() {
  switch (screen) {
    case scrVTVOLUME:
    case scrVTTIME:
      loopValueVT();
      break;

    case scrVTPAUSE:
      loopVTScreenRefresh();
      switch (lcd_key) {
        case btnLEFT:
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("Operation");
          lcd.setCursor(1, 1);
          lcd.print("Cancelled");
          delay(msgDELAY);
          screen = scrMAIN;
          flagClearScreen = true;
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
          screen = scrVTRUNNING;
          flagProgressScreen = true;
          millisStartScreen = millis();
          break;
      }
      break;
    case scrVTRUNNING:
      loopVTScreenRefresh();
      /* control por fin de carrera */
      checkEndstop();
      // refrescar valores de tiempo y volumen
      progressTime = (millis() - millisStartRuninng) / 1000.0;
      progressVolume = progressTime / totalTime * totalVolume;
      if (progressTime > totalTime) {
        // procedimiento de detencion
        noTone(pulsePin);
        digitalWrite(enablePin, currentDISABLE);
        screen = scrVTEND;
        flagProgressScreen = true;
        millisStartScreen = millis();
      }
      switch (lcd_key) {
        case btnSELECT:
          // comandos para detener
          // apaga el oscilador
          noTone(pulsePin);
          // apaga la corriente
          digitalWrite(enablePin, currentDISABLE);

          screen = scrVTSTOP;
          flagProgressScreen = true;
          millisStartScreen = millis();
          break;
      }
      break;
    case scrVTSTOP:
    case scrVTEND:
      loopVTScreenRefresh();
      if (lcd_key == btnSELECT) {
        flagClearScreen = true;
        screen = scrMAIN;
      }
      break;
  }
}

void loopVTScreenRefresh() {
  bool flagChange = flagProgressScreen;
  if ((millis() - millisStartScreen) % (screenCHANGE * 2) > screenCHANGE) {
    flagProgressScreen = false;
    if (flagChange == true) {
      flagClearScreen = true;
      printScreen();
    }
  }
  else {
    flagProgressScreen = true;
    if (flagChange == false) {
      flagClearScreen = true;
      printScreen();
    }
  }

  if (millis() - millisLastRefresh > screenREFRESH) {
    millisLastRefresh = millis();
    if (flagProgressScreen) {
      flagClearScreen = false;
      printScreen();
    }
  }
}

void printVTProgress() {
  lcd.setCursor(0, 0);
  lcd.print(progressVolume, 1);
  lcd.print("/");
  lcd.print(totalVolume);
  lcd.print(" ");
  lcd.write(228);
  lcd.print("L");
  lcd.setCursor(0, 1);
  lcd.print(progressTime, 1);
  lcd.print("/");
  lcd.print(totalTime);
  lcd.print(" s");
}

void printVTStatic() {
  switch (screen) {
    case scrVTPAUSE:
      lcd.setCursor(0, 0);
      lcd.print(" RIGHT TO START ");
      lcd.setCursor(0, 1);
      lcd.print(" LEFT TO CANCEL ");
      break;
    case scrVTRUNNING:
      lcd.setCursor(0, 0);
      lcd.print("     RUNNING   ");
      lcd.setCursor(0, 1);
      lcd.print(" SELECT TO STOP");
      break;
    case scrVTSTOP:
      lcd.setCursor(0, 0);
      lcd.print(" FLOW STOPPED ");
      lcd.setCursor(0, 1);
      lcd.print(" SELECT TO MENU");
      break;
    case scrVTEND:
      lcd.setCursor(0, 0);
      lcd.print(" FLOW FINISHED");
      lcd.setCursor(0, 1);
      lcd.print(" SELECT TO MENU");
      break;
  }
}

void loopValueVT() {
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
    case scrVTVOLUME: auxValue = totalVolume; break;
    case scrVTTIME: auxValue = totalTime; break;
  }
  unsigned long oldValue = auxValue;

  switch (lcd_key) {
    case btnSELECT:
      lcd.clear();
      lcd.noBlink();
      switch (screen) {
        // si esta en VOLUME pasa a TIME
        case scrVTVOLUME:
          lcd.setCursor(0, 0);
          lcd.print("Volume Selected");
          lcd.setCursor(1, 1);
          lcd.print(totalVolume);
          lcd.print(" ");
          lcd.write(228);
          lcd.print("L");
          delay(msgDELAY);
          // recalcula los limites
          minTotalTime = (float) totalVolume / maxFlowRate * 3600.;
          maxTotalTime = (float) totalVolume / minFlowRate * 3600.;
          screen = scrVTTIME;
          return;
          break;
        //si esta en TIME pasa a PAUSE
        case scrVTTIME:
          if (totalTime > minTotalTime & totalTime < maxTotalTime) {
            // Establece la nueva frecuencia
            frequency = (float)totalVolume * syringeLength / syringeVolume / totalTime * calibration / 1000;
            actualFlowrate = frequency * 1000. / calibration / syringeLength * syringeVolume * 3600.0;
            actualTotalTime = totalVolume / actualFlowrate;
            //
            lcd.setCursor(0, 0);
            lcd.print("Time acepted:");
            lcd.setCursor(1, 1);
            lcd.print(totalTime);
            lcd.print(" Seconds");
            delay(msgDELAY);
            // resetea los contadores de progreso
            progressTime = 0;
            progressVolume = 0;
            screen = scrVTPAUSE;
            flagProgressScreen = true;
            millisStartScreen = millis();
          }
          else {
            lcd.setCursor(0, 0);
            lcd.print("Time must be:");
            lcd.setCursor(0, 1);
            lcd.print(minTotalTime, 0);
            lcd.print("<t<");
            lcd.print(maxTotalTime, 0);
            delay(msgDELAY);
            screen = scrVTTIME;
          }
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
    case scrVTVOLUME: totalVolume = auxValue; break;
    case scrVTTIME: totalTime = auxValue; break;
  }
}

void printVTVolume() {
  lcd.setCursor(0, 0);
  lcd.print("Total Volume: ");
  lcd.setCursor(0, 1);
  if (totalVolume < 10) lcd.print("_");
  if (totalVolume < 100) lcd.print("_");
  if (totalVolume < 1000) lcd.print("_");
  if (totalVolume < 10000) lcd.print("_");
  if (totalVolume < 100000) lcd.print("_");

  lcd.print(totalVolume);
  lcd.setCursor(7, 1);
  lcd.write(228);
  lcd.print("L");
}

void printVTTime() {
  lcd.setCursor(0, 0);
  lcd.print("Total Time: ");
  lcd.setCursor(0, 1);
  if (totalTime < 10) lcd.print("_");
  if (totalTime < 100) lcd.print("_");
  if (totalTime < 1000) lcd.print("_");
  if (totalTime < 10000) lcd.print("_");
  if (totalTime < 100000) lcd.print("_");

  lcd.print(totalTime);
  lcd.setCursor(7, 1);
  lcd.print("Seconds");
}
