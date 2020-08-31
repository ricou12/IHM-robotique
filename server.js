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
var port = 4000;

// This section is optional and used to configure twig.
app.set("twig options", {
    allow_async: true, // Allow asynchronous compiling
    strict_variables: false
});


// AJOUTE UN MIDDLEWARE (autorise le chargement de fichiers static)
app.use(express.static('assets'));
app.use(express.static('node_modules/jquery/dist'));
app.use(express.static('node_modules/bootstrap/dist/js'));
// app.use(express.static('node_modules/@popperjs/core/dist/cjs'));

// DESACTIVATION DU PROTOCOLE CORS
app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    // authorized headers for preflight requests
    // https://developer.mozilla.org/en-US/docs/Glossary/preflight_request
    res.header('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept');
    res.header('Access-Control-Allow-Methods', 'GET, POST, PATCH, PUT, DELETE, OPTIONS');
    next();
});

// ------- GESTION DES ROUTES ---------
// Chargement de la page
app.get('/', function (req, res) {
    res.render('index.html.twig', {
        title: "IHM-robotique"
    });
    console.log('Page chargée');
});

// DEFINIT L'ACTION A REALISER ET ENVOIE DE LA COMMANDE VIA LE PORT SERIE (WRITE)
app.get('/:action', function (req, res) {
    message = '';
    let action = req.params.action || req.param('action');
    serialPortForArduino.write(action);
    io.sockets.emit('messageFromServer', action);
});

const myserver = app.listen(port, function () {
    console.log('Connection avec le server établit : IP pour l\'interface de commande http://10.3.141.1:' + port + ' !');
});


/* *******************************************************************
                            SOCKET.IO
******************************************************************* */
// Chargement de socket.io
let io = require('socket.io')(myserver);

io.sockets.on('connection', function (socket) {
    // QUAND UN CLIENT SE CONNECTE
    // on le note dans la console
    console.log('Un client est connecté !');

    // On informe le client qu'il est connecté
    socket.emit('messageFromServer', 'Vous êtes connecté au serveur !');

    // Envoi l'état du port sérial au client
    console.log(MessageOfSerialPort);
    socket.emit('messageFromServer', MessageOfSerialPort);

    // On écoute les requetes du client et on envoie la commande à l'arduino
    socket.on('commande', function (action) {
        console.log('message du client : ' + action);
        serialPortForArduino.write(action);
    });
});

// Remove the socket when it closes
// io.sockets.on('close', function () {
//     console.log('socket', socketId, 'closed');
//     socket.emit('messageFromServer','Fermeture du server !');
//     delete io.sockets[socketId];
// });

// Close the server
// myserver.close(function () { console.log('Server closed!'); });
// // Destroy all open sockets
// for (var socketId in io.sockets) {
//   console.log('socket', socketId, 'destroyed');
//   io.sockets[socketId].destroy();
// }

/* *******************************************************************
                            SERIAL PORT
******************************************************************* */
// LIBRAIRIE POUR COMMUNIQUER PORT SERIE (Node SerialPort)
// https://serialport.io/
// PORT COM WINDOWS
// let PortCOM = "COM5";
// PORT COM RASPBERRY
// const PortCOM = "/dev/ttyACM0";

let serialPortForArduino;
let MessageOfSerialPort;

// LISTE + INFOS DES PORTS SERIES, FONCTION ASYNCHRONE
function listPorts() {
    return SerialPort.list().then(
        ports => {
            if (ports.length > 0) {
                let PortCOM = "";
                // console.log("Liste des ports séries: ", ports);
                console.log("Auto-détection de l'arduino !");
                // MessageOfSerialPort = "Auto-détection de l'arduino !";
                io.sockets.emit('messageFromServer', "Auto-détection de l'arduino !");
                ports.forEach(port => {
                    // Numéro de serie de l'arduino mega
                    if (port.serialNumber == '5563530373835180F090') {
                        console.log("arduino détecté, serialNumber: " + port.serialNumber);
                        io.sockets.emit('messageFromServer', "arduino détecté, serialNumber: " + port.serialNumber);
                        PortCOM = port.path;
                    }
                });
                if (PortCOM != "") {
                    return {
                        'state': true,
                        'PortCOM': PortCOM
                    }
                } else {
                    detectSerialPort();
                    return {
                        'state': false,
                        'PortCOM': ''
                    };
                }
            } else {
                console.log('Aucun port série disponible ou non spécifié.');
                io.sockets.emit('messageFromServer', 'Aucun port série disponible ou non spécifié.');
                // MessageOfSerialPort =  'Aucun port série disponible ou non spécifié.';
                detectSerialPort();
                return {
                    'state': false,
                    'PortCOM': ''
                };
            }
        },
        err => {
            console.error('Error listing ports', err);
            io.sockets.emit('messageFromServer', 'Error listing ports', err);
            return {
                'state': false,
                'PortCOM': ''
            };
        }
    )
}

// INITISALISE LE PORT SERIE
let ConnectSerialPort = (PortCOM) => {
    // todo envoyer la liste des ports serie pour la connection

    // INITIALISE LA COMMUNICATION 
    serialPortForArduino = new SerialPort(PortCOM, {
        baudRate: 9600,
    });

    // PARAMETRAGE DES DONNEES RECUES VIA LE PORT SERIE (delimiter :  \n saut de ligne, \r retour chariot, \t tabulation "   ")  
    const parser = serialPortForArduino.pipe(new Readline({
        delimiter: '\n',
    }));

    // OUVERTURE DU PORT SERIE  
    serialPortForArduino.on('open', function () {
        MessageOfSerialPort = 'Le port serie ' + PortCOM + ' est ouvert.';
        console.log(MessageOfSerialPort);
        io.sockets.emit('messageFromServer',MessageOfSerialPort);
    });

    // ERREUR D'OUVERTURE DU PORT SERIE  
    serialPortForArduino.on('error', function () {
        MessageOfSerialPort = 'Erreur lors de l\'ouverture du port : ' + PortCOM + '.';
        console.error(MessageOfSerialPort);
        io.sockets.emit('messageFromServer',MessageOfSerialPort);
    });

    // FERMETURE DU PORT SERIE  
    serialPortForArduino.on('close', error => {
        MessageOfSerialPort = 'Le port serie ' + PortCOM + ' à été fermé.';
        console.log(MessageOfSerialPort);
        io.sockets.emit('messageFromServer',MessageOfSerialPort);
        // surveille la reconnection du port série
        detectSerialPort();
    });

    // AFFICHE LES DONNEES RECU VIA LE PORT SERIE
    parser.on('data', data => {
        console.log("Arduino émission de données :  " + data);
        io.sockets.emit('messageFromServer', "Arduino émission de données :  " + data);
    });
}

// FONCTION ASYNCHRONE ATTENDS LA LISTE PUIS SE CONNECTE AU PORT SERIE SI IL EXISTE.
async function asyncCall() {
    // Attends la liste des ports séries
    const updateListPorts = await listPorts();
    // Si l'arduino est connecté.
    if (updateListPorts.state) {
        ConnectSerialPort(updateListPorts.PortCOM);
    }
}

// surveille la reconnection du port série
const detectSerialPort = () => {
    setTimeout(function(){  asyncCall(); },2000);
}

// FONCTION EXECUTE AU CHARGEMENT DU SCRIPT
asyncCall();