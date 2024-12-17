#env bash
echo "Démarrage du serveur TFTP en écoute sur le port 1069"
echo "Liste des fichiers disponibles:"
ls -l o* z* alt* ensea.png
echo "<Ctrl-c> pour arrêter..."

./tftpd -c -L --address :1069 -s . -u $USER
