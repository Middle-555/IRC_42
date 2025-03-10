/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:24:18 by kpourcel          #+#    #+#             */
/*   Updated: 2025/03/10 17:33:54 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

/**
 * @brief Constructeur du serveur IRC.
 *
 * Initialise le serveur en créant un socket, en le configurant avec une adresse 
 * et un port, puis en le mettant en mode écoute pour accepter les connexions clients.
 * Utilise `poll()` pour gérer plusieurs clients simultanément sans blocage.
 *
 * @param port Le port sur lequel le serveur écoute les connexions.
 * @param password Le mot de passe requis pour se connecter au serveur.
 *
 * @throws EXIT_FAILURE en cas d'erreur lors de la création du socket, du bind ou du listen.
 */
Server::Server(int port, std::string password)
    : port(port), password(password), commandHandler(*this) {  // ✅ Initialisation du commandHandler
    struct sockaddr_in serverAddr;
    
    // Création du socket serveur
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Erreur socket()");
        exit(EXIT_FAILURE);
    }

    // Configuration du serveur
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Bind du socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Erreur bind()");
        exit(EXIT_FAILURE);
    }

    // Mise en écoute
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Erreur listen()");
        exit(EXIT_FAILURE);
    }

    std::cout << "🔵 Serveur IRC en écoute sur le port " << port << std::endl;

    // Ajout du socket serveur à poll()
    struct pollfd serverPollFd;
    serverPollFd.fd = serverSocket;
    serverPollFd.events = POLLIN;
    pollFds.push_back(serverPollFd);
}

/**
 * @brief Destructeur du serveur IRC.
 *
 * Ferme proprement le serveur en :
 * - Fermant le socket du serveur.
 * - Fermant tous les sockets des clients connectés.
 * - Libérant la mémoire allouée pour chaque client.
 * - Affichant un message indiquant l'arrêt du serveur.
 */
Server::~Server() {
    close(serverSocket);
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        close(it->first);
        delete it->second;
    }
    std::cout << "🔴 Serveur arrêté." << std::endl;
}

/**
 * @brief Boucle principale du serveur IRC.
 *
 * Gère les connexions et les communications avec les clients en utilisant `poll()`.
 * - Attend des événements sur les sockets (nouvelle connexion ou message client).
 * - Accepte les nouvelles connexions lorsqu'un client tente de se connecter.
 * - Traite les messages des clients déjà connectés.
 *
 * @throws EXIT_FAILURE en cas d'erreur sur `poll()`.
 */
void Server::run() {
    while (true) {
        for (size_t i = 0; i < pollFds.size(); ++i) {
            if (pollFds[i].fd < 0) {
                std::cerr << "❌ Erreur: pollFds[" << i << "].fd est invalide !\n";
                pollFds.erase(pollFds.begin() + i); // Supprime l'entrée invalide
                i--; // Ajuste l'index après suppression
            }
            pollFds[i].revents = 0;  // ✅ Évite un comportement indéfini
        }
        
        // Attente d'événements sur les sockets
        int ret = poll(pollFds.data(), pollFds.size(), -1);
        if (ret < 0) {
            exit(EXIT_FAILURE);
        }

        // Parcours des événements
        for (size_t i = 0; i < pollFds.size(); ++i) {
            if (pollFds[i].revents & POLLIN) {
                if (pollFds[i].fd == serverSocket) {
                    handleNewConnection();
                } else {
                    handleClientMessage(pollFds[i].fd);
                }
            }
        }
    }
}

/**
 * @brief Gère l'arrivée d'une nouvelle connexion client.
 *
 * - Accepte la connexion d'un nouveau client avec `accept()`.
 * - Affiche un message indiquant qu'un client s'est connecté.
 * - Ajoute le client à la liste des sockets surveillés par `poll()`.
 * - Crée un nouvel objet `Client` pour stocker ses informations.
 *
 * @throws perror() si `accept()` échoue.
 */
void Server::handleNewConnection() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

    if (clientSocket < 0) {
        perror("❌ Erreur accept()");
        return;
    }

    std::cout << "🟢 Nouveau client connecté : " << inet_ntoa(clientAddr.sin_addr) << " (fd: " << clientSocket << ")" << std::endl;

    // ✅ Vérification du socket avant de l'ajouter à pollFds
    if (clientSocket < 0) {
        std::cerr << "❌ Erreur: Socket client invalide !\n";
        return;
    }

    // ✅ Initialisation complète du pollfd
    struct pollfd clientPollFd;
    clientPollFd.fd = clientSocket;
    clientPollFd.events = POLLIN;
    clientPollFd.revents = 0;

    pollFds.push_back(clientPollFd);
    
    // ✅ Vérifier si l'allocation mémoire est réussie
    clients[clientSocket] = new Client(clientSocket);
    if (!clients[clientSocket]) {
        std::cerr << "❌ Erreur : Impossible d'allouer un Client pour fd " << clientSocket << std::endl;
        close(clientSocket);
        return;
    }
    // ✅ Envoi d'un message d'accueil au client
    std::string welcomeMessage = "Bienvenue sur le serveur IRC !\n";
    welcomeMessage += "Veuillez entrer les informations demandées :\n";
    welcomeMessage += "Mot de passe du serveur : ";

    int bytesSent = send(clientSocket, welcomeMessage.c_str(), welcomeMessage.length(), 0);
    if (bytesSent < 0) {
        perror("❌ Erreur send()");
        close(clientSocket);
        delete clients[clientSocket];
        clients.erase(clientSocket);
        return;
    }
}


/**
 * @brief Gère la réception d'un message d'un client.
 *
 * - Lit les données envoyées par le client avec `recv()`.
 * - Si le client se déconnecte (`bytesRead <= 0`), il est supprimé de la liste des clients.
 * - Affiche le message reçu dans la console.
 * - (Prévu pour être étendu avec un parseur de commandes IRC).
 *
 * @param clientSocket Le descripteur de fichier du client envoyant le message.
 */
void Server::handleClientMessage(int clientSocket) {
    char buffer[512];
    memset(buffer, 0, sizeof(buffer));
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0) {
        removeClient(clientSocket);
        return;
    }

    std::string message(buffer);
    message.erase(message.find_last_not_of("\r\n") + 1); // Nettoyage des sauts de ligne

    std::cout << "📩 Message reçu de " << clientSocket << " : " << message << std::endl;

    // Vérifier si le client existe
    if (clients.find(clientSocket) == clients.end()) {
        return;
    }

    // Vérifier si c'est une commande IRC
    if (message[0] == '/') {
        message = message.substr(1); // Supprime le "/"
    }

    // Debug : Afficher la commande analysée
    std::cout << "🔍 Commande détectée : [" << message << "]\n";

    // ✅ Appeler le gestionnaire de commandes
    commandHandler.handleCommand(clientSocket, message);
}

/**
 * @brief Déconnecte et supprime un client du serveur.
 *
 * - Ferme le socket du client pour libérer la ressource.
 * - Supprime l'objet `Client` correspondant et le retire de la liste des clients connectés.
 * - Supprime le client de la liste des sockets surveillés par `poll()`.
 * - Affiche un message indiquant la déconnexion du client.
 *
 * @param clientSocket Le descripteur de fichier du client à supprimer.
 */
void Server::removeClient(int clientSocket) {
    // Vérifie si le client existe avant de tenter de le supprimer
    if (clients.find(clientSocket) == clients.end()) {
        return;
    }

    std::cout << std::endl;
    std::cout << "🔴 Client déconnecté." << std::endl;
    close(clientSocket);

    // Supprimer le client en toute sécurité
    delete clients[clientSocket];  
    clients.erase(clientSocket);

    // Retirer de pollFds
    for (size_t i = 0; i < pollFds.size(); ++i) {
        if (pollFds[i].fd == clientSocket) {
            pollFds.erase(pollFds.begin() + i);
            break;
        }
    }
}


/**
 * @brief Gère la commande PASS pour l'authentification du client.
 *
 * - Vérifie si le client est déjà authentifié. Si oui, renvoie une erreur.
 * - Compare le mot de passe fourni avec celui du serveur.
 * - Si le mot de passe est incorrect, envoie une erreur et déconnecte le client.
 * - Si le mot de passe est correct, marque le client comme authentifié et envoie une confirmation.
 *
 * @param clientSocket Le descripteur de fichier du client.
 * @param password Le mot de passe fourni par le client.
 */
void Server::handlePass(int clientSocket, const std::string& password) {
    if (clients.find(clientSocket) == clients.end()) {
        return;
    }

    if (clients[clientSocket]->isAuthenticated()) {
        send(clientSocket, "ERROR :Already registered\r\n", 27, 0);
        return;
    }

    // Vérifier si le mot de passe est correct
    if (password != this->password) {
        send(clientSocket, "ERROR :Incorrect password\r\n", 30, 0);
        removeClient(clientSocket);
        return;
    }

    clients[clientSocket]->authenticate();
    send(clientSocket, "✅ OK :Password accepted\r\n", 28, 0);
}





/**
 * @brief Gère la commande NICK pour définir le pseudo du client.
 *
 * - Vérifie si le client est authentifié. Si non, renvoie une erreur.
 * - Met à jour le pseudo du client avec la valeur fournie.
 * - Envoie une confirmation au client avec le nouveau pseudo.
 *
 * @param clientSocket Le descripteur de fichier du client.
 * @param nickname Le pseudo choisi par le client.
 */
void Server::handleNick(int clientSocket, const std::string& nickname) {
    if (!clients[clientSocket]->isAuthenticated()) {
        send(clientSocket, "ERROR :You need to authenticate first\r\n", 39, 0);
        return;
    }

    clients[clientSocket]->setNickname(nickname);
    std::string response = "OK :Nickname set to " + nickname + "\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);
}

/**
 * @brief Gère la commande USER pour définir les informations utilisateur du client.
 *
 * - Vérifie si le client est authentifié. Si non, renvoie une erreur.
 * - Met à jour le nom d'utilisateur et le real name du client avec les valeurs fournies.
 * - Envoie une confirmation au client indiquant que les informations ont été mises à jour.
 *
 * @param clientSocket Le descripteur de fichier du client.
 * @param username Le nom d'utilisateur défini par le client.
 * @param realname Le real name défini par le client.
 */
void Server::handleUser(int clientSocket, const std::string& username, const std::string& realname) {
    if (!clients[clientSocket]->isAuthenticated()) {
        send(clientSocket, "ERROR :You need to authenticate first\r\n", 39, 0);
        return;
    }

    clients[clientSocket]->setUsername(username);
    clients[clientSocket]->setRealname(realname);

    std::string response = "OK :Username and realname set\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);
}

/**
 * @brief Gère la commande JOIN pour qu'un client rejoigne un channel.
 *
 * - Vérifie si le channel existe déjà ou le crée.
 * - Ajoute le client à la liste des membres du channel.
 * - Informe les autres membres de la présence du nouveau client.
 *
 * @param clientSocket Le descripteur du client.
 * @param channelName Le nom du channel à rejoindre.
 */
void Server::handleJoin(int clientSocket, const std::string& channelName) {
    if (channelName.empty()) {
        send(clientSocket, "ERROR :No channel name provided\r\n", 33, 0);
        return;
    }

    if (channels.find(channelName) == channels.end()) {
        channels[channelName] = new Channel(channelName);
    }

    Channel *channel = channels[channelName];
    channel->addClient(clientSocket);
    clients[clientSocket]->setCurrentChannel(channelName);

    std::string joinMsg = ":" + clients[clientSocket]->getNickname() + " JOIN " + channelName + "\r\n";

    std::cout << "🔹 Envoi du message JOIN à " << clients[clientSocket]->getNickname() << " : " << joinMsg;
    std::cout << std::endl;

    int bytesSent = send(clientSocket, joinMsg.c_str(), joinMsg.length(), 0);
    if (bytesSent == -1) {
        perror("❌ Erreur lors de l'envoi du message JOIN");
    } else {
        std::cout << "✅ Message JOIN envoyé avec succès (" << bytesSent << " bytes)\n";
    }

    // Envoyer le message aux autres membres du channel
    channel->broadcast(joinMsg, clientSocket);
}




/**
 * @brief Gère la commande PART pour qu'un client quitte un channel.
 *
 * - Vérifie si le client est dans le channel.
 * - Le retire du channel et en informe les autres membres.
 * - Si le channel est vide, il est supprimé.
 *
 * @param clientSocket Le descripteur du client.
 * @param channelName Le nom du channel à quitter.
 */
void Server::handlePart(int clientSocket, const std::string& channelName) {
    if (channels.find(channelName) == channels.end()) {
        send(clientSocket, "ERROR :No such channel\r\n", 24, 0);
        return;
    }

    Channel *channel = channels[channelName];
    if (!channel->isClientInChannel(clientSocket)) {
        send(clientSocket, "ERROR :You're not in this channel\r\n", 36, 0);
        return;
    }

    std::string partMsg = ":" + clients[clientSocket]->getNickname() + " PART " + channelName + "\r\n";

    std::cout << "🔹 Envoi message PART au client : " << partMsg << std::endl;
    int bytesSent = send(clientSocket, partMsg.c_str(), partMsg.length(), 0);
    if (bytesSent == -1) {
        perror("❌ Erreur lors de l'envoi du message PART");
    } else {
        std::cout << "✅ Message PART envoyé avec succès (" << bytesSent << " bytes)\n";
    }

    channel->broadcast(partMsg, clientSocket);

    channel->removeClient(clientSocket);
    clients[clientSocket]->setCurrentChannel("");

    std::cout << "✅ Client " << clients[clientSocket]->getNickname() << " a quitté " << channelName << std::endl;

    if (channel->isEmpty()) {
        delete channel;
        channels.erase(channelName);
    }
}


/**
 * @brief Trouve le socket d'un client à partir de son pseudo.
 *
 * @param nickname Le pseudo du client à rechercher.
 * @return int Le descripteur de fichier du client, ou -1 si introuvable.
 */
int Server::getClientSocketByNickname(const std::string& nickname) const {
    for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->getNickname() == nickname) {
            return it->first;
        }
    }
    return -1; // Retourne -1 si aucun client ne correspond
}

/**
 * @brief Gère la commande PRIVMSG pour envoyer un message privé.
 *
 * - Vérifie si la cible est un channel ou un utilisateur.
 * - Envoie le message à tous les membres du channel ou au destinataire direct.
 *
 * @param clientSocket Le descripteur du client envoyant le message.
 * @param target Le destinataire (pseudo ou channel).
 * @param message Le message à envoyer.
 */
void Server::handlePrivMsg(int clientSocket, const std::string& target, const std::string& message) {
    if (target.empty() || message.empty()) {
        send(clientSocket, "ERROR :Invalid PRIVMSG format\r\n", 31, 0);
        return;
    }

    if (channels.find(target) != channels.end()) {
        // Envoyer le message à tous les membres du channel
        Channel *channel = channels[target];
        std::string msg = ":" + clients[clientSocket]->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
        channel->broadcast(msg, clientSocket);
    } else {
        // Trouver le socket du client à partir de son pseudo
        int targetSocket = getClientSocketByNickname(target);
        if (targetSocket == -1) {
            send(clientSocket, "ERROR :No such nick\r\n", 21, 0);
            return;
        }

        // Envoyer le message au client spécifique
        std::string msg = ":" + clients[clientSocket]->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
        send(targetSocket, msg.c_str(), msg.length(), 0);
    }
}

void Server::handleList(int clientSocket) {
    std::string listMsg = "Active channels:\r\n";
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
        listMsg += "- " + it->first + "\r\n";
    }
    send(clientSocket, listMsg.c_str(), listMsg.length(), 0);
}

std::map<int, Client*>& Server::getClients() {
    return clients;
}

