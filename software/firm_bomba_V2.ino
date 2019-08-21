/*  Firmware V2 de la bomba de jeringa  */
/*-------------0123456789012345*/
char firm[] = "Firm V2.0 251018";
/*
   Historial:

   Se implementó paradas por fin de carrera.

   Respecto a V1 se cambia el orden de los menus
   considerando el flujo de trabajo del usuario.

   Se eliminó el modo de velocidad ya que solo es
   útil a fines de calibración.

   La generación de la onda cuadrada se realiza
   mediante la función tone() la cual se basa en los
   timers internos de la placa logrando independencia
   del tiempo de ejecución.

   El sistema se basa en el driver TB6600.

   Adicionalmente en el modo Serial se incorpora la
   capacidad de generar la onda mediante pulsos discretos.
   Esto resulta util a los fines de calibrar el
   conjunto motor-reductor-eje-carro.
*/
// Libreria para el manejo de la pantalla
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#define msgDELAY 1200

//---------------------------------------------------------
// Variables internas para manejar el motor
//---------------------------------------------------------
// Frecuencia de la onda cuadrada, funcion tone()
unsigned int frequency = 1727; // Hz
// minimo debido al metodo de generacion
#define MIN_FREQ 31
// maxima velocidad para que no patine el motor
#define MAX_FREQ 12500

/* Calibracion entre pulsos y avance del carro
   1 periodo / 1 pulso
   32 pulsos / 1 paso  (seteado en TB6600 1-OFF 2-OFF 3-OFF)
   200 pasos / 1 vueltaMotor  (del motor)
   26.851239 vueltaMotor / 1 vueltaEje (del reductor)
   vueltaEje / 8.0 mm (del datashet de la rosca)
   Valor teorico: 21841 +/- 380
*/
// Medido experimentalmente con regla/calibre 21440+/-10
// pulsos por mm (incertidumbre en la cuarta cifra)
unsigned long calibration = 21440;


//This software has been developed by Emanuel Elizalde
//---------------------------------------------------------
// Variables consignas del usuario
//---------------------------------------------------------
// Calibracion: largo de la jeringa en micrones
unsigned long syringeLength  = 58000;
// Calibracion: volumen total de la jeringa en microlitros
unsigned long syringeVolume =  1000;
// Caudal consigna - modo flowrate
unsigned long flowrate = 5000; // microlitros/hora
// Volumen/Tiempo consigna - modo volume-time
unsigned long totalVolume = 500; // microlitros
unsigned long totalTime = 360; // segundos

/* Calculo de la frecuencia
    Modo FlowRate:
      velocity  = flowrate*syringeLength/syringeVolume/3600
      frequency = velocity*calibration/1000
      Inverso:
      flowrate=frequency*1000./calibration*3600.*syringeVolume/syringeLength;
    Modo Volume-Time:
      totalLength  = totalVolume*syringeLength/syringeVolume
      velocity = totalLength/totalTime
      frequency = velocity*calibration/1000
      flowrate=totalVolume/totalTime
*/
float minFlowRate = MIN_FREQ * 1000. / calibration * 3600.*syringeVolume / syringeLength;
// minFlowRate 89 microlitros / hora
float maxFlowRate = MAX_FREQ * 1000. / calibration * 3600.*syringeVolume / syringeLength;
// maxFlowRate 28950 microlitros / hora
float minTotalTime = totalVolume / maxFlowRate * 3600;
// minTotalTime 62 seg  (500 microlitros)
float maxTotalTime = totalVolume / minFlowRate * 3600;
// maxTotalTime 20224 seg (500 microlitros)

//---------------------------------------------------------
// Valores reales y contadores de operacion
//---------------------------------------------------------
/*  Por el metodo de generacion de onda, los valores de
    frecuencia deben quedar limitados a numeros enteros.
    Por este redondeo las variables reales van a diferir
    levemente de las consignas.
*/
float actualFlowrate = frequency * 1000. / calibration / syringeLength * syringeVolume * 3600.0;
float actualTotalTime = totalVolume / actualFlowrate;
// Volumen/Tiempo acumulados en el modo VT
float progressVolume = 0; // microlitros
float progressTime = 0; // segundos

//---------------------------------------------------------
// Variables y defs para el manejo de menus y pantallas
//---------------------------------------------------------
// flag para no imprimir constantemente la pantalla
bool flagClearScreen = true;
unsigned long millisStartScreen = 0;
#define screenCHANGE 1320
unsigned long millisLastRefresh = 0;
#define screenREFRESH 180
bool flagProgressScreen = false;
unsigned short screenRotation = 0;

// Para pantallas donde se elijen valores numericos
short cursorPosition = 1; // 1 unidad 2 decena 3 centena ...

// Eleccion del modo
#define modeFR 0 // Mode Flow-Rate
#define modeVT 1 // Mode Volume-Time
unsigned short mode = modeFR;

// Eleccion de la pantalla
#define scrMAIN      1
#define scrCALLENGTH 2
#define scrCALVOLUME 3
#define scrINFO      4
#define scrSERIAL    5

#define scrFRFLOWRATE 20
#define scrFRPAUSE    21
#define scrFRRUNNING  22
#define scrFRFREQ     23

#define scrVTVOLUME  10
#define scrVTTIME    11
#define scrVTPAUSE   12
#define scrVTRUNNING 13
#define scrVTSTOP    14
#define scrVTEND     15

#define scrENDSTOP   30
unsigned short screen = scrMAIN;

// Menu principal
#define mnuFAST       1
#define mnuCAL        2
#define mnuFLOWRATE   3
#define mnuVOLUMETIME 4
#define mnuSERIAL     5
#define mnuINFO       6
unsigned short mainMenu = mnuFAST;
#define mnuITEMS 6 // numero total de items en el menu

// Variable para el manejo de la botonera
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

//---------------------------------------------------------
// Manejo del motor
//---------------------------------------------------------
// Configuracion de la conexion al driver del motor
const int pulsePin = 49;    // PUL+
#define directionFORWARD LOW
#define directionBACKWARD HIGH
const int directionPin = 51;// DIR+
#define currentENABLE LOW
#define currentDISABLE HIGH
const int enablePin = 53;   // ENA+
// Configuracion del final de carrera de avance
#define endstopPRESSED HIGH
#define endstopFREE LOW
const int endstopPin = 47;   // MICROSWITCH
// TODO: final de carrera de retroceso
// Frecuencia variable escalonada para el modo FAST
unsigned int freqStair;
// Variable que cuenta el tiempo desde el RUN
unsigned long millisStartRuninng;

//---------------------------------------------------------
// Variables para el modo Serial
//---------------------------------------------------------
#define delaySTEPSSERIAL 20
bool flagEndstopSerial = false;
bool flagNumberSteps = false;

void setup() {
  // inicia los pines de control del driver
  pinMode(pulsePin, OUTPUT);
  pinMode(directionPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  // inicia el pin del final de carrera
  pinMode(endstopPin, INPUT);
  // apaga la corriente al motor
  digitalWrite(enablePin, currentDISABLE);

  // Inicializar el LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  // Dibuja una secuencia (falsa) de inicializacion
  lcd.print("Initializing");
  for (int k = 0; k < 16; k++) {
    lcd.setCursor(k, 1);
    lcd.print(".");
    delay(30);
  }
  // primera pantalla
  printScreen();
  Serial.begin(9600);
}

void loop() {
  read_LCD_buttons();
  switch (screen) {
    case scrMAIN:
      loopMain();
      break;
    case scrCALLENGTH:
    case scrCALVOLUME:
      loopCal();
      break;
    case scrSERIAL:
      loopSerial();
      break;
    case scrINFO:
      switch (lcd_key) {
        case btnRIGHT:
          millisStartScreen=millis();
          flagNumberSteps=true;
          break;
        case btnPRESSED:
          if (millis()-millisStartScreen>6000 & flagNumberSteps)
            screen=256;
          break;
        case btnSELECT:
          flagNumberSteps=false;
          screen = scrMAIN;
          break;
      }
      break;

    case scrFRFLOWRATE:
    case scrFRPAUSE:
    case scrFRRUNNING:
    case scrFRFREQ:
      loopFR();
      break;

    case scrVTVOLUME:
    case scrVTTIME:
    case scrVTPAUSE:
    case scrVTRUNNING:
    case scrVTSTOP:
    case scrVTEND:
      loopVT();
      break;

    case scrENDSTOP:
      if ((millis() - millisStartScreen) > screenCHANGE) {
        millisStartScreen = millis();
        //rota la pantalla entre dos valores
        screenRotation = (screenRotation + 1) % 2;
        printScreen();
      }
      switch (lcd_key) {
        case btnSELECT:
          lcd.noBlink();
          flagClearScreen = true;
          screen = scrMAIN;
          break;
      }
      break;
    case 256:
      egg();
      return;
  }
  if (lcd_key != btnNONE & lcd_key != btnPRESSED) {
    printScreen();
  }
}

/*----------------------------------------
  Pantalla y menu principal
  ----------------------------------------*/
void loopMain() {
  switch (lcd_key) {
    case btnSELECT:
      switch (mainMenu) {
        case mnuFAST:
          break;
        case mnuCAL:
          screen = scrCALLENGTH;
          cursorPosition = 4;
          break;
        case mnuFLOWRATE:
          screen = scrFRFLOWRATE;
          cursorPosition = 4;
          break;
        case mnuVOLUMETIME:
          screen = scrVTVOLUME;
          cursorPosition = 3;
          break;
        case mnuSERIAL:
          screen = scrSERIAL;
          Serial.flush();
          break;
        case mnuINFO:
          screen = scrINFO;
          break;
      }
      break;

    case btnRIGHT:
      if (mainMenu == mnuFAST) {
        digitalWrite(enablePin, currentENABLE);
        digitalWrite(directionPin, directionBACKWARD);
        freqStair = 12000; // inicia rampa de velocidad
      }
      break;
    case btnLEFT:
      // solo habilita el avance si el endstop esta libre
      if (mainMenu == mnuFAST & digitalRead(endstopPin) == endstopFREE) {
        digitalWrite(enablePin, currentENABLE);
        digitalWrite(directionPin, directionFORWARD);
        freqStair = 12000; // inicia rampa de velocidad
      }
      break;
    case btnNONE:
      if (mainMenu == mnuFAST) {
        digitalWrite(directionPin, directionFORWARD);
        // detiene la corriente en el motor
        digitalWrite(enablePin, currentDISABLE);
        // apaga el oscilador
        noTone(pulsePin);
      }
      break;
    case btnPRESSED:
      if (mainMenu == mnuFAST) {
        if (pressed_key == btnRIGHT) {
          if (freqStair < 64000) {
            noTone(pulsePin);
            tone(pulsePin, freqStair);
            freqStair += 1000;
            delay(20);
          }
        }
        if (pressed_key == btnLEFT) {
          if (digitalRead(endstopPin) == endstopPRESSED) {
            // detiene al motor y al generador
            digitalWrite(enablePin, currentDISABLE);
            noTone(pulsePin);
            // muestra mensaje
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("ENDSTOP REACHED");
            delay(msgDELAY);
            printScreen();
          }
          else {
            if (freqStair < 64000) {
              noTone(pulsePin);
              tone(pulsePin, freqStair);
              freqStair += 1000;
              delay(20);
            }
          }
        }
      }
      break;
    case btnUP:
      mainMenu--;
      if (mainMenu == 0) mainMenu = mnuITEMS;
      break;
    case btnDOWN:
      mainMenu++;
      if (mainMenu == mnuITEMS + 1) mainMenu = 1;
      break;
  }
}

void printMain() {
  switch (mainMenu) {
    case mnuFAST:
      lcd.setCursor(0, 0);
      lcd.print("1  ");
      lcd.write(127);
      lcd.print("Fast Move");
      lcd.write(126);
      lcd.setCursor(0, 1);
      /*---------0123456789012345*/
      lcd.print("  Displace Cart");
      break;
    case mnuCAL:
      lcd.setCursor(0, 0);
      lcd.print("2 ");
      lcd.print("Calibrate");
      lcd.setCursor(0, 1);
      lcd.print("  Syringe");
      break;
    case mnuFLOWRATE:
      lcd.setCursor(0, 0);
      lcd.print("3 ");
      lcd.print("Enter Mode");
      lcd.setCursor(0, 1);
      /*---------0123456789012345*/
      lcd.print("  Flow-Rate");
      break;
    case mnuVOLUMETIME:
      lcd.setCursor(0, 0);
      lcd.print("4 ");
      lcd.print("Enter Mode");
      lcd.setCursor(0, 1);
      lcd.print("  Volume-Time");
      break;
    case mnuSERIAL:
      lcd.setCursor(0, 0);
      lcd.print("5 ");
      lcd.print("Serial Mode");
      lcd.setCursor(0, 1);
      //---------0123456789012345
      lcd.print("  StartListening");
      break;
    case mnuINFO:
      lcd.setCursor(0, 0);
      lcd.print("6 Info - Version");
      lcd.setCursor(0, 1);
      lcd.print("");
      break;
  }
}

void checkEndstop() {
  if (digitalRead(endstopPin) == endstopPRESSED) {
    noTone(pulsePin);
    digitalWrite(enablePin, currentDISABLE);
    screen = scrENDSTOP;
    lcd.noBlink();
    screenRotation = 0;
    flagClearScreen = true;
    millisStartScreen = millis();
    printScreen();
  }
}
