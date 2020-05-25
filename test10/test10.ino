
 
// --------------------------------------------------------partie setup-------------------------------------------------------//
#include <Wire.h>
#include <Arduino.h>
#include "ht1632.h"
#include <avr/pgmspace.h>
#include <TimerOne.h>

// SETUP ============================================
// Set up speaker on a PWM pin (digital 9, 10 or 11)
#define BUZZER 3
int DEBUG = 0;  //Pour débugger la musique, 1 pour l'activer, 2 pour le désactiver

void setup() {
  ht1632_setup();                       // initialise the wire library and hardware  
  Serial.begin(9600);
  Wire.begin();
  setup7Seg();    
  cls();
  pinMode(BUZZER, OUTPUT);
   

  Timer1.attachInterrupt(fnPeriodique,25000);//appelle la fonction periodique toute les 25ms

  }
  // --------------------------------------------------------partie déclaration-------------------------------------------------------//
  
unsigned int compte=0;
unsigned int vitesse=350;
byte etatInitialisation=0;
byte balleLancee=0;
byte changementBalle=0;
byte changementRaquette=0;
byte changementBonus=0;
byte changementBrique=0;
byte changement7seg=1;
byte chargementMap=0; //permet de charger la map voulue
const int nombreBriquePremiereLigne=10;
const int nombreBriqueDeuxiemeLigne=8;
const int nombreBriqueTotal=nombreBriquePremiereLigne+nombreBriqueDeuxiemeLigne;
const int nombreBriquePremiereLigne2=10;
const int nombreBriqueDeuxiemeLigne2=10;
const int nombreBriqueTroisiemeLigne2=4;
const int nombreBriqueQuatriemeLigne2=4;
const int nombreBriqueTotal2=nombreBriquePremiereLigne+nombreBriqueDeuxiemeLigne+nombreBriqueQuatriemeLigne2;
const int maxBrique=35;
const int maxBalle=3;
bool victoire;


#define Y_pin 1   //entrées joystick
#define X_pin 0

#define augLargeur 1
#define balleBonus 2
#define laser 3

typedef struct{
  byte jogPosX;
  byte largeur;
  byte jogCouleur;
  } T_raquette;

T_raquette raquetteJoueur= {NULL, NULL, ORANGE };


typedef struct {
  byte posX; // byte car ne peut être qu’entre 0 et 31
  byte posY; 
  byte lastPosX; 
  byte lastPosY; 
  int deltaX; // char car peut être négatif !!
  int deltaY; 
  byte vie; // Nombre de vie 
} T_balle; 

T_balle balle1={NULL, NULL, NULL, NULL, 0, 0, 4};
T_balle tableauBalle[maxBrique];

typedef struct{
  byte vieBrique; //pour savoir si vivant ou non et la couleur et le nombre de coup pour les tomber
  byte coteBrique;
  byte briquePosX;
  byte briquePosY;
  }T_brique;

T_brique tableauBrique[maxBrique];

typedef struct {
  byte posX; // byte car ne peut être qu’entre 0 et 31
  byte posY; 
  byte lastPosX; 
  byte lastPosY; 
  byte bonusType;
  int duree; 
} T_bonus; 
T_bonus bonus1;


// --------------------------------------------------------partie balle-------------------------------------------------------//

int lancerBalle(int DeltaX, int DeltaY){
   if(analogRead(Y_pin)>600){ 
      if(DeltaX==0 and DeltaY==0){
        randomSeed(analogRead(A3));
        //DeltaX=random(-1,2);
        DeltaX=1;
        //DeltaY=random(1, 3); le jeu marche pas très bien avec Y=2
        balle1.deltaX=DeltaX;
        balle1.deltaY=1;
        balleLancee=1;
      }
   }
  }
  
void deplacerBalle(int DeltaX, int DeltaY, byte PositionX, byte PositionY, byte lastPositionX, byte lastPositionY){
 /*lastPositionX=PositionX;
 lastPositionY=PositionY;*/
  
  balle1.lastPosX=balle1.posX;
  balle1.lastPosY=balle1.posY;
  balle1.posX=balle1.posX-DeltaX;
  balle1.posY=balle1.posY-DeltaY;
  changementBalle=1;
  }


void effacerEtAfficherBalle(byte lastPositionX, byte lastPositionY, byte positionX, byte positionY){
  ht1632_plot(lastPositionX,lastPositionY,LOW);
  ht1632_plot(positionX,positionY,GREEN);
  }

// --------------------------------------------------------partie compteur-------------------------------------------------------//  


  void compteur (unsigned int vitesse){
  if(compte%(vitesse/3)==0){    //permet de bouger la raquette plus vite que la balle bouge
    lancerBalle(balle1.deltaX, balle1.deltaY);
    deplacerRaquette(analogRead(X_pin));
    }
  if(compte%vitesse==0){
    testCollision(balle1.posX, balle1.posY);
      if(victoire!=1){
         deplacerBalle(balle1.deltaX,balle1.deltaY,balle1.posX,balle1.posY,balle1.lastPosX,balle1.lastPosY);
      }
      else{
           jouerMusiqueVictoire();
           chargementMap=2;
           balle1.vie+=3;//on ajoute 2 vie (on en perd une pendant l'initialisation
           initialisation();
           ht1632_plot(balle1.posX,balle1.posY,LOW);

           //vitesse=vitesse/2+vitesse/3;    //on peut éventuellement accélérer le jeu, mais ça rend pas super bien     
           victoire=0; 
        }   
  if(bonus1.bonusType!=NULL){
    if(compte%(vitesse*4)==0){//permet de bouger le bonus plus lentement que le jeu tourne
      deplacerBonus();
      } 
  }
  
  /*Serial.print("position bonus: ");
  Serial.print(bonus1.posX);
  Serial.print("  ");
  Serial.println(bonus1.posY);
  Serial.print("position balle: ");
  Serial.print(balle1.posX);
  Serial.print("  ");
  Serial.println(balle1.posY);
  Serial.print("déplacement: ");
  Serial.print(balle1.deltaX);
  Serial.print("  ");
  Serial.println(balle1.deltaY);*/
}

  }
// --------------------------------------------------------partie collision-------------------------------------------------------//  

#define collisionCote 4
#define collisionHautBas 2
#define collisionPlateformeGauche 3
#define collisionPlateformeMilieu 1
#define collisionPlateformeDroite 5
#define collisionCoin 6
#define chute 7


byte collision=0;

void testCollision(byte positionX, byte positionY){
  if((positionX==0 and positionY==0)or(positionX==31 and positionY==0)){
    collision=collisionCoin;
    }
  else if(positionX<=0 or positionX>=31){
    collision=collisionCote; 
    }
   else if (positionY<=0){
    collision=collisionHautBas;
    }
   else if (positionY==14 and balle1.deltaY<0){ //collision avec la raquette, on vérifie ou est-ce que la balle touche la raquette
      byte i=raquetteJoueur.jogPosX;
      for(i=raquetteJoueur.jogPosX;i<=raquetteJoueur.jogPosX+(raquetteJoueur.largeur/3);i++){
        if(positionX==i){
          collision=collisionPlateformeGauche;
         }
       }
      for(i=raquetteJoueur.jogPosX+raquetteJoueur.largeur/3+1;i<=raquetteJoueur.jogPosX+2*(raquetteJoueur.largeur/3);i++){
        if(positionX==i){
          collision=collisionPlateformeMilieu;
         }
       }
      for(i=raquetteJoueur.jogPosX+2*(raquetteJoueur.largeur/3)+1;i<=raquetteJoueur.jogPosX+raquetteJoueur.largeur;i++){
        if(positionX==i){
          collision=collisionPlateformeDroite;
          }
        }
   }  
    else if (positionY>=15){
      collision=chute;
      } 
   else{
        for(byte i=1,l=0;i<maxBrique;i++){//partie brique
              if(tableauBrique[i].vieBrique==1){    //vérifie s'il y a au moins une brique en vie
                 l=1;
                 }
                    for(byte j=0;j<tableauBrique[1].coteBrique;j++){
                      for(byte k=0;k<tableauBrique[1].coteBrique;k++){
                          if(tableauBrique[i].vieBrique==1){//vérifie que la Ième brique est en vie
                            if(positionX-balle1.deltaX==tableauBrique[i].briquePosX+j and positionY-balle1.deltaY==tableauBrique[i].briquePosY+k){//quand la balle est sur le point de toucher la brique effectuer:
                                    tableauBrique[i].vieBrique=0;//casse la brique
                                    changementBrique=1;            //réaffiche les brique (pour update la map), est buggé
                                    jouerMusiqueBrique();         //joue le son du casse brique (éventuellement le mettre ailleurs, ça slow le jeu
                                    generationBonus(tableauBrique[i].briquePosX,tableauBrique[i].briquePosY);   //a une chance de générer un bonus sur la pos ou la brique est détruite
                                      if(positionY<tableauBrique[i].briquePosY or positionY>tableauBrique[i].briquePosY+1){//regarde si la balle a touché la brique par les côte ou le haut/bas
                                      collision+=collisionHautBas;
                                      }
                                    else{
                                      collision+=collisionCote;
                                      }
                                   }
                                }    
                              }
                          }
                 if(i==nombreBriqueTotal and l==0){  //Si à la fin de la boucle for l est toujours égale à 0, cela signifie qu'il n'y a plus de brique
                    victoire=1;          
                       }
            }
      
   }
    switch(collision){
      case collisionCote:
        balle1.deltaX=-balle1.deltaX; 
        break;
      case collisionHautBas:
        balle1.deltaY=-balle1.deltaY;
        break;
      case collisionPlateformeGauche:
        balle1.deltaY=-balle1.deltaY;
        balle1.deltaX=1;
        break;
      case collisionPlateformeMilieu:
        balle1.deltaY=-balle1.deltaY;
        break;
      case collisionPlateformeDroite:
        balle1.deltaY=-balle1.deltaY;
        balle1.deltaX=-1;
        break;    
      case collisionCoin:
        balle1.deltaX=-balle1.deltaX; 
        balle1.deltaY=-balle1.deltaY;
        break;
      case chute:
        balleTombe(balle1.vie);
        break;
      }
    collision=0;   
  }


void balleTombe(byte nombreVie){
  jouerMusiqueChute();
  initialisation();
  }
// --------------------------------------------------------partie raquette-------------------------------------------------------//

void effacerEtAfficherRaquette(byte PositionX,byte largeurDeLaRaquette){
byte i;
for(i=0;i<PositionX;i++){
  ht1632_plot(i,15,LOW);
  }
for(i=PositionX;i<=PositionX+largeurDeLaRaquette;i++){
  ht1632_plot(i,15,ORANGE);
  }
for(i=PositionX+largeurDeLaRaquette+1;i<=32;i++){
  ht1632_plot(i,15,LOW);
  }  
 }
  

void deplacerRaquette(unsigned int posControleur){

  if(posControleur>600){                        //déplacement à gauche
    if(raquetteJoueur.jogPosX>=1){
         raquetteJoueur.jogPosX--;
         changementRaquette=1;
         if(balleLancee==0){
              ht1632_plot(balle1.posX,balle1.posY,LOW);     //quand la balle n'est pas encore lancée, elle est figée sur la raquette
              balle1.posX--;
              ht1632_plot(balle1.posX,balle1.posY,GREEN);        
            }
          }
        }
   if(posControleur<420){                      //déplacement à droite
     if(raquetteJoueur.jogPosX<=30-raquetteJoueur.largeur){
          raquetteJoueur.jogPosX++;
          changementRaquette=1;
          if(balleLancee==0){
              ht1632_plot(balle1.posX,balle1.posY,LOW);     //quand la balle n'est pas encore lancée, elle est figée sur la raquette
              balle1.posX++;
              ht1632_plot(balle1.posX,balle1.posY,GREEN);
            }
          }
        }
  }


// --------------------------------------------------------partie initialisation-------------------------------------------------------//


void initialisation(){ //remet la balle et la raquette au centre
  if(balle1.vie>0){
    balle1.posX=15;
    balle1.posY=14;
    balle1.deltaX=NULL;
    balle1.deltaY=NULL;
    balle1.vie=balle1.vie-1;
    changement7seg=1;
    raquetteJoueur= {13, 6, ORANGE };
    etatInitialisation=1;  
    balleLancee=0;
    affichageBrique();
    }
  else{
    jouerMusiqueDefaite();    //joue la musique de défaite et dégage l balle
    balle1.posX=0;
    balle1.posY=32;
    balle1.deltaX=0;
    balle1.deltaY=0;
    }
}

// --------------------------------------------------------partie brique-------------------------------------------------------//

void placementBriqueMap1(){                                               //on rentre les informations concernants les briques
  for (byte i=1;i<=maxBrique;i++){
    if(i<=nombreBriquePremiereLigne){
     tableauBrique[i]={1,2,4+2*i,4};
    }
    else if(i<=nombreBriqueTotal){
      int j=i-nombreBriquePremiereLigne;  
      tableauBrique[i]={1,2,6+2*j,6};       
    }
    else{
      tableauBrique[i].vieBrique=NULL;
      }
  }
  chargementMap=1; 
  }
  
void placementBriqueMap2(){
  for (byte i=1;i<=maxBrique;i++){
    if(i<=nombreBriquePremiereLigne2){
     tableauBrique[i]={1,2,4+2*i,3};
    }
    else if(i<=nombreBriquePremiereLigne2+nombreBriqueDeuxiemeLigne2){
      int j=i-nombreBriquePremiereLigne2;
      tableauBrique[i]={1,2,4+2*j,5};       
    }
    else if(i<=nombreBriquePremiereLigne2+nombreBriqueDeuxiemeLigne2+nombreBriqueTroisiemeLigne2){
      int k=i-nombreBriquePremiereLigne2-nombreBriqueDeuxiemeLigne2;
      if(k<=2){
        tableauBrique[i]={1,2,4+2*k,7};
        }
      else{
        tableauBrique[i]={1,2,16+2*k,7};
        }
    }
    else if(i<=nombreBriquePremiereLigne2+nombreBriqueDeuxiemeLigne2+nombreBriqueTroisiemeLigne2+nombreBriqueQuatriemeLigne2){
      int l=i-nombreBriquePremiereLigne2-nombreBriqueDeuxiemeLigne2-nombreBriqueTroisiemeLigne2;
      if(l<=2){
        tableauBrique[i]={1,2,4+2*l,9};
        }
      else{
        tableauBrique[i]={1,2,16+2*l,9};
        }
    }
    else{
      tableauBrique[i].vieBrique=NULL;
      }
  }
  chargementMap=1; 
  }
           

void affichageBrique(){                                               //on affiche pour la première fois les briques
  for (byte i=1;i<maxBrique;i++){
      for(byte j=0;j<tableauBrique[1].coteBrique;j++){
       for(byte k=0;k<tableauBrique[1].coteBrique;k++){
        if(tableauBrique[i].vieBrique==1){
          if((i%2)==0){
              ht1632_plot(tableauBrique[i].briquePosX+k,tableauBrique[i].briquePosY+j,RED);
            }
          else{
            ht1632_plot(tableauBrique[i].briquePosX+k,tableauBrique[i].briquePosY+j,ORANGE);
            }
          }
         else if(tableauBrique[i].vieBrique==0){
          ht1632_plot(tableauBrique[i].briquePosX+k,tableauBrique[i].briquePosY+j,LOW);
          }     
        }
      }
    }
  }
  
// --------------------------------------------------------partie bonus-------------------------------------------------------//

void generationBonus(byte posBriqueX, byte posBriqueY){
  if(bonus1.bonusType==NULL){   // ne génére un bonus que s'il n'y en a pas déjà 1 actif
  byte choix=random(1,5);   //va générer un bonus 1 fois sur 4
    if(choix==1){       
      //type=random(1,4);
      byte type=1;
      switch(type){
        case augLargeur:    //créer le bonus qui augmente la largeur de la plateforme
          bonus1={posBriqueX,posBriqueY, NULL,NULL,augLargeur,NULL};
          break;
  /*      case balleBonus:
          break;*/
  /*      case laser:
          break;*/
        }
       
      }
  }
}

void deplacerBonus(){
   if(bonus1.posY==14){//quand le bonus atteinds la derniere ligne, voir s'il attérit sur la pltaeforme ou non
    attraperOuPerdreBonus();
    }
  else{
    bonus1.lastPosX=bonus1.posX;
    bonus1.lastPosY=bonus1.posY;
    bonus1.posY=bonus1.posY+1; 
    changementBonus=1;
    }
}
void effacerEtAfficherBonus(){
  ht1632_plot(bonus1.lastPosX,bonus1.lastPosY,LOW);
  ht1632_plot(bonus1.posX,bonus1.posY,RED);

  }
  
void attraperOuPerdreBonus(){
  byte i;
for(i=0;i<raquetteJoueur.jogPosX;i++){//le bonus n'est pas sur la raquette
  if(bonus1.posX==i){
    bonus1.bonusType=NULL;
    }
  }
for(i=raquetteJoueur.jogPosX;i<=raquetteJoueur.jogPosX+raquetteJoueur.largeur;i++){
    if(bonus1.posX==i){//le bonus est sur la raquette
      jouerMusiqueBonus();
      switch(bonus1.bonusType){
        case augLargeur:
            raquetteJoueur.largeur=raquetteJoueur.largeur+3;
            bonus1.bonusType=NULL;
            break;
        }
      }
    }
for(i=raquetteJoueur.jogPosX+raquetteJoueur.largeur+1;i<=32;i++){//le bonus n'est pas sur la raquette
  if(bonus1.posX==i){
      bonus1.bonusType=NULL;    }
  }
  ht1632_plot(bonus1.posX,bonus1.posY,LOW);

  }

// --------------------------------------------------------partie afficheur 7 segments-------------------------------------------------------//

void gauche(byte octet){
  Wire.beginTransmission(0x22);      // start talking to the device
  Wire.write(0x09);                  // select the GPIO register
  Wire.write(Tab7Segts[octet%10]);         // set register value-all low
  // droite : unités
  Wire.endTransmission();            // stop talking to the device

  Wire.beginTransmission(0x23);      // start talking to the device
  Wire.write(0x09);                  // select the GPIO register
  Wire.write(Tab7Segts[octet/10]);         // set register value-all low
  // gauche : dizaines
  Wire.endTransmission();            // stop talking to the device
}

void droite(byte octet){
  Wire.beginTransmission(0x20);      // start talking to the device
  Wire.write(0x09);                  // select the GPIO register
  Wire.write(Tab7Segts[octet%10]);          // set register value-all low
  // unités
  Wire.endTransmission();            // stop talking to the device

  Wire.beginTransmission(0x21);      // start talking to the device
  Wire.write(0x09);                  // select the GPIO register
  Wire.write(Tab7Segts[octet/10]);          // set register value-all low
  // dizaines
  Wire.endTransmission();            // stop talking to the device
}
  
// --------------------------------------------------------partie son-------------------------------------------------------//

// TONES  ==========================================
// Start by defining the relationship between
//       note, period, &  frequency.
#define  c     3830    // 261 Hz
#define  d     3400    // 294 Hz
#define  e     3038    // 329 Hz
#define  f     2864    // 349 Hz
#define  g     2550    // 392 Hz
#define  a     2272    // 440 Hz
#define  b     2028    // 493 Hz
#define  C     1912    // 523 Hz
// Define a special note, 'R', to represent a rest
#define  R     0

// MELODY and TIMING  =======================================On ecrit ici le morceau
//  melody[] is an array of notes, accompanied by beats[],
//  which sets each note's relative length (higher #, longer note)
int melodyChute[] = {  C,  b,  g,  e};
int beatsChute[]  = { 32, 32, 32,  64};
int melodyBrique[] = { b};
int beatsBrique[]  = { 2};
int melodyDefaite[] = { b, g, C,  b, g,  e};
int beatsDefaite[]  = { 32, 32, 32, 32, 32,  64};
int melodyVictoire[] = { f, g, a,  b, f, g, a,  b, f,  g, b, C};
int beatsVictoire[]  = { 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 64};
int melodyBonus[] = { e, C};
int beatsBonus[]  = { 8, 16};
int MAX_COUNT_CHUTE = sizeof(melodyBrique) / 2;// Melody length, for looping.
int MAX_COUNT_BRIQUE = sizeof(melodyBrique) / 2;// Melody length, for looping.
int MAX_COUNT_DEFAITE = sizeof(melodyDefaite) / 2;// Melody length, for looping.
int MAX_COUNT_VICTOIRE = sizeof(melodyVictoire) / 2;// Melody length, for looping.
int MAX_COUNT_BONUS = sizeof(melodyBonus) / 2;// Melody length, for looping.



// Set overall tempo
long tempo = 10000;
// Set length of pause between notes
int pause = 1000;
// Loop variable to increase Rest length
int rest_count = 100; //<-BLETCHEROUS HACK; See NOTES

// Initialize core variables
int tone_ = 0;
int beat = 0;
long duration  = 0;

// PLAY TONE  ==============================================
// Pulse the speaker to play a tone for a particular duration
void playTone() {
  long elapsed_time = 0;
  if (tone_ > 0) { // if this isn't a Rest beat, while the tone has
    //  played less long than 'duration', pulse speaker HIGH and LOW
    while (elapsed_time < duration) {

      digitalWrite(BUZZER,HIGH);
      delayMicroseconds(tone_ / 2);
      // DOWN
      digitalWrite(BUZZER, LOW);
      delayMicroseconds(tone_ / 2);
      // Keep track of how long we pulsed
      elapsed_time += (tone_);
    }
  }
  else { // Rest beat; loop times delay
    for (int j = 0; j < rest_count; j++) { // See NOTE on rest_count
      delayMicroseconds(duration);  
    }                                
  }                                
}

// Joue le morceau =============================
void jouerMusiqueChute() {
  // Set up a counter to pull from melody[] and beats[]
  for (int i=0; i<MAX_COUNT_CHUTE; i++) {
    tone_ = melodyChute[i];
    beat = beatsChute[i];

    duration = beat * tempo; // Set up timing

    playTone();
    // A pause between notes...
    delayMicroseconds(pause);

    if (DEBUG) { // If debugging, report loop, tone, beat, and duration
      Serial.print(i);
      Serial.print(":");
      Serial.print(beat);
      Serial.print(" ");    
      Serial.print(tone_);
      Serial.print(" ");
      Serial.println(duration);
    }
  }
}
void jouerMusiqueBrique() {
  // Set up a counter to pull from melody[] and beats[]
  for (int i=0; i<MAX_COUNT_BRIQUE; i++) {
    tone_ = melodyBrique[i];
    beat = beatsBrique[i];

    duration = beat * tempo; // Set up timing

    playTone();
    // A pause between notes...
    delayMicroseconds(pause);
  }
}

void jouerMusiqueDefaite() {
  // Set up a counter to pull from melody[] and beats[]
  for (int i=0; i<MAX_COUNT_DEFAITE; i++) {
    tone_ = melodyDefaite[i];
    beat = beatsDefaite[i];

    duration = beat * tempo; // Set up timing

    playTone();
    // A pause between notes...
    delayMicroseconds(pause);
  }
}

void jouerMusiqueVictoire() {
  // Set up a counter to pull from melody[] and beats[]
  for (int i=0; i<MAX_COUNT_VICTOIRE; i++) {
    tone_ = melodyVictoire[i];
    beat = beatsVictoire[i];

    duration = beat * tempo; // Set up timing

    playTone();
    // A pause between notes...
    delayMicroseconds(pause);
  }
}

void jouerMusiqueBonus() {
  // Set up a counter to pull from melody[] and beats[]
  for (int i=0; i<MAX_COUNT_BONUS; i++) {
    tone_ = melodyBonus[i];
    beat = beatsBonus[i];

    duration = beat * tempo; // Set up timing

    playTone();
    // A pause between notes...
    delayMicroseconds(pause);
  }
}
// --------------------------------------------------------partie loop-------------------------------------------------------//
  void loop () {
    if(changement7seg==1){
      gauche(balle1.vie);
      changement7seg=0;
    }
    
  if(chargementMap==0){
      placementBriqueMap1();
      affichageBrique();
      }
   if(chargementMap==2){
      placementBriqueMap2();
      affichageBrique();
      }   
  
  if(etatInitialisation==0){
      initialisation();
      }
  
  if(changementBalle==1){
    effacerEtAfficherBalle(balle1.lastPosX,balle1.lastPosY,balle1.posX, balle1.posY);
    changementBalle=0;
    }

  if(bonus1.bonusType!=NULL and changementBonus==1){
        effacerEtAfficherBonus();
        changementBonus=0;    
        affichageBrique();      
     }
   else if(changementBrique==1){   //le bonus peut traverser des briques, on affiche les briques donc s'il y a un bonus ou il une balle touche la brique
    affichageBrique();
    changementBrique=0;
    }    
    
effacerEtAfficherRaquette(raquetteJoueur.jogPosX, raquetteJoueur.largeur);
    
   
}
  
  

// --------------------------------------------------------partie fonction périodique-------------------------------------------------------//
void fnPeriodique(void) {
  compteur(vitesse);  
  compte++;
  }
