void egg() {
  if (flagEndstopSerial == false) {
    flagEndstopSerial = true;
    byte eggCH[8][8] = {
      { 31, 31, 31, 0,  0, 0,  0, 0 },
      { 28, 30, 31, 31, 31, 31, 31, 31 },
      { 31, 31, 31, 31, 31, 31, 15, 7 },
      {  0, 0,  0, 0,  0, 31, 31, 31 },
      { 31, 31, 31, 31, 31, 31, 30, 28 },
      { 31, 31, 31, 0,  0, 0, 31, 31 },
      {  7, 15, 31, 31, 31, 31, 31, 31 },
      {  31, 31, 31, 31, 31, 31, 31, 31 },
    };
    for (int k = 0; k < 8; k++)
      lcd.createChar(k, eggCH[k]);
    lcd.clear();
    //for (int k = 0; k < 16; k++) lcd.write(k+32);
    byte l1[] = {32, 32, 06, 32, 32, 32, 06, 05, 00, 32, 07, 00, 01, 32, 32};
    byte l2[] = {32, 32, 02, 03, 03, 32, 02, 32, 32, 32, 07, 03, 04, 32, 32};
    lcd.setCursor(0, 0);
    for (int k = 0; k < 16; k++) lcd.write(l1[k]);
    lcd.setCursor(0, 1);
    for (int k = 0; k < 16; k++) lcd.write(l2[k]);
  }
}
