# TP2 — ESP32, LoRa-E5, LoRaWAN, The Things Network et Node-RED

## Présentation du projet

Ce projet correspond à la mise en place d'une chaîne IoT complète basée sur un **ESP32**, un **module LoRa-E5**, un **capteur DHT11**, **The Things Network (TTN)** et **Node-RED**.

Le principe général est le suivant : l'ESP32 récupère la température et l'humidité avec le DHT11, transforme ces mesures en une trame hexadécimale puis demande au module LoRa-E5 de l'envoyer sur le réseau LoRaWAN. La trame arrive ensuite sur The Things Network, puis elle est récupérée dans Node-RED avec MQTT afin d'afficher les valeurs sur un dashboard.

La communication fonctionne également dans l'autre sens. Un bouton présent dans Node-RED permet d'envoyer un **downlink** à travers TTN. Ce downlink est reçu par le LoRa-E5 puis lu par l'ESP32, qui allume ou éteint la LED connectée au GPIO 2.

Le fonctionnement peut donc être résumé ainsi :

```text
DHT11
  │
  ▼
ESP32 ──UART2──> LoRa-E5 ──LoRaWAN──> TTN ──MQTT──> Node-RED
  ▲                                                   │
  │                                                   │
  └──────── LED GPIO 2 <── Downlink <── TTN <─────────┘
```

> Ce README a été réalisé à partir des fichiers du projet, du code ESP32, de la configuration TTN, du flow Node-RED, des scripts et des captures fournies. Le contenu du compte rendu `CR_TP_2_IOT_Youssef_EL_KATTOUFI.docx` n'a pas été utilisé pour rédiger cette documentation.

---

## 1. Organisation des fichiers

La structure principale du projet est la suivante :

```text
TP2/
│
├── Esp32_lorawan/
│   ├── src/
│   │   └── main.cpp
│   ├── include/
│   ├── lib/
│   ├── test/
│   ├── platformio.ini
│   └── .gitignore
│
├── Node_red_architecture/
│   └── flows.json
│
├── Config_the_thing.docx
├── photo_dashboard.png
├── The_things.url
├── NodeRED_Manager.bat - Raccourci.lnk
├── git_push.bat
├── TP2_LoRaWAN_UpLink_DownLink_v2025.pdf
└── .gitignore
```

### Rôle des principaux fichiers

- **`Esp32_lorawan/`** : projet PlatformIO contenant le programme exécuté par l'ESP32.
- **`src/main.cpp`** : code principal. Il gère le DHT11, le LoRa-E5, l'envoi des uplinks et la réception des downlinks.
- **`platformio.ini`** : configuration de la carte ESP32 et des bibliothèques nécessaires.
- **`Node_red_architecture/flows.json`** : export du flow Node-RED utilisé pour recevoir les données TTN, afficher les graphiques et envoyer les commandes de LED.
- **`Config_the_thing.docx`** : captures de la configuration du end device dans The Things Network.
- **`photo_dashboard.png`** : résultat du dashboard Node-RED avec les courbes de température, d'humidité et le bouton LoRaWAN.
- **`The_things.url`** : raccourci permettant d'ouvrir directement la page du end device TTN utilisée pendant le TP.
- **`git_push.bat`** : petit script permettant d'enchaîner les commandes Git utilisées pour enregistrer et envoyer les modifications.
- **`NodeRED_Manager.bat - Raccourci.lnk`** : raccourci vers un script personnel présent sur mon ordinateur pour gérer Node-RED. Le fichier `.bat` d'origine n'est pas contenu dans cette archive, le raccourci seul ne permet donc pas de l'utiliser sur un autre PC.

---

## 2. Environnement de développement

J'ai réalisé le projet avec **Visual Studio Code et PlatformIO** plutôt qu'avec l'Arduino IDE. Cela me permet d'avoir un projet mieux organisé et de gérer les bibliothèques directement à partir du fichier `platformio.ini`.

La configuration utilisée est :

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 9600
lib_deps = adafruit/DHT sensor library@^1.4.7
```

La carte sélectionnée est donc une **ESP32 DOIT DevKit V1**, avec le framework Arduino. Le moniteur série est configuré à **9600 bauds** et PlatformIO télécharge automatiquement la bibliothèque Adafruit nécessaire au DHT11.

Pour ouvrir le projet, il suffit d'ouvrir le dossier `Esp32_lorawan` avec VS Code/PlatformIO, puis de compiler et téléverser le programme sur l'ESP32.

---

## 3. Câblage utilisé

Le programme utilise principalement trois éléments : le DHT11, le LoRa-E5 et la LED de l'ESP32.

### DHT11

La broche DATA du DHT11 est reliée au :

```cpp
#define DHTPIN 4
```

Le capteur est donc lu depuis le **GPIO 4**.

### LED

La LED est commandée avec :

```cpp
#define LED_PIN 2
```

Elle est donc reliée au **GPIO 2**.

### LoRa-E5

La communication entre l'ESP32 et le LoRa-E5 est effectuée avec **UART2** à 9600 bauds :

```cpp
Serial2.begin(9600);
```

Le câblage prévu pour le projet est :

```text
ESP32 GPIO 17 (TX2)  ---> RX du LoRa-E5
ESP32 GPIO 16 (RX2)  <--- TX du LoRa-E5
ESP32 3V3            ---> 3V3 du LoRa-E5
ESP32 GND            ---> GND du LoRa-E5
```

---

## 4. Configuration de The Things Network

Avant de pouvoir envoyer des données, j'ai créé une application puis un **end device** dans The Things Network.

Dans la configuration du périphérique, le module a été associé au profil LoRaWAN de Seeed Technology correspondant au **LoRaWAN Dev Kit**, avec la région **EU_863_870** et le plan de fréquence européen **863-870 MHz**.

Le fonctionnement repose sur une activation **OTAA**. TTN fournit ou permet de définir les informations suivantes :

- JoinEUI / AppEUI ;
- DevEUI ;
- AppKey ;
- identifiant du end device.

Ces valeurs doivent correspondre à celles envoyées au module LoRa-E5 par l'ESP32.

Dans le code, elles sont configurées avec des commandes de ce type :

```cpp
envoyerCommande("AT+ID=AppEUI,XXXXXXXXXXXXXXXX");
envoyerCommande("AT+ID=DevEUI,XXXXXXXXXXXXXXXX");
envoyerCommande("AT+KEY=APPKEY,XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
```

Les valeurs réelles sont déjà présentes dans le projet fourni, mais elles ne sont volontairement pas recopiées dans ce README.

> **Important :** l'AppKey est une information sensible. Si le projet doit être publié sur un dépôt public, il faut supprimer cette clé du code, la remplacer par une valeur privée ou la régénérer dans TTN.

Le fichier `Config_the_thing.docx` contient les captures utilisées pendant cette étape. Le fichier `The_things.url` est simplement un raccourci vers la console du device et nécessite évidemment d'être connecté à un compte TTN ayant accès à l'application.

---

## 5. Initialisation du LoRa-E5 depuis l'ESP32

J'ai choisi de commander le LoRa-E5 directement avec des **commandes AT envoyées par UART**.

Pour éviter de répéter le même code, j'ai créé la fonction :

```cpp
bool envoyerCommande(String commande, unsigned long timeout = 2000)
```

Cette fonction réalise plusieurs opérations :

1. elle vide les anciennes données encore présentes dans le buffer UART ;
2. elle affiche la commande sur le moniteur série ;
3. elle transmet la commande au LoRa-E5 avec `Serial2.println()` ;
4. elle lit et affiche la réponse du module ;
5. elle vérifie si la réponse contient `ERROR` ou `FAIL` ;
6. elle retourne `true` si la commande ne présente pas d'erreur détectée.

Au démarrage, le programme teste d'abord la présence du module :

```cpp
envoyerCommande("AT");
```

Si le module répond, la configuration LoRaWAN est ensuite envoyée :

```text
AT+ID=AppEUI,...
AT+ID=DevEUI,...
AT+KEY=APPKEY,...
AT+MODE=LWOTAA
AT+DR=DR3
AT+JOIN
```

`AT+MODE=LWOTAA` sélectionne l'activation OTAA et `AT+JOIN` demande ensuite au module de rejoindre le réseau LoRaWAN.

Une première trame de test est envoyée avec :

```cpp
AT+MSGHEX=01020304
```

Cela permet de vérifier que le module est bien connecté et qu'une trame remonte dans les Live Data de TTN.

---

## 6. Lecture du DHT11 et construction de la trame

Dans la boucle principale, l'ESP32 lit la température et l'humidité :

```cpp
int temp = dht.readTemperature();
int hum = dht.readHumidity();
```

Les valeurs sont ensuite multipliées par 10 :

```cpp
int temp10 = temp * 10;
int hum10 = hum * 10;
```

Puis elles sont regroupées dans une chaîne hexadécimale :

```cpp
char message[9];
sprintf(message, "%04X%04X", temp10, hum10);
```

La trame contient donc :

```text
TTTT HHHH
```

avec :

- `TTTT` : température multipliée par 10 et codée sur 4 caractères hexadécimaux ;
- `HHHH` : humidité multipliée par 10 et codée sur 4 caractères hexadécimaux.

Par exemple, avec une température entière de 25 °C et une humidité entière de 50 %, le principe donne :

```text
25 × 10 = 250  -> 00FA
50 × 10 = 500  -> 01F4

Payload : 00FA01F4
```

La trame est ensuite transmise au LoRa-E5 :

```cpp
envoyerCommande("AT+MSGHEX=" + String(message));
```

Le LoRa-E5 se charge alors de l'envoi LoRaWAN vers TTN.

### Remarque sur la précision

Même si les valeurs sont multipliées par 10 avant l'envoi, le code actuel stocke d'abord la température et l'humidité dans des variables de type `int`. Les décimales retournées par le DHT sont donc supprimées avant la création de la trame. Le fonctionnement actuel envoie donc essentiellement des valeurs entières mises à l'échelle par 10.

---

## 7. Décodage côté TTN

Le flow Node-RED ne travaille pas directement avec les quatre octets du payload. Il récupère :

```javascript
msg.payload.uplink_message.decoded_payload
```

et attend deux champs :

```text
temp
hum
```

Cela signifie qu'un **Payload Formatter / Uplink Decoder** doit être configuré dans TTN afin de transformer la trame reçue en données lisibles.

Le code du décodeur TTN n'est pas présent dans les fichiers de l'archive, mais le format envoyé par l'ESP32 permet de déterminer le principe attendu : deux valeurs de 16 bits en big-endian, divisées par 10 après décodage.

Un décodeur compatible avec le format du programme serait par exemple :

```javascript
function decodeUplink(input) {
    if (input.bytes.length < 4) {
        return {
            errors: ["Payload trop court"]
        };
    }

    let temp10 = (input.bytes[0] << 8) | input.bytes[1];
    let hum10  = (input.bytes[2] << 8) | input.bytes[3];

    return {
        data: {
            temp: temp10 / 10,
            hum: hum10 / 10
        }
    };
}
```

Ce décodeur est donné ici parce qu'il correspond au format construit dans `main.cpp` et aux champs attendus dans `flows.json`.

---

## 8. Récupération des données TTN dans Node-RED

Le flow complet est disponible dans :

```text
Node_red_architecture/flows.json
```

Il peut être importé dans Node-RED avec :

```text
Menu -> Import -> Clipboard / File
```

Le flow utilise directement le broker MQTT de The Things Network :

```text
Broker : eu1.cloud.thethings.network
Port   : 8883
TLS    : activé
```

Le topic d'uplink suit la structure TTN V3 :

```text
v3/<application>@ttn/devices/<device>/up
```

Le flow fourni contient déjà les identifiants d'application et de device utilisés pendant le TP. Pour réutiliser le projet avec un autre compte ou un autre end device, il faut donc modifier ces topics.

Les identifiants MQTT sensibles ne sont pas présents dans le fichier `flows.json`. Après import du flow, il faut donc vérifier/configurer les informations d'authentification du broker TTN dans le nœud MQTT.

Le flow utilise également le module :

```text
node-red-dashboard 3.6.6
```

Il doit être installé dans Node-RED pour que les graphiques et le switch soient disponibles.

---

## 9. Traitement de l'uplink dans Node-RED

Lorsqu'une nouvelle trame est reçue depuis TTN, un nœud `function` récupère les données décodées :

```javascript
let data = msg.payload.uplink_message.decoded_payload;

msg.payload = {
    temperature: data.temp,
    humidite: data.hum
};

return msg;
```

Le message est ensuite séparé en deux branches.

Pour la température :

```javascript
msg.payload = msg.payload.temperature;
return msg;
```

Pour l'humidité :

```javascript
msg.payload = msg.payload.humidite;
return msg;
```

Chaque valeur est envoyée vers un graphique du dashboard.

La capture `photo_dashboard.png` montre le résultat obtenu :

- une courbe **Temperature** ;
- une courbe **humidite** ;
- un switch **Button lora wan**.

---

## 10. Envoi d'un downlink depuis Node-RED

Le bouton du dashboard permet de commander la LED de l'ESP32 à distance.

Le switch produit :

```text
1 -> LED demandée ON
0 -> LED demandée OFF
```

Un nœud `function` transforme cette valeur en un payload compatible avec l'API MQTT de TTN :

```javascript
let etat = Number(msg.payload);

let payload = Buffer.from([etat]).toString("base64");

msg.payload = JSON.stringify({
    downlinks: [
        {
            f_port: 1,
            frm_payload: payload,
            priority: "NORMAL"
        }
    ]
});

return msg;
```

La valeur est convertie en un octet puis encodée en **Base64**, car c'est le format attendu dans le message MQTT de downlink TTN.

Le message est ensuite publié sur :

```text
v3/<application>@ttn/devices/<device>/down/push
```

TTN place alors la commande dans la file de downlink du device.

---

## 11. Réception du downlink sur l'ESP32

Après l'envoi d'un uplink, l'ESP32 écoute la réponse du LoRa-E5 avec :

```cpp
String reponse = lireReponseLoRa();
```

Cette fonction lit pendant plusieurs secondes les caractères reçus sur `Serial2` et reconstitue la réponse complète du module.

Le programme recherche ensuite directement le contenu du downlink :

```cpp
if (reponse.indexOf("RX: \"01\"") != -1)
{
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ALLUMEE");
}

if (reponse.indexOf("RX: \"00\"") != -1)
{
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED ETEINTE");
}
```

Si le LoRa-E5 indique qu'il a reçu `01`, la LED est allumée. S'il reçoit `00`, la LED est éteinte.

Cette méthode permet donc d'obtenir une communication dans les deux sens :

```text
ESP32 -> TTN -> Node-RED      : uplink
Node-RED -> TTN -> ESP32      : downlink
```

---

## 12. Dashboard Node-RED

Le dashboard final sert à visualiser rapidement le fonctionnement du système.

Il contient :

- un graphique de température ;
- un graphique d'humidité ;
- un bouton permettant d'envoyer un état 0 ou 1 vers l'ESP32.

Le fichier `photo_dashboard.png` permet de voir le résultat obtenu pendant le TP.

---

## 13. Utilisation de Git

J'ai également ajouté un petit fichier :

```text
git_push.bat
```

Son contenu est :

```bat
echo off
git status && git add . && git commit -m "TP fini" && git push
```

Le but est d'éviter de retaper les mêmes commandes à chaque fois que je souhaite envoyer une nouvelle version du projet.

Le script effectue dans l'ordre :

```text
git status
    ↓
git add .
    ↓
git commit -m "TP fini"
    ↓
git push
```

L'utilisation de `&&` est importante : la commande suivante n'est exécutée que si la précédente s'est terminée correctement.

Ce fichier simplifie donc la démarche **une fois que le dépôt Git est déjà initialisé et relié à un dépôt distant**.

Pour un nouveau projet, la configuration initiale doit d'abord être faite une fois, par exemple :

```bash
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin <URL_DU_DEPOT>
git push -u origin main
```

Après cela, `git_push.bat` suffit pour les mises à jour courantes.

Le `.gitignore` du projet PlatformIO permet notamment de ne pas envoyer les fichiers générés localement dans `.pio` ainsi que plusieurs fichiers temporaires de VS Code.

---

## 14. À propos du raccourci NodeRED Manager

Le fichier :

```text
NodeRED_Manager.bat - Raccourci.lnk
```

n'est pas le script lui-même. Il pointe vers un fichier `NodeRED_Manager.bat` qui se trouve dans un autre dossier de mon ordinateur.

Ce fichier `.bat` original n'est donc **pas fourni avec le TP**. Sur un autre ordinateur, le raccourci ne fonctionnera pas puisque le chemin local n'existera pas.

Cela ne bloque cependant pas le projet : Node-RED peut être lancé normalement depuis son installation. Le raccourci était uniquement une méthode personnelle pour lancer ou gérer plus rapidement mon environnement Node-RED.

---

## 15. Ce qu'il faut modifier pour réutiliser le projet

Pour refaire le TP avec un autre compte TTN ou un autre matériel, il faut principalement modifier :

1. les valeurs **AppEUI / JoinEUI, DevEUI et AppKey** dans `main.cpp` ;
2. le device correspondant dans la console TTN ;
3. les topics MQTT du flow Node-RED ;
4. les identifiants MQTT de TTN dans le broker Node-RED ;
5. éventuellement les GPIO si le câblage est différent.

Il faut ensuite :

```text
1. Configurer le device dans TTN
2. Câbler ESP32 + DHT11 + LoRa-E5
3. Compiler et téléverser le projet PlatformIO
4. Vérifier le JOIN LoRaWAN dans le moniteur série
5. Vérifier les uplinks dans les Live Data TTN
6. Configurer le décodeur de payload dans TTN
7. Importer flows.json dans Node-RED
8. Configurer l'accès MQTT TTN
9. Vérifier les graphiques du dashboard
10. Tester le switch de downlink pour commander la LED
```

---

## 16. Points à connaître dans la version actuelle

Quelques éléments sont spécifiques à la version actuelle du projet :

- les identifiants LoRaWAN sont actuellement écrits directement dans `main.cpp` ;
- le décodeur TTN n'est pas exporté dans l'archive ;
- les identifiants MQTT TTN ne sont pas présents dans `flows.json` ;
- le script réel `NodeRED_Manager.bat` n'est pas fourni, seul son raccourci Windows est présent ;
- les commandes Mosquitto laissées en commentaire au début de `main.cpp` ne correspondent pas au chemin MQTT principal de ce TP : le flow Node-RED fourni communique directement avec le broker TTN sur le port 8883 ;
- le commentaire du code indique une attente de 10 secondes dans la boucle, mais l'instruction actuelle est `delay(10000 / 10)`, ce qui correspond en réalité à **1000 ms**.

Ces points n'empêchent pas de comprendre le fonctionnement du projet, mais ils sont utiles si quelqu'un souhaite le reprendre ou le reproduire sur une autre machine.

---

## Résumé

Le projet met en œuvre une chaîne IoT bidirectionnelle complète :

```text
MESURES
DHT11 -> ESP32 -> LoRa-E5 -> LoRaWAN -> TTN -> MQTT -> Node-RED

COMMANDE
Node-RED -> MQTT -> TTN -> Downlink LoRaWAN -> LoRa-E5 -> ESP32 -> LED
```

L'ESP32 s'occupe de récupérer les mesures et de piloter le LoRa-E5 avec des commandes AT. TTN assure le lien avec le réseau LoRaWAN et expose les données en MQTT. Node-RED sert enfin d'interface de visualisation et de commande.

Le résultat est donc un système dans lequel je peux à la fois **faire remonter des mesures de température et d'humidité à distance** et **envoyer une commande dans l'autre sens pour agir sur l'ESP32**.
