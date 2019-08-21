/* Función de impresion de la pantalla */
void printScreen() {

  if (flagClearScreen) {
    lcd.clear();
  }

  switch (screen) {
    case scrMAIN:
      printMain();
      break;

    case scrCALLENGTH:
      printCalLength();
      break;

    case scrCALVOLUME:
      printCalVolume();
      break;

    case scrSERIAL:
      lcd.setCursor(0, 0);
      lcd.print("Listening Serial");
      lcd.setCursor(0, 1);
      lcd.print("---");
      break;

    case scrINFO:
      lcd.setCursor(0, 0);
      lcd.print(firm);
      lcd.setCursor(0, 1);
      lcd.print("LFD FIUBA");
      break;

    case scrENDSTOP:
      switch (screenRotation) {
        case 0:
          lcd.setCursor(0, 0);
          lcd.print("Flow Aborted");
          lcd.setCursor(0, 1);
          lcd.print("Endstop Reached");
          break;
        case 1:
          lcd.setCursor(0, 0);
          /*---------0123456789012345*/
          lcd.print("Press SELECT to");
          lcd.setCursor(0, 1);
          lcd.print("return Main Menu");
          break;
      }
      break;

    case scrFRFLOWRATE:
      printFRFlowRate();
      break;
    case scrFRPAUSE:
      printFRPause();
      break;
    case scrFRRUNNING:
      printFRRunning();
      break;
    case scrFRFREQ:
      printFRFrequency();
      break;

    case scrVTVOLUME:
      printVTVolume();
      break;
    case scrVTTIME:
      printVTTime();
      break;

    case scrVTPAUSE:
    case scrVTRUNNING:
    case scrVTSTOP:
    case scrVTEND:
      if (flagProgressScreen)
        printVTProgress();
      else
        printVTStatic();
      break;
  }
}
