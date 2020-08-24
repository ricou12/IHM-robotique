const images = [{
        'id': 'moteur_AvGauche',
        'modifSpeed': true,
        'default': 'hautGauche.png',
        'hover': 'hautGaucheCl.png',
        'action': 'MotorsFrontLeft'
    },
    {
        'id': 'moteur_Avancer',
        'modifSpeed': true,
        'default': 'avancer.png',
        'hover': 'avancerCl.png',
        'action': 'MotorsForward'
    },
    {
        'id': 'moteur_AvDroite',
        'modifSpeed': true,
        'default': 'hautDroite.png',
        'hover': 'hautDroiteCl.png',
        'action': 'MotorsFrontRight'
    },
    {
        'id': 'moteur_Gauche',
        'modifSpeed': true,
        'default': 'gauche.png',
        'hover': 'gaucheCl.png',
        'action': 'MotorsLeft'
    },
    {
        'id': 'moteur_Stop',
        'modifSpeed': true,
        'default': 'centrer.png',
        'hover': 'centrerCl.png',
        'action': 'MotorsStop'
    },
    {
        'id': 'moteur_Droite',
        'modifSpeed': true,
        'default': 'droite.png',
        'hover': 'droiteCl.png',
        'action': 'MotorsRight'
    },
    {
        'id': 'moteur_ArGauche',
        'modifSpeed': true,
        'default': 'basGauche.png',
        'hover': 'basGaucheCl.png',
        'action': 'MotorsBackLeft'
    },
    {
        'id': 'moteur_Reculer',
        'modifSpeed': true,
        'default': 'reculer.png',
        'hover': 'reculerCl.png',
        'action': 'MotorsBackward'
    },
    {
        'id': 'moteur_ArDroite',
        'modifSpeed': true,
        'default': 'basDroite.png',
        'hover': 'basDroiteCl.png',
        'action': 'MotorsBackRight'
    },
    {
        'id': 'camera_Haut',
        'modifSpeed': false,
        'default': 'avancer.png',
        'hover': 'avancerCl.png',
        'action': 'CamUp'
    },
    {
        'id': 'camera_Gauche',
        'modifSpeed': false,
        'default': 'gauche.png',
        'hover': 'gaucheCl.png',
        'action': 'CamLeft'
    },
    {
        'id': 'camera_Centre',
        'modifSpeed': false,
        'default': 'centrer.png',
        'hover': 'centrerCl.png',
        'action': 'CamCenter'
    },
    {
        'id': 'camera_Droite',
        'modifSpeed': false,
        'default': 'droite.png',
        'hover': 'droiteCl.png',
        'action': 'CamRight'
    },
    {
        'id': 'camera_Bas',
        'modifSpeed': false,
        'default': 'reculer.png',
        'hover': 'reculerCl.png',
        'action': 'CamDown'
    },
    {
        'id': 'camera_Scan-X',
        'modifSpeed': false,
        'default': 'scanX.png',
        'hover': 'scanXCl.png',
        'action': 'CamScanX'
    },
    {
        'id': 'camera_Scan-Y',
        'modifSpeed': false,
        'default': 'scanY.png',
        'hover': 'scanYCl.png',
        'action': 'CamScanY'
    },
];


let last_action = "";
// Dossier contenant les images des boutons de commandes
const path = "/images/";
const icons = document.querySelectorAll('.icon');
const dataServer = document.getElementById('dataServer');
const dataSerialPort = document.getElementById('dataSerialPort');

/* *****************************
            SOCKET
***************************** */
let socket = io.connect('//:3000');

// messages renvoyés par le serveur.
socket.on('messageServer', function (message) {
    dataServer.innerHTML += message + '<br>';
    dataServer.scrollTop = dataServer.scrollHeight;
});

// message envoyé par l'arduino au server node via le port serie.
socket.on('sendSerialPort', function (message) {
    dataSerialPort.innerHTML += message + '<br>';
    dataSerialPort.scrollTop = dataSerialPort.scrollHeight;
});

/* ************************************************************************************
    ENVOIE DES COMMANDES AU SERVER "app.js" PUIS TRANSFERT A L4ARDUINO VIA PORT SERIE
* *************************************************************************************/
// Parcours l'ensemble des boutons de contrôle commande pour :
// Recupèrer l'action a envoyer au server
// et modifier l'image lorsque le bouton de la souris est apuuyé puis relaché.
icons.forEach(icon => {
    // Ecoute de l'évenement appuyer sur le bouton de la souris
    icon.addEventListener('mousedown', event => {
        // Récupère la valeur de l'attribut id du bouton de commande
        let id = icon.getAttribute('data-id');
        // Parcours le tableau.
        images.forEach(element => {
            if (element.id == id) {
                // Récupere l'image pour l'évenement mousedown et l'affiche.
                icon.setAttribute('src', path + element.hover);
                // Verifie si la commande gère la vitesse
                if (element.modifSpeed == true) {
                    // enregistre la dernière action pour l'entrainement des moteurs
                    last_action = element.action;
                    /* Récupère l'action et le mode vitesse (lent ou rapide) pour l'entrainement des roues et l'envoie au serveur
                        qui sera ensuite envoyé par le serveur via le port serie à l'arduino */
                    // La chaine sera analysé par l'arduino grace au délimiter
                    // Délimiter pour l'action se termine par : ";"
                    // Délimiter pour la vitesse se termine par un retour à la ligne : "\n".
                    socket.emit('commande', element.action + ';' + slider.value + '\n');
                } else {
                    socket.emit('commande', element.action + ";\n");
                }
            }
        });
    });
    // Ecoute de l'évenement relacher le bouton de la souris
    icon.addEventListener('mouseup', event => {
        // Récupère la valeur de l'attribut id du bouton de commande
        let id = icon.getAttribute('data-id');
        // Récupere l'image pour l'évenement mouseup.
        images.forEach(element => {
            if (element.id == id) {
                icon.setAttribute('src', path + element.default);
            }
        });
    });
});

/* ****************************************************************************
 * SLIDER (Variateur de vitesse pour moteurs d'entrainement des roues)
 ***************************************************************************** */
// ecoute l'evenement radio (modification de la vitesse)
const slider = document.querySelector('.slider-moteurs');
slider.addEventListener('change', event => {
    // Commande envoyé au serveur
    socket.emit('commande', last_action + ';' + slider.value + '\n');
});
/* ***************************************************************************** */


document.getElementById("execAction").addEventListener('click', event => {
    const myAction = document.getElementById("myAction").value;
    // Commande envoyé au serveur
    socket.emit('commande', myAction);
});

// CHARGEMENT DE LA PAGE HTML ET DEMARRAGE DES ANIMATIONS(alternative à load)
document.onreadystatechange = function () {
    if (document.readyState == "complete") {
        $("#myConsole").addClass('started');
        $("#rpiCam").addClass('started');
        setTimeout(function () {
            $("#container__boxMoteur").removeClass("cacher");
            $("#container__boxMoteur").addClass('derouler');
            setTimeout(function () {
                $("#container__boxCamera").removeClass("cacher");
                $("#container__boxCamera").addClass('derouler');
                setTimeout(function () {
                    $("#container__boxScanCam").removeClass("cacher");
                    $("#container__boxScanCam").addClass('derouler');
                }, 500);
            }, 500);
        }, 1000);
    }
}