/*
 *        BUGGY.ino
 * 
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    LIBRAIRIE
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/

// -------- LIBRAIRIE module MCP23017 (Extension d'E / S I2C 16 bits avec interface série) --------
#include <Adafruit_MCP23017.h>

// -------- LIBRAIRIE EMETTEUR RECEPTEUR RADIO --------
// MIRF PERMETTANT LE CONTROLE DU MODULE NRF24L01
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include <MirfSpiDriver.h>


// -------- LIBRAIRIE SERVOMOTEURS --------
#include <Servo.h>
Servo Servomoteur_cam1, Servomoteur_cam2;  // VARIABLE REPRESENTANT DEUX SERVOS MOTEURS.


/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    DENOMINATION DES BROCHES DIGITAL/ANALOGIQUE
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/
// RACCORDEMENT DES COMPOSANTS ET CIRCUITS ELECTRONIQUES AUX BROCHES DE L'ARDUINO

// -------- RACCORDEMENT DES LEDS AUX BROCHES NUMERIQUES --------
// Const int REPRESENTE UNE VARIABLE NOMBRE ENTIER CODE SUR 2 BYTES OU OCTETS : 2 ^ 8 = 256 COMBINAISON
// ON POURRAIT REMPLACER LE TYPE int PAR char DANS LE BUT D'ECONOMISER DE LA MEMOIRE char ETANT CODE SUR 1 BYTES
const int LED_ROUGE = 7;  // LED ROUGE RACCORDE A LA BROCHE NUMERIQUE DE L'ARDUINO NUMERO 7.
const int LED_VERTE = 8;  // LED VERTE RACCORDE A LA BROCHE NUMERIQUE DE L'ARDUINO NUMERO 8.
const int LED_BLEU = 9;   // LED BLEU RACCORDE A LA BROCHE NUMERIQUE DE L'ARDUINO NUMERO 9.

// --------  RACCORDEMENT DES MOTEURS AUX BROCHES DIGITALES  --------
// PWM (MODULATION DE LARGEUR D'IMPLUSION) VARIATION DU RAPPORT CYCLIQUE
// MOTEUR ARRIERE DROITE
int const RPWM_Droite = 4;  // FIL GRIS BROCHE RPWM DU MOTEUR ARRIERE DROITE RACCORDE SUR LA BROCHE 5 PWM DE L'ARDUINO  9.
int const LPWM_Droite = 5;  // FIL BLANC BROCHE LPWM DU MOTEUR ARRIERE DROITE RACCORDE SUR LA BROCHE 5 PWM DE L'ARDUINO  10.
// MOTEUR ARRIERE GAUCHE
int const RPWM_Gauche = 6;  // FIL BLANC BROCHE RPWM DU MOTEUR ARRIERE GAUCHE RACCORDE SUR LA BROCHE 5 PWM DE L'ARDUINO  5.
int const LPWM_Gauche = 7;  // FIL GRIS BROCHE LPWM DU MOTEUR ARRIERE GAUCHE RACCORDE SUR LA BROCHE 5 PWM DE L'ARDUINO  6.


/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    DEFINITION DES STRUCTURES
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/
// ------- DEFINIT LA DIRECTION -------
typedef struct {
  int X = 0;
  int Y = 0;
} Repere_Ortho;
Repere_Ortho Direction;
Repere_Ortho Direction_Servo;

// ------- STOCKER LES VALEURS DE POSITION DES JOYSTICKS ENVOYE PAR LE JOYPAD VIA L'EMETTEUR RADIO -------
typedef struct {
  int X_G;
  int Y_G;
  int SW_G;
  int X_D;
  int Y_D;
  int SW_D;
  byte bouton_Haut;
  byte bouton_Bas;
  byte bouton_Gauche;
  byte bouton_Droite;
} MaStructure;
MaStructure joystick;
// 16 Octets = ( valeurs int 2 octets X 4 = 12 octets  + valeurs type byte X 4 = 4 octets)
byte taille_message = sizeof(MaStructure); 


/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    INITIALISATION DES VARIABLES
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/
// -------- STOCKE LA POSITION DES SERVOMOTEURS AU DEMARRAGE --------
int message = 0;
int valueX = 80;
int valueY = 80;
int indiceDeplac = 10;


/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    CREATION D'UNE TEMPORISATION
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/
// ETAT D'ACTIVATION DE LA TEMPORISATION
//int tempoActive = 0;
// TEMPS A L'ACTIVATION DE LA TEMPORISATION
unsigned long previousMillis = 0;
// INTERVAL DE TEMPS (milliseconds)
const long interval = 1000;


/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    MODE D'ENTRAINEMENT DES DEUX MOTEURS DC
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/
/*
 ETAPE 1: RECUPERER LES VALEURS DEFINISSANT LA POSITION DES JOYSTICK VIA LE RECEPTEUR RADIO;
 ETAPE 2: DEFINIR LA DIRECTION EN DEFINISSANT 8 POSITION + 1 POINT MORT
 ETAPE 3: DEFINIR 8 + 1 ACTION EN FONCTION DE LA DIRECTION ET APPLIQUER UNE VITESSE


                                                     DETAIL
 -  ETAPE 1  -
 POSITION JOYSTICK ENVOYE VIA LE JOYPAD
                                                       (-)
                                                  (Axe des  X)
                                                        0
                                                        |
                                                        |
                                                        |
                                                        |
                                                        |
         (Axe des  Y)(+) 1023 --------------------(Point_Mort)------------------------ 0  (-)
                                                        |
                                                        |
                                                        |
                                                        |
                                                      1023
                                                       (+)

 -  ETAPE 2  -
 DEFINITION DE 8 POSITION POUR LE DEPLACEMENT + 1 POSITION POUR LE POINT MORT
 RAPPORT AU VALEUR JOYSTICK RENVOYE DONT LE MONTAGE EST INVERSE SUR TELECOMMANDE
 COUPLE (Y,X)
                            Axe Y
         1  <---------------- 0 ------------->  -1
         |  (1,1)           (0,1)       (-1,1)
         |
  Axe X  0  (1,0)           (0,0)       (-1,0)
         |
         |  (1,-1)         (0,-1)      (-1,-1)
        -1


 -  ETAPE 3  -
 ON OBTIENT UN COUPLE DE VALEUR (Y;X) QUI CORRESPOND AU TABLEAU
 CI DESSUS ET QU'IL SUFFIT DE TESTER POUR DETERMINER L'ACTION A REALISER
*/


/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    CONFIGURATION
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/
void setup() {
  Serial.begin(9600);
  //Serial.setTimeout(50);
  Serial.println("Démarrage ...");


  // -------- CONFIGURATON DES BROCHES POUR LES SERVOMOTEURS. --------
  // - ROUGE A RACCORDER  A LA BORNE + DE L'ALIMENTATION 5 VOLTS DE L'ARDUINO.
  // - NOIR A RACCORDER  A LA BORNE - (LA MASSE) DE L'ALIMENTATION  DE L'ARDUINO.
  // - JAUNE A RACCORDER  A UNE BROCHE PWM DE L'ARDUINO VALEUR DE 0 A 1023 ( MODULATION A LARGEUR D'IMPULSION ).
  Servomoteur_cam1.attach(2);      // FIL JAUNE  DU PREMIER SERVO MOTEUR SUR LEQUEL EST FIXE LE DEUXIEME SERVO MOTEUR ET RACCORDE A LA BROCHE PWM NUMERO 11.
  Servomoteur_cam2.attach(3);      // FIL JAUNE DU DEUXIEME SERVO MOTEUR  SUR LEQUEL EST FIXE LA CAMERA INFRA ROUGE ET RACCORDE A LA BROCHE PWM NUMERO 12.

   
// -------- PARAMETRE CAMERA (DEPLACEMENT DE 0  A 180) --------
  Servomoteur_cam1.write(valueY);       // DEPLACEMENT DU PREMIER SERVO MOTEUR A 70 DEGRES.
  Servomoteur_cam2.write(valueX);       // DEPLACEMENT DU SECOND SERVO MOTEUR A 70 DEGRES.
  delay(200);
  Serial.println("Configuration servomoteur : OK !");
  
  // -------- CONFIGURATION DES LED --------
  pinMode(LED_ROUGE, OUTPUT);
  pinMode(LED_VERTE, OUTPUT);
  pinMode(LED_BLEU, OUTPUT);
  Serial.println("Configuration LED : OK !");

  
  // -------- CONFIGURATION DES MOTEURS D'ENTRAINEMENTS DES ROUES --------
  pinMode(RPWM_Gauche, OUTPUT);
  pinMode(LPWM_Gauche, OUTPUT);
  pinMode(RPWM_Droite, OUTPUT);
  pinMode(LPWM_Droite, OUTPUT);
  Serial.println("Configuration moteurs : OK !");

/*
   -------- CONFIGURATION DE L'EMETTEUR RECEPTTEUR RADIO NRF24L01 --------
  Brochage pour l'emetteur et le recpteur MODULE NRF24L04
  Broches utilisees pour le MEGA 2560
          MISO -> 50 (violet)       * MOSI -> 51 (jaune)       * SCK -> 52 (orange)
          CE -> 48 (vert)           * CSN -> 49 (bleu)
          GND -> GND                * VCC -> 3


  Broches utilisees pour le UNO
          MISO -> 12 (gris)         * MOSI -> 11 (bleu)        * SCK -> 13 (violet)
          CE -> 8 (vert)            * CSN -> 7 (jaune)
          GND -> GND                * VCC -> 3.3v
  */

  // Configuration des broches CSN et CE :
  Mirf.cePin = 48;    // vert
  Mirf.csnPin = 49;   // bleu
  // configuration du SPI : utiliser le port SPI hardware
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init(); // initialisation du module
  // canal et longueur utile doivent etre identiques
  // pour le client et le serveur
  Mirf.channel = 12;
  Mirf.payload = 16; // taille utile des donnees transmises
  // Configuration des adresses de reception et d'emission
  Mirf.setRADDR((byte *)"clie1"); // adresse de reception du module (de 5 octets)
  Mirf.setTADDR((byte *)"serv1"); // adresse vers laquelle on transmet (de 5 octets)
  Mirf.config(); // ecriture de la configuration
  Serial.println("Configuration de l'émétteur récepteur radio : OK !");


  // -------- AFFICHE LE MESSAGE DANS LE MONITEUR SERIE --------
  Serial.println("PRET POUR RECEPTION DES DONNEES !");
}


/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    DEMARRGE DU PROGRAMME
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/
void loop() {
   Commande_Radio();
   controle_interface_web(); 
}


/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    COMMUNICATION PAR RADIO EMETTEUR
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/

void Commande_Radio(void) {
  
   // -------- ON BOUCLE TANT QUE LE MESSAGE N'A PAS ETE RECU --------
 // while (!Mirf.dataReady()) {
 // }

  // -------- ON VERFIE SI UN MESSAGE A ETE RECU --------
  if( Mirf.dataReady() ){
    // -------- RECUPERE MES DONNEES ENVOYER PAR L'EMETTEUR  --------
    Mirf.getData((byte *)&joystick); // Réception du paquet
    
    // Affiche le message envoyé par la télécommande  dans le moniteur serie
    //monitoring_joystick(); 
  
     /*----------------------------------------
     *          MOBILITEE DE LA CAMERA
     ----------------------------------------*/
    // DEPLACEMENT DE LA CAMERA.
    Direction_Servomoteur();
    // ENVOIE LES PARAMETRES AUX SERVO-MOTEURS
    deplacement_Servo_Cam();
  
    /*----------------------------------------
     *          ENTRAINEMENT DES ROUES
     ----------------------------------------*/
    Direction_Moteur();
    // ENVOIE LES PARAMETRE AU MOTEURS
    speed_rotate_motor();
  }
}

void monitoring_joystick(void){
  Serial.print("Message reçu: ");
  Serial.print("moteur axe X : ");
  Serial.print(joystick.X_G);
  Serial.print(" , moteur axe Y : ");
  Serial.print(joystick.Y_G);
  Serial.print(" , moteur bouton  : ");
  Serial.print(joystick.SW_G);
  Serial.print(" ,  Camera axe X : ");
  Serial.print(joystick.X_D);
  Serial.print(" , Camera axe Y : ");
  Serial.print(joystick.Y_D);
  Serial.print(" , Camera bouton : ");
  Serial.print(joystick.SW_D);
  Serial.print(" , Bouton haut: ");
  Serial.print(joystick.bouton_Haut);
  Serial.print(" , Bouton bas: ");
  Serial.print(joystick.bouton_Bas);
  Serial.print(" , Bouton gauche: ");
  Serial.print(joystick.bouton_Gauche);
  Serial.print(" , Bouton droite: ");
  Serial.println(joystick.bouton_Droite);
}

/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    GESTION DES MOTEURS DC
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/
/*
 ***********************************************************************
 DIRECTION DES MOTEURS : POINT MORT, AVANT, ARRIERE, DROITE, GAUCHE. 
 ***********************************************************************
*/
void Direction_Moteur(void) {
  
  // On obtient un couple de valeur  ( Direction.X , Direction.Y )

  // POINT MORT = VALEUR X ET Y A 0
  Direction.X = 0;
  Direction.Y = 0;

  // DIRECTION ARRIERE VALEUR RENVOYES DE -255 A -1.
  if (joystick.X_G < 0) {
    Direction.X = -1;
  }
  // DIRECTION AVANT VALEUR RENVOYES DE 1 A 255.
  else if (joystick.X_G > 0) {
    Direction.X = 1;
  }

  // DIRECTION DROITE VALEUR RENVOYES DE 1 A 255.
  if (joystick.Y_G > 0) {
    Direction.Y = -1;
  }
  // DIRECTION GAUCHE VALEUR RENVOYES DE -255 A -1.
  else if (joystick.Y_G < 0) {
    Direction.Y = 1;
  }
}
/*
 ******************************************************************************
 DEFINI LE SENS ET LA VITESSE DE ROTATION DES MOTEURS
 DEFINITION DE 8 POSITION POUR LE DEPLACEMENT + 1 POSITION POUR LE POINT MORT
 ******************************************************************************
*/
void speed_rotate_motor(void) {
  // RAPPORT AU VALEUR JOYSTICK RENVOYE DONT LE MONTAGE EST INVERSE SUR TELECOMMANDE
  // COUPLE (Y,X)
  //                      Y
  //   1 <--------- ----- 0 -------------> -1
  //   |    (1,1)       (0,1)        (-1,1)
  //   |
  // X 0    (1,0)       (0,0)        (-1,0)
  //   |
  //   |   (1,-1)      (0,-1)        (-1,-1)
  //  -1

  float variateur_vitesse = 0.75;
  
  Serial.print(" - MOTEURS : ");

  if (Direction.Y == 0) { 
  
    // --------- POINT MORT : couple:(0;0) ---------
    if (Direction.X == 0) {
      Serial.print("POINT MORT");
      Actionneur_Moteur(0, 0, 0, 0);
    }
    
    // --------- AVANCER : couple:(0;1) ---------
    if (Direction.X == 1) { 
      Serial.print("AVANCER");
      Actionneur_Moteur(0, joystick.X_G, joystick.X_G, 0);
    }
    
    // --------- RECULER : couple:(0;-1) ---------
    if (Direction.X == -1) {
      Serial.print("RECULER");
      Actionneur_Moteur(-joystick.X_G, 0, 0, -joystick.X_G);
    }
 }

  if (Direction.Y == 1) {

    // --------- GAUCHE : couple:(1;0) ---------
    if (Direction.X == 0) { // couple: (1;0)
      Serial.print("GAUCHE");
      Actionneur_Moteur(-joystick.Y_G, 0, -joystick.Y_G, 0);
    }
    
    // --------- AVANT GAUCHE : couple:(1;1) ---------
    if (Direction.X == 1) {
      Serial.print("AVANT GAUCHE");
      //  DEFINI LA VITESSE LA PLUS GRANDE
      int  vitesse_max;
      if (joystick.X_G > joystick.Y_G) {
        // la plus grande
        vitesse_max = joystick.X_G;
      }
      else {
        vitesse_max =  joystick.Y_G;
      }
      int vitesse_mini = vitesse_max * (float)variateur_vitesse;
      Actionneur_Moteur(0, vitesse_mini, vitesse_max, 0);
    }
    
    // --------- ARRIERE GAUCHE : couple:(1;-1) ---------
    if (Direction.X == -1) {
      Serial.print("ARRIERE GAUCHE");
      //  DEFINI LA VITESSE LA PLUS GRANDE

      int  vitesse_max;
      if (-joystick.X_G > -joystick.Y_G) {
        // la plus grande
        vitesse_max = -joystick.X_G;
      }
      else {
        vitesse_max = -joystick.Y_G;
      }
      int vitesse_mini = vitesse_max * (float)variateur_vitesse;
      Actionneur_Moteur(vitesse_mini, 0, 0, vitesse_max);
    }
  }

  if (Direction.Y == -1) {
    
    // --------- DROITE : couple:(-1;0) ---------
    if (Direction.X == 0) {
      Serial.print("DROITE");
      Actionneur_Moteur(0, joystick.Y_G, 0, joystick.Y_G);
    }
    
    // --------- AVANT DROITE : couple:(-1;1) ---------
    if (Direction.X == 1) {
      Serial.print("AVANT DROITE : ");
      //  DEFINI LA VITESSE LA PLUS GRANDE
      int  vitesse_max;
      if (joystick.X_G > -joystick.Y_G) {
        // la plus grande
        vitesse_max = joystick.X_G;
      }
      else {
        vitesse_max = -joystick.Y_G;
      }
      int vitesse_mini = vitesse_max * (float)variateur_vitesse;
      Actionneur_Moteur(0, vitesse_max, vitesse_mini, 0);
    }
    
    // --------- ARRIERE DROITE : couple:(-1;-1) ---------
    if (Direction.X == -1) {
      Serial.print("ARRIERE DROITE");
      //  DEFINI LA VITESSE LA PLUS GRANDE
      int  vitesse_max;
      if (-joystick.X_G > joystick.Y_G) {
        // la plus grande
        vitesse_max = -joystick.X_G;
      }
      else {
        vitesse_max = joystick.Y_G;
      }
      int vitesse_mini = vitesse_max * (float)variateur_vitesse;
      Actionneur_Moteur(vitesse_max, 0, 0, vitesse_mini);
    }
  }
}

/**********************************************************************
 * 
 *        GESTION DE VITESSE DE ROTATION DES DES MOTEURS (PWM)
 * 
 *********************************************************************/

void Actionneur_Moteur(int LPWM_Ar_G, int RPWM_Ar_G, int LPWM_Ar_D, int RPWM_Ar_D) {
 // Moteur droit
 analogWrite(LPWM_Droite, LPWM_Ar_D); // En arrière moteur droite
 analogWrite(RPWM_Droite, RPWM_Ar_D); // En avance moteur droite
 // Moteur gauche
 analogWrite(LPWM_Gauche, LPWM_Ar_G); // En arrière moteur gauche
 analogWrite(RPWM_Gauche, RPWM_Ar_G); // En avance moteur gauche
 
 //Afficher dans le monietur serie
 monitoringMoteur(LPWM_Ar_G, RPWM_Ar_G, LPWM_Ar_D, RPWM_Ar_D);
}

void monitoringMoteur(int LPWM_Ar_G, int RPWM_Ar_G, int LPWM_Ar_D, int RPWM_Ar_D){
  Serial.print(" : ");
  Serial.print(LPWM_Ar_G);
  Serial.print(" , ");
  Serial.print(RPWM_Ar_G);
  Serial.print(" , ");
  Serial.print(LPWM_Ar_D);
  Serial.print(" , ");
  Serial.println(RPWM_Ar_D);
}


/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    GESTION DES SERVOMOTEURS
  ::::::::::
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/
// On obtient un couple de valeur ( Direction_Servo.X , Direction_Servo.Y ) qui correspond au tableau
// ci-dessus et qu'il suffit de tester pour determiner l'action a  realiser.
void Direction_Servomoteur(void) {
  // POINT MORT INITIALISE LES VALEUR A 0
  Direction_Servo.X = 0;
  Direction_Servo.Y = 0;

  // DIRECTION ARRIERE VALEUR RENVOYES DE -255 A 0-1.
  if (joystick.X_D < 0) {
    Direction_Servo.X = -1;
  }
  // DIRECTION AVANT VALEUR RENVOYES DE 1 A 255.
  else if (joystick.X_D > 0) {
    Direction_Servo.X = 1;
  }
  // DIRECTION DROITE VALEUR RENVOYES DE 1 A 255.
  if (joystick.Y_D > 0) {
    Direction_Servo.Y = -1;
  }
  // DIRECTION GAUCHE VALEUR RENVOYES DE -255 A -1.
  else if (joystick.Y_D < 0) {
    Direction_Servo.Y = 1;
  }
}

void deplacement_Servo_Cam(void) {
  // DEFINITION DE 8 POSITION POUR LE DEPLACEMENT + 1 POSITION POUR LE POINT MORT
  // RAPPORT AU VALEUR JOYSTICK RENVOYE DONT LE MONTAGE EST INVERSE SUR TELECOMMANDE
  // COUPLE (Y,X)
  //                      Y
  //   1 <--------- ----- 0 -------------> -1
  //   |    (1,1)       (0,1)        (-1,1)
  //   |
  // X 0    (1,0)       (0,0)        (-1,0)
  //   |
  //   |   (1,-1)      (0,-1)        (-1,-1)
  //  -1

  Serial.print("CAMERA : ");
  
  if (Direction_Servo.Y == 0) {
    if (Direction_Servo.X == 0) { // couple: (0;0)
      // POINT MORT
      Serial.print("CENTRE");
      Servomoteur_cam1.write(80);
      Servomoteur_cam2.write(90);
    }
    if (Direction_Servo.X == 1) { // couple: (0;1)
      // REGARDER EN HAUT
      Serial.print("HAUT ");
      int value = map( joystick.X_D, 0, 255, 90, 0);
      Servomoteur_cam1.write(value);
    }
    if (Direction_Servo.X == -1) { // couple: (0;-1)
      // REGARDER EN BAS
      Serial.print("BAS");
      int value = map( -joystick.X_D, 0, 255, 90, 180);
      Servomoteur_cam1.write(value);
    }
  }
  if (Direction_Servo.Y == 1) {
    if (Direction_Servo.X == 0) { // couple: (1;0)
      // REGARDER A GAUCHE
      Serial.print("GAUCHE");
      int value = map( -joystick.Y_D, 0, 255, 90, 180);
      Servomoteur_cam2.write(value);
    }
    if (Direction_Servo.X == 1) { // couple: (1;1)
      //REGARDER EN HAUT A GAUCHE
      Serial.print("HAUT A GAUCHE");
      int value = map( joystick.X_D, 0, 255, 90, 0);
      Servomoteur_cam1.write(value);
      int value1 = map( -joystick.Y_D, 0, 255, 90, 180);
      Servomoteur_cam2.write(value1);
    }
    if (Direction_Servo.X == -1) { // couple: (1;-1)
      // REGARDER EN BAS A GAUCHE
      Serial.print("BAS A GAUCHE");
      int value = map( -joystick.X_D, 0, 255, 90, 180);
      Servomoteur_cam1.write(value);
      int value1 = map( -joystick.Y_D, 0, 255, 90, 180);
      Servomoteur_cam2.write(value1);
    }
  }
  if (Direction_Servo.Y == -1) {
    if (Direction_Servo.X == 0) { // couple: (-1;0)
      // REGARDER A DROITE
      Serial.print("DROITE");
      int value = map( joystick.Y_D, 0, 255, 90, 0);
      Servomoteur_cam2.write(value);
    }
    if (Direction_Servo.X == 1) { // couple: (-1;1)
      // REGARDER EN HAUT A DROITE
      Serial.print("HAUT A DROITE");
      int value = map( joystick.X_D, 0, 255, 90, 0);
      Servomoteur_cam1.write(value);
      int value1 = map( joystick.Y_D, 0, 255, 90, 0);
      Servomoteur_cam2.write(value1);
    }
    if (Direction_Servo.X == -1) { // couple: (-1;-1)
      // REGARDER EN BAS A DROITE
      Serial.print("BAS A DROITE");
      int value = map( -joystick.X_D, 0, 255, 90, 180);
      Servomoteur_cam1.write(value);
      int value1 = map( joystick.Y_D, 0, 255, 90, 0);
      Servomoteur_cam2.write(value1);
    }
  }
}


/*
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ::::::::::
  ::::::::::    COMMUNICATION PORT SERIE CONTROLE PAR INTERFACE GRAPHIQUE
  ::::::::::        DEPLACEMENT DE LA CAMERA VIA LES SERVOMOTEURS
  :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/
void controle_interface_web(void) {
  /*
   * Caméra : CamUp, CamDown, CamLeft, CamCenter, CamRight, CamScanX, CamScanY
   * Déplacement du véhicule : MotorsStop,MotorsForward,MotorsBackward,MotorsLeft,
   * MotorsFrontLeft,MotorsBackLeft,MotorsRight,MotorsFrontRight,MotorsBackRight
   */
  // Wait for string
  if (Serial.available()) 
  {
    //lit les caractères d'un flux dans une chaîne. La fonction se termine si le caractère de fin est détecté ou expire (voir setTimeout ()).
    String dataAction = Serial.readStringUntil(';');
    String dataSpeed= Serial.readStringUntil('\n');
    
    Serial.print("Action requise par le client  : ");
    if( dataSpeed != "" ){
      Serial.print(dataAction);
      Serial.print(" ; ");
      Serial.println(dataSpeed);
    } else {
      Serial.println(dataAction);
    }

  
    // Calibre de la vitesse pour tourner coef : 1.275
    int speed_max, speed_min; 
      if (dataSpeed != ""){
        speed_max = dataSpeed.toInt();
        float cInt = speed_max / 1.275;
        speed_min = (int)cInt;
      }
    
    // TRAITEMENT DES DONNEES RECUES.
    Serial.print("Traitement et exécution de la commande : ");
    
    if (dataAction == "CamUp"){ 
      Serial.println("caméra : vers le haut");
      deplacHaut();
    }
    else if (dataAction == "CamDown"){ 
      Serial.println("caméra : vers le bas");
      deplacBas();
    }
    else if (dataAction == "CamLeft"){ 
      Serial.println("caméra : vers la gauche");
      deplacGauche();
    }
    else if (dataAction == "CamCenter"){
      Serial.println("caméra : centré");
      deplacCentre();
    }
    else if (dataAction == "CamRight"){
      Serial.println("caméra : vers la droite");
      deplacDroite();
    }
    else if (dataAction == "CamScanX"){
      Serial.println("caméra : scan sur l'axe des X");
      scanX();
    }
    else if (dataAction == "CamScanY"){
      Serial.println("caméra : scan sur l'axe des Y");
      scanY();
    }
    else if (dataAction == "MotorsStop"){
      Serial.println("moteurs : arrêtés");
      Actionneur_Moteur(0, 0, 0, 0);
    }
    else if (dataAction == "MotorsForward"){
      Serial.println("moteurs : en avant");
      Actionneur_Moteur(0, speed_max, speed_max, 0);
    }
    else if (dataAction == "MotorsBackward"){
      Serial.println("moteurs : en arrère");
      Actionneur_Moteur(speed_max, 0, 0, speed_max);
    }
    else if (dataAction == "MotorsLeft") 
    {
      Serial.println("moteurs : tourner à gauche");
      Actionneur_Moteur(speed_max, 0, speed_max, 0);
    }
    else if (dataAction == "MotorsFrontLeft"){
      Serial.println("moteurs : tourner avant gauche");
      Actionneur_Moteur(0, speed_min, speed_max, 0);
    }
    else if (dataAction == "MotorsBackLeft"){
      Serial.println("moteurs : tourner vers l'arrière gauche");
      Actionneur_Moteur(speed_min, 0, 0, speed_max);
    }
    else if (dataAction == "MotorsRight"){
      Serial.println("moteurs : tourner à droite");
      Actionneur_Moteur(0, speed_max, 0, speed_max);
    }
    else if (dataAction == "MotorsFrontRight"){
      Serial.println("moteurs : tourner vers l'avant gauche");
      Actionneur_Moteur(0, speed_max, speed_min, 0);
    }
    else if (dataAction == "MotorsBackRight"){
      Serial.println("moteurs : tourner vers l'arrière droite");
      Actionneur_Moteur(speed_max, 0, 0, speed_min);
    }
    else
    {
    Serial.print(dataAction);
    Serial.println(" : n'est pas une action reconnue !");
    }
  }
}

// :::::::::::::::::::::::::::::::::::
// SCAN DE LA CAMERA SUR L'AXE DES X
void scanX(void) {
  // Déplacement vers la gauche
  indiceDeplac = 1;
  while (valueX - indiceDeplac >= 0) {
    deplacDroite();
    delay(50);
  }
  // Déplacement de la droite
  while (valueX + indiceDeplac <= 180) {
    deplacGauche();
    delay(50);
  }
  // Déplacement de la caméra de la gauche vers le centre
  while (valueX - indiceDeplac >= 70) {
    deplacDroite();
    delay(50);
  }
  indiceDeplac = 20;
}

// :::::::::::::::::::::::::::::::::::
// SCAN DE LA CAMERA SUR L'AXE DES Y
void scanY(void) {
  // Déplacement vers le haut
  indiceDeplac = 1;
  while (valueY - indiceDeplac >= 0) {
    deplacHaut();
    delay(50);
  }

  // Déplacement vers le bas
  while (valueY + indiceDeplac <= 180) {
    deplacBas();
    delay(50);
  }

  // Déplacement du haut vers le centre
  while (valueY - indiceDeplac >= 70) {
    deplacHaut();
    delay(50);
  }
  indiceDeplac = 20;
}

// :::::::::::::::::::::::::::::::::::
// DEPLACEMENT CAMERA VERS LA GAUCHE
void deplacGauche(void) {
  if (valueX + indiceDeplac <= 180) {
    valueX = valueX + indiceDeplac ;
    Servomoteur_cam2.write(valueX);
  }
}

// :::::::::::::::::::::::::::::::::::
// DEPLACEMENT CAMERA VERS LA DROITE
void deplacDroite(void) {
  if (valueX - indiceDeplac >= 0) {
    valueX = valueX - indiceDeplac ;
    Servomoteur_cam2.write(valueX);
  }
}

// :::::::::::::::::::::::::::::::::::
// POSITIONNER LA CAMERA AU CENTRE
void deplacCentre(void) {
  valueX = 80;
  valueY = 80;
  Servomoteur_cam1.write(valueY);
  Servomoteur_cam2.write(valueX);
}

// :::::::::::::::::::::::::::::::::::
// DEPLACEMENT DE LA CAMERA HAUT GAUCHE
void deplacHaut(void) {
  if (valueY - indiceDeplac >= 0) {
    valueY = valueY - indiceDeplac ;
    Servomoteur_cam1.write(valueY);
  }
}

// :::::::::::::::::::::::::::::::::::
// DEPLACEMENT DE LA CAMERA VERS LE BAS
void deplacBas(void) {
  if (valueY + indiceDeplac <= 180) {
    valueY = valueY + indiceDeplac ;
    Servomoteur_cam1.write(valueY);
  }
}
