
const images = [{
        'setAction': 'moteur_AvGauche',
        'default': 'hautGauche.png',
        'hover': 'hautGaucheCl.png'
    },
    {
        'setAction': 'moteur_Avancer',
        'default': 'avancer.png',
        'hover': 'avancerCl.png'
    },
    {
        'setAction': 'moteur_AvDroite',
        'default': 'hautDroite.png',
        'hover': 'hautDroiteCl.png'
    },
    {
        'setAction': 'moteur_Gauche',
        'default': 'gauche.png',
        'hover': 'gaucheCl.png'
    },
    {
        'setAction': 'moteur_Stop',
        'default': 'centrer.png',
        'hover': 'centrerCl.png'
    },
    {
        'setAction': 'moteur_Droite',
        'default': 'droite.png',
        'hover': 'droiteCl.png'
    },
    {
        'setAction': 'moteur_ArGauche',
        'default': 'basGauche.png',
        'hover': 'basGaucheCl.png'
    },
    {
        'setAction': 'moteur_Reculer',
        'default': 'reculer.png',
        'hover': 'reculerCl.png'
    },
    {
        'setAction': 'moteur_ArDroite',
        'default': 'basDroite.png',
        'hover': 'basDroiteCl.png'
    },
    {
        'setAction': 'camera_Haut',
        'default': 'avancer.png',
        'hover': 'avancerCl.png'
    },
    {
        'setAction': 'camera_Gauche',
        'default': 'gauche.png',
        'hover': 'gaucheCl.png'
    },
    {
        'setAction': 'camera_Centre',
        'default': 'centrer.png',
        'hover': 'centrerCl.png'
    },
    {
        'setAction': 'camera_Droite',
        'default': 'droite.png',
        'hover': 'droiteCl.png'
    },
    {
        'setAction': 'camera_Bas',
        'default': 'reculer.png',
        'hover': 'reculerCl.png'
    },
    {
        'setAction': 'camera_Scan-X',
        'default': 'scanX.png',
        'hover': 'scanXCl.png'
    },
    {
        'setAction': 'camera_Scan-Y',
        'default': 'scanY.png',
        'hover': 'scanYCl.png'
    },
];

// Dossier contenant les images des boutons de commandes
const path = "/images/";
const icons = document.querySelectorAll('.icon');
const dataServer = document.getElementById('dataServer');
const dataSerialPort = document.getElementById('dataSerialPort');

/* *****************************
            SOCKET
***************************** */
let socket = io.connect('http://localhost:3000');

// messages renvoyés par le serveur.
socket.on('messageServer', function(message) {
    dataServer.innerHTML += message + '<br>';
    dataServer.scrollTop = dataServer.scrollHeight;
});

// message envoyé par l'arduino au server node via le port serie'
socket.on('sendSerialPort', function(message) {
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
        // Récupère la valeur de l'attribut action du bouton de commande
        let action = icon.getAttribute('action');
        // Parcours le tableau recupere l'action et change l'image sur l'évenement down et up.
        images.forEach(element => {
            if (element.setAction == action) {
                icon.setAttribute('src', path + element.hover);
                // Commande envoyé au serveur
                socket.emit('commande',element.setAction);
            }
        });
    });
    // Ecoute de l'évenement relacher le bouton de la souris
    icon.addEventListener('mouseup', event => {
        // Récupère la valeur de l'attribut action du bouton de commande
        let action = icon.getAttribute('action');
        // Parcours le tableau recupere l'action etr change l'image sur l'évenement down et up.
        images.forEach(element => {
            if (element.setAction == action) {
                icon.setAttribute('src', path + element.default);
            }
        });
    });
});



