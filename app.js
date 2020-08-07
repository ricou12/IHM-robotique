// node server.js

/* *******************************************************************
  INITIALISATION DES VARIABLES 
******************************************************************* */
let Twig = require("twig"),
    express = require('express'),
    SerialPort = require('serialport'),
    Readline = require('@serialport/parser-readline');

/* *******************************************************************
  FRAMEWORK STANDARD POUR LE DEVELOPPEMENT DE SERVEUR EN NODE.JS
******************************************************************* */
// https://www.npmjs.com/package/express || http://expressjs.com/
let app = express();
var port = 3000;

// This section is optional and used to configure twig.
app.set("twig options", {
    allow_async: true, // Allow asynchronous compiling
    strict_variables: false
});

// AJOUTE UN MIDDLEWARE (autorise le chargement de fichiers static)
app.use(express.static('assets'));

// ------- GESTION DES ROUTES ---------
// Chargement de la page
app.get('/', function (req, res) {
    // return res.send('<h2>Bienvenue !</h2>');
    res.render('index.html.twig', {
        title: "Surveillance vidéo"
    });
    console.log('page chargée');
});

// DESACTIVATION DU PROTOCOLE CORS
app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    // authorized headers for preflight requests
    // https://developer.mozilla.org/en-US/docs/Glossary/preflight_request
    res.header('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept');
    res.header('Access-Control-Allow-Methods', 'GET, POST, PATCH, PUT, DELETE, OPTIONS');
    next();
});

const myserver = app.listen(port, function () {
    console.log('Connection avec le server établit : URL pour l\'interface de commande http://localhost:' + port + ' !');
});


/* *******************************************************************
                            SOCKET.IO
******************************************************************* */
// Chargement de socket.io
let io = require('socket.io').listen(myserver);

io.sockets.on('connection', function (socket) {
    // Quand un client se connecte, on le note dans la console
    console.log('Un client est connecté !');

    // Quand un client se connecte, on envoie un message
    socket.emit('messageServer', 'Vous êtes bien connecté !');

    // On écoute les requetes du client 
    socket.on('commande', function (message) {
        console.log('message du client : ' + message);
        switch (message) {
            case 'camera_Haut':
                arduinoSerialPort.write("CamUp");
                message = "Regarde en haut";
                break;
    
            case 'camera_Bas':
                arduinoSerialPort.write("CamDown");
                message = "Regarde en bas";
                break;
    
            case 'camera_Gauche':
                arduinoSerialPort.write("CamLeft");
                message = "Regarde à gauche";
                break;
    
            case 'camera_Centre':
                arduinoSerialPort.write("CamCenter");
                message = "Centrage de la caméra";
                break;
    
            case 'camera_Droite':
                arduinoSerialPort.write("CamRight");
                message = "Regarde à droite";
                break;
    
            case 'camera_Scan-X':
                arduinoSerialPort.write("CamScanX");
                message = "Regarde de gauche à droite";
                break;
    
            case 'camera_Scan-Y':
                arduinoSerialPort.write("CamScanY");
                message = "Regarde de haut en bas";
                break;
    
            case 'moteur_Stop':
                arduinoSerialPort.write("MotorsStop");
                message = "Arrêt du véhicule";
                break;
    
            case 'moteur_Avancer':
                arduinoSerialPort.write("MotorsForward");
                message = "Déplacement vers l'avant";
                break;
    
            case 'moteur_Reculer':
                arduinoSerialPort.write("MotorsBackward");
                message = "Déplacement marche arrière";
                break;
    
            case 'moteur_Gauche':
                arduinoSerialPort.write("MotorsLeft");
                message = "Déplacement à gauche";
                break;
    
            case 'moteur_AvGauche':
                arduinoSerialPort.write("MotorsFrontLeft");
                message = "Déplacement avant gauche";
                break;
    
            case 'moteur_Droite':
                arduinoSerialPort.write("MotorsRight");
                message = "Déplacement à droite";
                break;
    
            case 'moteur_ArGauche':
                arduinoSerialPort.write("MotorsBackLeft");
                message = "Déplacement arrière gauche";
                break;
    
            case 'moteur_AvDroite':
                arduinoSerialPort.write("MotorsFrontRight");
                message = "Déplacement avant droite";
                break;
    
            case 'moteur_ArDroite':
                arduinoSerialPort.write("MotorsBackRight");
                message = "Déplacement arrière droite";
                break;
            
            default:
                console.log(`Désolé cette action n'est pas reconnue : ${action}.`);
        }
        socket.emit('messageServer',message);
    });
});


/* *******************************************************************
                            SERIAL PORT
******************************************************************* */
// LIBRAIRIE POUR COMMUNIQUER PORT SERIE (Node SerialPort)
// https://serialport.io/
const arduinoCOMPort = "COM5";
let infoSerailPort = "";

// INITIALISE LA COMMUNICATION 
let arduinoSerialPort = new SerialPort(arduinoCOMPort, {
    baudRate: 9600,
});

// PARAMETRAGE DES DONNEES RECUES VIA LE PORT SERIE (delimiter :Longueur de la chaine \n saut de ligne, \r retour chariot)  
const parser = arduinoSerialPort.pipe(new Readline({
    delimiter: '\n',
}));

// AFFICHE LES DONNEES RECU VIA LE PORT SERIE
parser.on('data', data => {
    infoSerailPort = data;
    console.log('Arduino émission de données : ', data);
    socket.emit('sendSerialPort : ', data);
});

// OUVERTURE DU PORT SERIE  
arduinoSerialPort.on('open', function () {
    infoSerailPort = 'Le port serie ' + arduinoCOMPort + ' est ouvert.';
    console.log(infoSerailPort);
});