#!/bin/bash

# Configuration du serveur
SERVER_IP="127.0.0.1"
SERVER_PORT="6667"
PASSWORD="mypassword"

echo "🚀 Nettoyage et compilation du projet..."
cd ..
make fclean > /dev/null 2>&1
make > /dev/null 2>&1

if [ ! -f "./irc" ]; then
    echo "❌ Erreur : La compilation a échoué, l'exécutable du serveur est introuvable."
    exit 1
fi

echo "✅ Compilation réussie."

# Vérifier si un serveur tourne déjà sur le port 6667 et le tuer
PID=$(lsof -ti :$SERVER_PORT)
if [ ! -z "$PID" ]; then
    echo "⚠ Un serveur utilise déjà le port $SERVER_PORT. Fermeture..."
    kill -9 $PID
    sleep 2  # Attendre que le port soit libéré
fi

# Vérifier que le port est bien libéré
if lsof -i :$SERVER_PORT > /dev/null; then
    echo "❌ Erreur : Le port $SERVER_PORT est toujours occupé, impossible de démarrer le serveur."
    exit 1
fi

# Lancer le serveur en arrière-plan
SERVER_EXEC="./irc"
$SERVER_EXEC $SERVER_PORT $PASSWORD &  
SERVER_PID=$!  
sleep 1  

# Vérifier si le serveur s'est bien lancé
if ! ps -p $SERVER_PID > /dev/null; then
    echo "❌ Erreur : Le serveur n'a pas pu démarrer."
    exit 1
fi

echo "✅ Serveur IRC en écoute sur le port $SERVER_PORT"

echo "📢 Ouvre un **nouvel onglet** et exécute la commande suivante pour te connecter :"
echo "   nc $SERVER_IP $SERVER_PORT"
echo ""

sleep 2  # Attendre avant d'envoyer les commandes

# Envoyer les commandes au serveur
echo -e "PASS $PASSWORD\nNICK TestUser\nUSER testuser 0 * :Test User\n" | nc $SERVER_IP $SERVER_PORT

if ps -p $SERVER_PID > /dev/null; then
    echo "✅ Test réussi : Connexion et authentification fonctionnent !"
else
    echo "❌ Erreur : Le serveur s'est arrêté de manière inattendue."
fi

# Arrêter le serveur proprement
kill $SERVER_PID 2>/dev/null
