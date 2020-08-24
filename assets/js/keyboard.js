let myCommande;

// GESTION DES EVENEMENTS CLAVIER
document.addEventListener('keydown', e => {
    console.log(e.keyCode);
	switch (e.keyCode) {
		case 38 :
            // up
            socket.emit('commande','CamUp');
		  break;

		case 40:
            // down
            
			socket.emit('commande','CamDown');
			break;

			case 37:
			// left
			socket.emit('commande','CamLeft');
			break;

		case 39:
			// right
			socket.emit('commande','CamRight');
		  break;

		case 96:
			// Center
            socket.emit('commande','CamCenter');
            break;

		default:
		  console.log(`Désolé aucune action pour : ${e.keyCode}.`);
	  }	  
});
