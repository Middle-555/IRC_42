#!/bin/bash

echo "🔍 Recherche des processus IRC en cours..."

# Trouver tous les processus qui correspondent à "irc" sauf grep lui-même
PIDS=$(ps aux | grep "[i]rc" | awk '{print $2}')

if [ -z "$PIDS" ]; then
    echo "✅ Aucun processus IRC trouvé."
else
    echo "⚠ Processus trouvés : $PIDS"
    echo "⏳ Arrêt en cours..."
    kill -9 $PIDS
    sleep 1  # Laisser le temps aux processus de se terminer
fi

# Vérifier si le port 6667 est toujours occupé
if lsof -i :6667 > /dev/null 2>&1; then
    echo "⚠ Le port 6667 est encore utilisé. Tentative de libération..."
    
    # Trouver et tuer le processus qui utilise le port 6667
    PORT_PID=$(lsof -ti :6667)
    if [ ! -z "$PORT_PID" ]; then
        kill -9 $PORT_PID
        echo "✅ Port 6667 libéré avec succès."
    else
        echo "❌ Impossible de libérer le port 6667."
        exit 1
    fi
else
    echo "✅ Le port 6667 est libre."
fi

echo "✅ Tous les processus IRC ont été arrêtés et le port 6667 est prêt."
