/*
 * Station météo by ensciences.fr
 * Développement : Th. G
 */

/* Définition de l'ethernet */
#include <Ethernet.h>
#include <SPI.h>
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 }; // RESERVED MAC ADDRESS
EthernetClient client;
String data;

/* Définition du module humidité */
#include <dht11.h>
#define DHT11PIN 4 // broche DATA -> broche 4
dht11 DHT11;

/* Définition du module barométrique */
#include <SFE_BMP180.h>
#include <Wire.h>
SFE_BMP180 pressure;
#define ALTITUDE 50.0   // Altitude du module

/* Définition du module PM2.5 */
#define COV_RATIO 0.2         // µg/m3 / mv
#define NO_DUST_VOLTAGE 400   // mv
#define SYS_VOLTAGE 5000      

/* Définition des constantes et variables pour la thermistance */
#define RT0 10000   // Ω
#define B 3977      // K
#define VCC 5       //Supply voltage
#define R 10000     //R=10KΩ
float RT, VR, ln, TX, T0, VRT;

/* Entrées analogiques */
int temp_id = 0;      // Analogique thermistance
const int iled = 7;   // Capteur LED PM2.5
const int vout = 1;   // Analogique PM2.5


/* Fonction pour le module PM2.5 */
float density, voltage;
int   adcvalue;
int Filter(int m)
{
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}


/*
 * Initialisation du serial port.
 */
void setup( )
{
  Serial.begin(115200);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP"); 
  }

  /* Message de bienvenue. */
  Serial.println("Station météo par ensciences.fr (Th. G)");
  Serial.println();

  /* Initialisation du module DHT11. */

  while (!Serial) {
    // Attente de connexion du serial port
  }

  Serial.println("DHT11 init success");

  /* Initialisation de module BMP180. */

  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail");
    while(1); 
  }

  /* Initialisation de module PM2.5. */
  pinMode(iled, OUTPUT);
  digitalWrite(iled, LOW);  

  Serial.println();
}


/*
 * Boucle principale.
 */
void loop( )
{


  /*
   * Module thermistance et humidité/température (DHT11).
   */
  float temperature;
  float humidity;

  T0 = 25 + 273.15;                  // Calcul de la température standard
  VRT = analogRead(temp_id);         // Acquisition de la valeur analogique de la thermistance
  VRT = (5.00 / 1023.00) * VRT;      // Conversion den volts
  VR = VCC - VRT;
  RT = VRT / (VR / R);               // Calcul de la résistance

  ln = log(RT / RT0);
  TX = (1 / ((ln / B) + (1 / T0))); // Temperature (en K) de la thermistance

  TX = TX - 273.15;                 // Conversion den Celsius

  DHT11.read(DHT11PIN);

  Serial.print("Température 1 (°C): ");
  Serial.println(TX);
  Serial.print("Température 2 (°C): ");
  Serial.println((float)DHT11.temperature, 2);
  Serial.print("Humidité (%): ");
  Serial.println((float)DHT11.humidity, 2);
  

  /*
   * Module barométrique (BMP180).
   */

  char status;
  double T,P,p0,a;

  status = pressure.startTemperature();
  if (status != 0)
  {
    // On attend que la mesure se termine
    delay(status);

    // Mesure de la température
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      Serial.print("Temperature 3 (°C): ");
      Serial.println(T,2);

      
      // Mesure de la pression (paramètre : 0 à 3 = haute res, temps long)
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // On attend que la mesure se termine
        delay(status);
        
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          Serial.print("Pression absolue (hPa): ");
          Serial.println(P,2);

          // On s'affranchit de l'influence de l'altitude : p0 retourne la pression au niveau de la mer (pression relative)
          p0 = pressure.sealevel(P,ALTITUDE); 
          Serial.print("Pression relative (hPa): ");
          Serial.println(p0,2);
        }
        else Serial.println("Erreur lors de la mesure de la pression.\n");
      }
      else Serial.println("Erreur lors de la mesure de la pression.\n");
    }
    else Serial.println("Erreur lors de la mesure de la température.\n");
  }
  else Serial.println("Erreur lors de la mesure de la température.\n");
  
  
  /*
   * Module de détection de particules (PM2.5).
   */
   
    digitalWrite(iled, HIGH);
    delayMicroseconds(280);
    adcvalue = analogRead(vout);
    digitalWrite(iled, LOW);
    
    adcvalue = Filter(adcvalue);
    
    voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11; // Conversion du voltage
    
    if(voltage >= NO_DUST_VOLTAGE) // Conversion du voltage en densité
    {
      voltage -= NO_DUST_VOLTAGE;
      
      density = voltage * COV_RATIO;
    }
    else
      density = 0;
      
    Serial.print("Concentration en particules (µg/m3): ");
    Serial.println(density);
   

  Serial.println();

  /*
   * Transmission vers le serveur
   */
 
float T1 = (float)TX;
float T2 = (float)DHT11.temperature;
float T3 = (float)T;
float h = (float)DHT11.humidity;
float Pabs = (float)P;
float Prel = (float)p0;
float PM25 = (float)density;


  data = "temp1=" + String(T1,2) + "&temp2=" + String(T2,2) + "&temp3=" + String(T3,2) +  "&humi=" + String(h,2) + "&pres=" + String(Pabs,2) + "&relpres=" + String(Prel,2) + "&pm25=" + String(PM25,2) ;
  Serial.println(data);
  
  if (client.connect("votre_serveur",80)) { // REPLACE WITH YOUR SERVER ADDRESS
    client.println("POST /add.php HTTP/1.1"); 
    client.println("Host: votre_serveur"); // SERVER ADDRESS HERE TOO
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded"); 
    client.print("Content-Length: "); 
    client.println(data.length()); 
    client.println(); 
    client.print(data); 
    client.println(); 
    Serial.println("Envoi des données avec succès");
  } else {
    Serial.println("Echec de l'envoi");
  }

  if (client.connected()) { 
    client.stop();  // DISCONNECT FROM THE SERVER
  }

   Serial.println();
  delay(300000);  // Pause de 2min.
}
