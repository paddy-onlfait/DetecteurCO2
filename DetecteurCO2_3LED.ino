//---CAPTEUR DE CO2----Novembre 2020 - Tony Vanpoucke (Edulab, Rennes 2)----------------------------//
//                                                                                                  //
//   Petit capteur de CO2 avec affichage 7 segments et triple seuil d'alerte                        //
//   Bibliothèques pré-requises : TM1637Display (Avishay Orpaz, IL) et MHZ19 (Jonathan Dempsey, UK) //
//   Ajout de la gestion des LEDs pour les niveaux d'alerte - p@ddY (fablab On l'fait - Genève)     //
//                                                                                                  //
//--------------------------------------------------------------------------------------------------//

// ----- PARAMETRES MODIFIABLES ----------------------------------//

int SeuilPPM1 = 800;    //1er Seuil d'alerte au CO2 (800 PPM : 1% de l’air dans la pièce a déjà été respiré)
int SeuilPPM2 = 1600;   //2e Seuil d'alerte au CO2 (1600 PPM : 3% de l'air dans la pièce a déjà été respiré)
int SeuilPPM3 = 2400;   //3e Seuil d'alerte au CO2 (2400 PPM : 5% de l'air dans la pièce a déjà été respiré, mauvais air, maux de tetes, malaise possible, ventillation indispensable)

int SensorInterval = 1000;  //Interval de capture du CO2

float luminosite = 2.5;   //luminosité des écrans
int loadTime = 32000;     //temps de "chauffe" du capteur CO2
int loadSpeed = 200;       //modifie la durée de l'animation de chargement
int SpeedFactor = 2;       //ralenti ou acccèlere l'animation
int messageInterval = 4;  //fréquence des messages d'alerte si un seuil CO2 est passé (4 par défaut)

// ----- Variables du programme

int CO2PPM = 0;
int messageBox = 0;

// ----- Bibliothèques utilisées

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TM1637Display.h>
#include "MHZ19.h"

#define BAUDRATE 9600                       // fréquence de fonctionnement du capteur CO2 (ne pas changer)

// ----- Branchements arduino

// Branchement des 3 LEDs: Verte, Jaune, Rouge
#define LEDG 8
#define LEDY 7
#define LEDR 6

// Branchement capteur MHZ16B
#define RX_PIN 10
#define TX_PIN 11

// Branchement ecran 7 segment
#define CLK 4
#define DIO 5

// Raccordement electronique a la Arduino
MHZ19 myMHZ19;
SoftwareSerial mySerial(RX_PIN, TX_PIN);    
TM1637Display displayScreen(CLK, DIO);
unsigned long getDataTimer = 0;

// ----- INITIALISATION ---------------------------------------//

void setup()
{
  // Demarre la communication serie avec la Arduino
  Serial.begin(9600);
  // set the LED pins as outputs
  pinMode(LEDG, OUTPUT);
  pinMode(LEDY, OUTPUT);
  pinMode(LEDR, OUTPUT);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDY , HIGH);
  digitalWrite(LEDR , HIGH);

  // Initialise la lecture du capteur C02
  mySerial.begin(BAUDRATE);
  myMHZ19.begin(mySerial);
  myMHZ19.setFilter(true, true);
  myMHZ19.autoCalibration(true);

  CO2PPM = myMHZ19.getCO2(true, true);                        // Capture de la donnée pour "chauffer" le capteur de CO2

  displayScreen.setBrightness(luminosite);                    // Applique la lumisosité à l'ecran 7 seg

  loadingAnimation();                                         // Lance une animation ecran le temps de "chauffer" la capteur CO2

}

// ----- PROGRAMME ---------------------------------------//

void loop()
{
  // A intervales réguliers capture la donnée et l'affiche :
  if (millis() - getDataTimer >= SensorInterval)
  {
    CO2PPM = myMHZ19.getCO2(true, true);    // Numérisation de la donnée de CO2
    
    Serial.println(CO2PPM);    // affichage du taux de CO2 sur le port série Arduino

    getDataTimer = millis();   // stocker les donnée d'interval

    screenDisplay();           // afficher la donnee sur l'ecran 7 segment (si un seuil est atteint, declanche animation)
  }
}


// --------------------------------------------------------//

void loadingAnimation()
{
  uint8_t loadA[] = { 0x30, 0x00, 0x00, 0x00 };                //animation de chargment 1/5
  uint8_t loadB[] = { 0x06, 0x30, 0x00, 0x00 };                //animation de chargment 2/5
  uint8_t loadC[] = { 0x00, 0x06, 0x30, 0x00 };                //animation de chargment 3/5
  uint8_t loadD[] = { 0x00, 0x00, 0x06, 0x30 };                //animation de chargment 4/5
  uint8_t loadE[] = { 0x00, 0x00, 0x00, 0x06 };                //animation de chargment 5/5
  Serial.println("Initialisation en cours");
  //Affiche l'animation en boucle
  for (int i = 1; i <= loadSpeed; i++) {

    displayScreen.setSegments(loadA);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDY, LOW);
    digitalWrite(LEDR, LOW);
    delay((loadTime / (2*i+loadSpeed)) / SpeedFactor);
    displayScreen.setSegments(loadB);
    //Serial.println((loadTime / (2*i+loadSpeed)) / SpeedFactor);
    delay((loadTime / (2*i+loadSpeed)) / SpeedFactor);
    displayScreen.setSegments(loadC);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDY, HIGH);                         
    delay((loadTime / (2*i+loadSpeed)) / SpeedFactor);
    displayScreen.setSegments(loadD);                          
    //delay((loadTime / loadSpeed) / 5);
    delay((loadTime / (2*i+loadSpeed)) / SpeedFactor);
    displayScreen.setSegments(loadE);
    digitalWrite(LEDY, LOW);
    digitalWrite(LEDR, HIGH);                          
    delay((loadTime / (2*i+loadSpeed)) / SpeedFactor);
  }
}


void screenDisplay()
{
  // Si une des valeurs CO2 se trouve entre le seuil 1 et le seuil 2 :
  
  if ((SeuilPPM1 < CO2PPM) && (SeuilPPM2 > CO2PPM)) { 
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDY, LOW);
    digitalWrite(LEDR, LOW);
    if (messageBox >= messageInterval) {
      uint8_t LowAlert[] = { 0x39, 0x5c, 0x5b, 0x08 }; //Affiche le message d'alerte (Low)
      displayScreen.setSegments(LowAlert);
      messageBox = 0;
    } else {
      displayScreen.showNumberDecEx(CO2PPM, false);    //Affiche les valeurs dans les écrans 7 segments
      messageBox++;
    }

  // Si une des valeurs CO2 se trouve entre le seuil 2 et le seuil 3 :  
  } else if ((SeuilPPM2 < CO2PPM) && (SeuilPPM3 > CO2PPM)) {  
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDY, HIGH);
      digitalWrite(LEDR, LOW);

      if (messageBox >= messageInterval/2) {
        uint8_t MedAlert[] = { 0x39, 0x5c, 0x5b, 0x40 }; //Affiche le message d'alerte (Med)
        displayScreen.setSegments(MedAlert);
        messageBox = 0;
    } else {
        displayScreen.showNumberDecEx(CO2PPM, false);    //Affiche les valeurs dans les écrans 7 segments
        messageBox++;
    }

  // Si une des valeurs CO2 se trouve au dela du seuil 3 : 
  } else if (SeuilPPM3 < CO2PPM) {
      digitalWrite(LEDG, LOW);
      digitalWrite(LEDY, LOW);
      digitalWrite(LEDR, HIGH);
      if (messageBox >= messageInterval/messageInterval) {
        uint8_t HiAlert[] = { 0x39, 0x5c, 0x5b, 0x01 }; //Affiche le message d'alerte (High)
        displayScreen.setSegments(HiAlert);
        messageBox = 0;
    } else {
        displayScreen.showNumberDecEx(CO2PPM, false);    //Affiche les valeurs dans les écrans 7 segments
        messageBox++;
      }

  // Si une des valeurs CO2 se trouve sous le seuil 1 (alors il n'y a pas l'alerte) : 
  } else {
    //digitalWrite(LEDG, HIGH);
    digitalWrite(LEDY, LOW);
    digitalWrite(LEDR, LOW);  
    displayScreen.showNumberDecEx(CO2PPM, false);    //Sinon valeurs dans les écrans 7 segments (sans message d'alerte)
    messageBox = 0;
    //delay(200);
    digitalWrite(LEDG,LOW);
  }
}
