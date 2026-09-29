#include <Arduino.h>
// Capteur de temperature et d'humidite DHT11
// https://tutoduino.fr/
// Copyleft 2020
#include "DHT.h"
// Definit la broche de l'Arduino sur laquelle la
// broche DATA du capteur est reliee
#define DHTPIN 4
// Definit le type de capteur utilise
#define DHTTYPE DHT11
// Declare un objet de type DHT
// Il faut passer en parametre du constructeur
// de l'objet la broche et le type de capteur
DHT dht(DHTPIN, DHTTYPE);
// void setup()
// {
//   Serial.begin(9600);
//   Serial2.begin(9600);

//   delay(1000);

//   Serial2.println("AT+ID=AppEUI,0000000000000000");

//   Serial2.println("AT+ID=DevEUI,70B3D57ED007922E");
//   Serial2.println("AT+KEY=APPKEY,AF083CDC5BE612AAEB359989124B096B");

//   Serial2.println("AT+MODE=LWOTAA");//  set OTAA join mode
//   Serial2.println("AT+DR=DR3"); //  change speed for SF9 BW125

//   Serial2.println("AT+MSGHEX=01020304");

//   // Initialise la capteur DHT11
//   dht.begin();
// }

// Envoie une commande AT et affiche la reponse du LoRa-E5
bool envoyerCommande(String commande, unsigned long timeout = 2000)
{
  // Vide les anciennes donnees dans le buffer UART
  while (Serial2.available())
  {
    Serial2.read();
  }

  Serial.print("Commande envoyee : ");
  Serial.println(commande);

  Serial2.println(commande);

  String reponse = "";
  unsigned long debut = millis();

  // Attend la reponse du LoRa-E5
  while (millis() - debut < timeout)
  {
    while (Serial2.available())
    {
      char c = Serial2.read();
      reponse += c;

      // Affiche directement ce que repond le LoRa-E5
      Serial.write(c);
    }
  }

  Serial.println();

  // Aucune reponse
  if (reponse.length() == 0)
  {
    Serial.println("ERREUR : aucune reponse du LoRa-E5");
    return false;
  }

  // Detection d'une erreur dans la reponse
  if (reponse.indexOf("ERROR") != -1 ||
      reponse.indexOf("FAIL") != -1)
  {
    Serial.println("ERREUR : commande refusee");
    return false;
  }

  Serial.println("Commande OK");
  Serial.println();

  return true;
}

void setup()
{
  Serial.begin(9600);

  Serial2.begin(9600);

  delay(1000);

  Serial.println();
  Serial.println("===== Initialisation LoRa-E5 =====");

  if (!envoyerCommande("AT"))
  {
    Serial.println("LoRa-E5 non detecte !");
    return;
  }

  Serial.println("LoRa-E5 detecte !");
  Serial.println();

  envoyerCommande("AT+ID=AppEUI,0000000000000000");
  envoyerCommande("AT+ID=DevEUI,70B3D57ED007922E");
  envoyerCommande("AT+KEY=APPKEY,XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

  envoyerCommande("AT+MODE=LWOTAA");
  envoyerCommande("AT+DR=DR3");

  envoyerCommande("AT+JOIN");

  delay(5000);

  envoyerCommande("AT+MSGHEX=01020304");

  dht.begin();

  Serial.println();
  Serial.println("===== Initialisation terminee =====");
}

void loop()
{
  // Recupere la temperature et l'humidite du capteur et l'affiche
  // sur le moniteur serie
  int temp = dht.readTemperature();
  int hum = dht.readHumidity();
  Serial.println("Temperature = " + String(temp) + " °C");
  Serial.println("Humidite = " + String(hum) + " %");

  int temp10 = temp * 10;
  int hum10 = hum * 10;
  char message[9];

  sprintf(message, "%04X%04X", temp10, hum10);
  Serial.print("Message HEX : ");
  Serial.println(message);

  envoyerCommande("AT+MSGHEX=" + String(message));

  // Attend 10 secondes avant de reboucler
  delay(1000);
}
