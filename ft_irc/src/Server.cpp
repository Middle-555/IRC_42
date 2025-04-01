/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kpourcel <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:24:18 by kpourcel          #+#    #+#             */
/*   Updated: 2025/04/01 15:33:00 by kpourcel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

/* -------------------------------------------------------------------------- */
/*                                Constructeur / Destructeur                  */
/* -------------------------------------------------------------------------- */
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
    : port(port), password(password), commandHandler(*this) {
    struct sockaddr_in serverAddr;
    
    // Création du socket serveur
    serverName = "irc.42server.com";
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

/* -------------------------------------------------------------------------- */
/*                                Gestion du Serveur                          */
/* -------------------------------------------------------------------------- */

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

void Server::shutdownServer() {
    std::cout << "\n🛑 Arrêt du serveur IRC...\n";

    // 🔹 Envoyer un message de fermeture à tous les clients
    std::string shutdownMsg = "ERROR :Server shutting down\r\n";
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        send(it->first, shutdownMsg.c_str(), shutdownMsg.size(), 0);
        close(it->first);
        delete it->second;  // ✅ Libérer chaque client
    }
    clients.clear();  // ✅ Vider la map après suppression

    // 🔹 Libérer la mémoire des canaux
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
        delete it->second;  // ✅ Libérer chaque canal
    }
    channels.clear();  // ✅ Vider la map après suppression

    // 🔹 Assurer la libération des objets internes
    std::cout << "🔄 Nettoyage final des ressources...\n";

    // ✅ Vider et forcer la libération du vecteur pollFds
    pollFds.clear();
    std::vector<pollfd>().swap(pollFds);

    // ✅ Vider et libérer les strings utilisées
    serverName = "";
    password = "";
    std::string().swap(serverName);
    std::string().swap(password);
    std::string().swap(shutdownMsg);
    


    // 🔹 Fermer le socket du serveur
    close(serverSocket);

    std::cout << "✅ Serveur IRC arrêté proprement.\n";
    exit(0);
}

/* -------------------------------------------------------------------------- */
/*                                Gestion des Connexions                      */
/* -------------------------------------------------------------------------- */

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

    struct pollfd clientPollFd;
    clientPollFd.fd = clientSocket;
    clientPollFd.events = POLLIN;
    clientPollFd.revents = 0;

    pollFds.push_back(clientPollFd);
    
    clients[clientSocket] = new Client(clientSocket);
    if (!clients[clientSocket]) {
        std::cerr << "❌ Erreur : Impossible d'allouer un Client pour fd " << clientSocket << std::endl;
        close(clientSocket);
        return;
    }
    
    // Envoi d'un message d'accueil conforme au protocole IRC
    std::string serverName = "irc.42server.com";
    std::string welcomeMessage = ":" + serverName + " 001 * :Welcome to the Internet Relay Network\r\n";
    send(clientSocket, welcomeMessage.c_str(), welcomeMessage.length(), 0);
}


/**
 * @brief Supprime un client du serveur.
 */
void Server::removeClient(int clientSocket) {
    if (clients.find(clientSocket) == clients.end()) return;

    Client *client = clients[clientSocket];

    if (!client->getNickname().empty()) {
        std::string quitMsg = ":" + client->getNickname() + "!" + client->getUsername() +
                            "@localhost QUIT :Client disconnected\r\n";

        // Informe tous les canaux auxquels appartient ce client
        for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
            Channel* channel = it->second;
            if (channel->isClientInChannel(clientSocket)) {
                channel->broadcast(quitMsg, clientSocket);
                channel->removeClient(clientSocket);

                // Retirer l'utilisateur des opérateurs s'il en fait partie
                if (channel->isOperator(clientSocket)) {
                    channel->removeOperator(clientSocket);
                    if (!channel->isEmpty()) {
                        int newOp = *channel->getClients().begin();
                        channel->addOperator(newOp);
                    }
                }
            }
        }
    }

    close(clientSocket);
    delete clients[clientSocket];
    clients.erase(clientSocket);

    std::cout << "🚪 Client " << clientSocket << " supprimé du serveur.\n";
}

/* -------------------------------------------------------------------------- */
/*                                Gestion des Messages                        */
/* -------------------------------------------------------------------------- */


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

    // Si une erreur se produit, on déconnecte
    if (bytesRead < 0) {
        removeClient(clientSocket);
        return;
    }
    
    // Si recv retourne 0 (EOF)...
    if (bytesRead == 0) {
        Client* client = clients[clientSocket];
        // Vérifier si le buffer du client contient un fragment partiel
        if (!client->getBufferRef().empty()) {
            std::string leftover = client->getBufferRef();
            std::cout << "Partial command received (without CRLF): [" << leftover << "]\n";
            // Renvoyer ce fragment au client afin qu'il puisse continuer la saisie.
            // Ici, nous utilisons NOTICE pour signaler qu'il s'agit d'un fragment partiel.
            std::string echoMsg = ":irc.42server.com NOTICE * :" + leftover + "\r\n";
            send(clientSocket, echoMsg.c_str(), echoMsg.size(), 0);
            // Ne pas vider le buffer pour permettre au client de compléter la commande.
            return;
        }
        // Si le buffer est vide, alors le client a bien fermé la connexion.
        removeClient(clientSocket);
        return;
    }

    // Sinon, bytesRead > 0 : on ajoute les données reçues dans le buffer du client.
    buffer[bytesRead] = '\0';
    Client* client = clients[clientSocket];
    client->appendToBuffer(buffer, bytesRead);

    std::cout << "📩 Message reçu de " << clientSocket << " : " << buffer << std::endl;

    // Tenter d'extraire toutes les commandes complètes présentes dans le buffer
    std::string message;
    while ((message = client->extractNextMessage()) != "") {
        // Optionnel : si la commande commence par '/', on la retire.
        if (!message.empty() && message[0] == '/') {
            message.erase(0, 1);
        }
        std::cout << "🔍 Commande complète extraite : [" << message << "]\n";
        commandHandler.handleCommand(clientSocket, message);
    }
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
    if (message.empty()) {
        send(clientSocket, "ERROR :No text to send\r\n", 25, 0);
        return;
    }

    std::string cleanMessage = message;
    size_t firstCharPos = cleanMessage.find_first_not_of(" \t\r\n");
    if (firstCharPos != std::string::npos && cleanMessage[firstCharPos] == ':') {
        cleanMessage = cleanMessage.substr(firstCharPos + 1);
    }

    std::string fullMessage = ":" + clients[clientSocket]->getNickname() +
                              " PRIVMSG " + target + " :" + cleanMessage + "\r\n";

    // Si le message est destiné à un channel
    if (channels.find(target) != channels.end()) {
        Channel* channel = channels[target];

        // ✅ Vérifie que le client est bien dans le canal
        if (!channel->isClientInChannel(clientSocket)) {
            std::string errorMsg = ":irc.42server.com 442 " + clients[clientSocket]->getNickname() + 
                                   " " + target + " :You're not on that channel\r\n";
            send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
            return;
        }

        // 🔁 Diffuse le message aux autres membres du canal
        channel->broadcast(fullMessage, clientSocket);
        return;
    }

    // Si le message est destiné à un utilisateur
    bool userFound = false;
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->getNickname() == target) {
            send(it->second->getSocketFd(), fullMessage.c_str(), fullMessage.size(), 0);
            userFound = true;
            break;
        }
    }

    // Si aucun utilisateur ou channel trouvé
    if (!userFound) {
        std::string errorMsg = ":irc.42server.com 401 " + clients[clientSocket]->getNickname() + 
                               " " + target + " :No such nick/channel\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
    }
}


/* -------------------------------------------------------------------------- */
/*                                Gestion des Commandes IRC                   */
/* -------------------------------------------------------------------------- */

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
void Server::handlePass(int clientSocket, std::string& password) 
{
    if (!password.empty() && password[0] == ':') 
    {
        password.erase(0, 1);
    }
    if (clients.find(clientSocket) == clients.end()) {
        return;
    }

    if (clients[clientSocket]->isAuthenticated()) {
        send(clientSocket, ":irc.42server.com 462 * :You may not reregister\r\n", 51, 0);
        return;
    }

    if (password != this->password) {
        send(clientSocket, ":irc.42server.com 464 * :Password incorrect\r\n", 50, 0);
        removeClient(clientSocket);
        return;
    }

    clients[clientSocket]->authenticate();
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
        send(clientSocket, ":irc.42server.com 451 * :You must specify a password first\r\n", 58, 0);
        return;
    }

    // Vérifier si le pseudo est déjà pris
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->getNickname() == nickname) {
            std::string errorMsg = ":irc.42server.com 433 * " + nickname + " :Nickname is already in use\r\n";
            send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
            return;
        }
    }

    clients[clientSocket]->setNickname(nickname);

    std::string nickMsg = ":" + nickname + " NICK :" + nickname + "\r\n";
    send(clientSocket, nickMsg.c_str(), nickMsg.length(), 0);
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
        send(clientSocket, ":irc.42server.com 451 * :You must specify a password first\r\n", 58, 0);
        return;
    }

    if (clients[clientSocket]->getUsername() != "") {
        send(clientSocket, ":irc.42server.com 462 * :You may not reregister\r\n", 51, 0);
        return;
    }

    clients[clientSocket]->setUsername(username);
    clients[clientSocket]->setRealname(realname);

    // Vérifier si le client est maintenant complètement enregistré (NICK + USER)
    if (clients[clientSocket]->isFullyRegistered()) {
        std::string welcomeMsg = ":irc.42server.com 001 " + clients[clientSocket]->getNickname() +
                                 " :Welcome to the Internet Relay Network " +
                                 clients[clientSocket]->getNickname() + "!" +
                                 clients[clientSocket]->getUsername() + "@localhost\r\n";

        send(clientSocket, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
}

/**
 * @brief Gère la commande JOIN pour qu'un client rejoigne un canal.
 */
void Server::handleJoin(int clientSocket, const std::string& channelName, const std::string& password) {
    if (channelName.empty()) {
        send(clientSocket, ":irc.42server.com 461 JOIN :Not enough parameters\r\n", 50, 0);
        return;
    }

    // Création du channel s'il n'existe pas
    bool isNewChannel = (channels.find(channelName) == channels.end());
    if (isNewChannel) {
        channels[channelName] = new Channel(channelName);
    }
    Channel* channel = channels[channelName];

    // Vérification des permissions : invite only et mot de passe (si applicable)
    if (channel->getInviteOnly() && !channel->isInvited(clientSocket)) {
        std::string errorMsg = ":irc.42server.com 473 " + clients[clientSocket]->getNickname() + " " + channelName + " :Cannot join channel (+i) - Invite only\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }
    if (!channel->getPassword().empty() && channel->getPassword() != password) {
        std::string errorMsg = ":irc.42server.com 475 " + clients[clientSocket]->getNickname() + " " + channelName + " :Cannot join channel (+k) - Incorrect password\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }
    if (channel->isInvited(clientSocket)) {
        channel->removeInvitation(clientSocket);
    }

    // Vérification de la limitation : s'il y a une limite (non nulle) et le nombre d'utilisateurs est atteint
    if (channel->getUserLimit() != 0 && channel->getClients().size() >= static_cast<size_t>(channel->getUserLimit())) {
        std::string errorMsg = ":irc.42server.com 471 " + clients[clientSocket]->getNickname() + " " + channelName + " :Channel is full\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    // Ajout du client dans le channel
    channel->addClient(clientSocket);
    clients[clientSocket]->setCurrentChannel(channelName);

    // Si c'est un nouveau channel, le premier utilisateur devient opérateur
    if (isNewChannel) {
        channel->addOperator(clientSocket);
    }

    std::string nick = clients[clientSocket]->getNickname();
    std::string serverName = "irc.42server.com";

    // 1️⃣ Message JOIN : broadcast dans le channel
    std::string joinMsg = ":" + nick + " JOIN " + channelName + "\r\n";
    channel->broadcast(joinMsg, -1);

    // 2️⃣ Envoi du topic du channel (332 ou 331)
    std::string topicMsg;
    if (!channel->getTopic().empty()) {
        topicMsg = ":" + serverName + " 332 " + nick + " " + channelName + " :" + channel->getTopic() + "\r\n";
    } else {
        topicMsg = ":" + serverName + " 331 " + nick + " " + channelName + " :No topic is set\r\n";
    }
    send(clientSocket, topicMsg.c_str(), topicMsg.size(), 0);

    // 3️⃣ Envoi de la liste des utilisateurs du channel (353)
    std::string userList = ":" + serverName + " 353 " + nick + " = " + channelName + " :";
    const std::set<int>& channelClients = channel->getClients();
    for (std::set<int>::iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
        if (it != channelClients.begin()) {
            userList += " ";
        }
        std::string prefix = channel->isOperator(*it) ? "@" : "";
        userList += prefix + clients[*it]->getNickname();
    }
    userList += "\r\n";
    send(clientSocket, userList.c_str(), userList.size(), 0);

    // 4️⃣ Fin de la liste des utilisateurs (366)
    std::string endOfListMsg = ":" + serverName + " 366 " + nick + " " + channelName + " :End of NAMES list\r\n";
    send(clientSocket, endOfListMsg.c_str(), endOfListMsg.size(), 0);

    std::cout << "✅ [" << nick << "] a rejoint le canal " << channelName << std::endl;
}




/**
 * @brief Gère la commande PART pour qu'un client quitte un canal.
 */
void Server::handlePart(int clientSocket, const std::string& channelName) {
    if (channels.find(channelName) == channels.end()) {
        send(clientSocket, "ERROR :No such channel\r\n", 24, 0);
        return;
    }
    Channel* channel = channels[channelName];
    if (!channel->isClientInChannel(clientSocket)) {
        send(clientSocket, "ERROR :You're not in this channel\r\n", 34, 0);
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

    // Vérifier si l'utilisateur était un opérateur et réassigner
    if (channel->isOperator(clientSocket)) {
        channel->removeOperator(clientSocket);
        if (!channel->isEmpty()) {
            int newOp = *channel->getClients().begin();
            channel->addOperator(newOp);
        }
    }

    std::cout << "✅ Client " << clients[clientSocket]->getNickname() << " a quitté " << channelName << std::endl;

    if (channel->isEmpty()) {
        delete channel;
        channels.erase(channelName);
    }
}

/**
 * @brief Gère la commande QUIT lorsqu'un utilisateur quitte le serveur.
 */
void Server::handleQuit(int clientSocket, const std::string& quitMessage) {
    if (clients.find(clientSocket) == clients.end()) {
        return; // Le client n'existe pas (déjà supprimé)
    }

    Client* client = clients[clientSocket];
    std::string nick = client->getNickname();
    std::string fullQuitMessage = ":" + nick + " QUIT :" + (quitMessage.empty() ? "Client exited" : quitMessage) + "\r\n";

    // 🔹 Informer le canal où était le client (si applicable)
    std::string currentChannel = client->getCurrentChannel();
    if (!currentChannel.empty() && channels.find(currentChannel) != channels.end()) {
        Channel* channel = channels[currentChannel];
        channel->broadcast(fullQuitMessage, clientSocket);
        channel->removeClient(clientSocket);
        
        // Vérifier si un opérateur quitte et réassigner
        if (channel->isOperator(clientSocket)) {
            channel->removeOperator(clientSocket);
            if (!channel->isEmpty()) {
                int newOp = *channel->getClients().begin();
                channel->addOperator(newOp);
            }
        }
    }

    // 🔹 Fermer la connexion
    send(clientSocket, fullQuitMessage.c_str(), fullQuitMessage.size(), 0);
    close(clientSocket);

    // 🔹 Supprimer le client de la liste
    delete clients[clientSocket];
    clients.erase(clientSocket);

    std::cout << "🚪 [" << nick << "] s'est déconnecté proprement.\n";
}


void Server::handleList(int clientSocket) {
    std::string listMsg = "Active channels:\r\n";
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
        listMsg += "- " + it->first + "\r\n";
    }
    send(clientSocket, listMsg.c_str(), listMsg.length(), 0);
}

/**
 * @brief Gère la commande KICK pour éjecter un utilisateur d'un channel.
 */
void Server::handleKick(int clientSocket, const std::string& channelName, const std::string& targetNick) {
    if (channels.find(channelName) == channels.end()) {
        send(clientSocket, "ERROR :No such channel\r\n", 24, 0);
        return;
    }
    Channel* channel = channels[channelName];
    if (!channel->isOperator(clientSocket)) {
        send(clientSocket, "ERROR :You're not a channel operator\r\n", 38, 0);
        return;
    }
    int targetSocket = getClientSocketByNickname(targetNick);
    if (targetSocket == -1 || !channel->isClientInChannel(targetSocket)) {
        send(clientSocket, "ERROR :User not in channel\r\n", 28, 0);
        return;
    }
    
    // Créer le message de KICK selon le format IRC
    std::string kickerNick = clients[clientSocket]->getNickname();
    std::string kickMessage = ":" + kickerNick + "!" + clients[clientSocket]->getUsername() + "@localhost KICK " + channelName + " " + targetNick + " :Kicked by " + kickerNick + "\r\n";
    
    // Envoyer le message au canal
    channel->broadcast(kickMessage, targetSocket);
    
    // Envoyer le message directement à l'utilisateur kické
    send(targetSocket, kickMessage.c_str(), kickMessage.length(), 0);
    
    // Retirer le client du canal
    channel->removeClient(targetSocket);
}

/**
 * @brief Gère la commande INVITE pour inviter un utilisateur dans un channel.
 */
void Server::handleInvite(int clientSocket, const std::string& targetNick, const std::string& channelName) {
    if (channels.find(channelName) == channels.end()) {
        std::string errorMsg = ":irc.42server.com 403 " + clients[clientSocket]->getNickname() + " " + channelName + " :No such channel\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    Channel* channel = channels[channelName];

    // Vérifier si l'utilisateur est dans le canal
    if (!channel->isClientInChannel(clientSocket)) {
        std::string errorMsg = ":irc.42server.com 442 " + clients[clientSocket]->getNickname() + " " + channelName + " :You're not on that channel\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    // Vérifier si l'utilisateur est opérateur du canal
    if (!channel->isOperator(clientSocket)) {
        std::string errorMsg = ":irc.42server.com 482 " + clients[clientSocket]->getNickname() + " " + channelName + " :You're not a channel operator\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    // Vérifier si la cible existe
    int targetSocket = getClientSocketByNickname(targetNick);
    if (targetSocket == -1) {
        std::string errorMsg = ":irc.42server.com 401 " + clients[clientSocket]->getNickname() + " " + targetNick + " :No such nick\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    // Ajouter l'utilisateur à la liste des invités du canal
    channel->inviteClient(targetSocket);

    // Envoyer la confirmation au client émetteur
    std::string inviteMsg = ":irc.42server.com 341 " + clients[clientSocket]->getNickname() + " " + targetNick + " " + channelName + "\r\n";
    send(clientSocket, inviteMsg.c_str(), inviteMsg.length(), 0);

    // Informer la cible qu'il a été invité
    std::string noticeMsg = ":" + clients[clientSocket]->getNickname() + " INVITE " + targetNick + " " + channelName + "\r\n";
    send(targetSocket, noticeMsg.c_str(), noticeMsg.length(), 0);
}


/**
 * @brief Gère la commande TOPIC pour modifier ou afficher le sujet d'un channel.
 */
void Server::handleTopic(int clientSocket, const std::string& channelName, const std::string& topic) {
    if (channels.find(channelName) == channels.end()) {
        std::string errorMsg = ":irc.42server.com 403 " + clients[clientSocket]->getNickname() + " " + channelName + " :No such channel\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    Channel* channel = channels[channelName];

    // 🔹 Si aucun topic n'est donné, retourner le topic actuel
    if (topic.empty()) {
        std::string currentTopic = channel->getTopic();
        std::string response;
        
        if (currentTopic.empty()) {
            // Pas de topic défini
            response = ":irc.42server.com 331 " + clients[clientSocket]->getNickname() + 
                       " " + channelName + " :No topic is set\r\n";
        } else {
            // Retourner le topic actuel
            response = ":irc.42server.com 332 " + clients[clientSocket]->getNickname() + 
                       " " + channelName + " :" + currentTopic + "\r\n";
        }
    
        std::cout << "📩 Envoi du topic à " << clients[clientSocket]->getNickname() << " : " << response;
        send(clientSocket, response.c_str(), response.size(), 0);
        return;
    }
    
    std::cout << "📌 Demande du topic pour " << channelName << std::endl;


    // 🔹 Vérification : Seuls les opérateurs peuvent changer le topic si le mode +t est activé
    if (channel->getTopicRestricted() && !channel->isOperator(clientSocket)) {
        std::string errorMsg = ":irc.42server.com 482 " + clients[clientSocket]->getNickname() + " " + channelName + " :You're not a channel operator\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    // 🔹 Appliquer la modification du topic
    // Nettoyer le topic si un ":" est présent au début
    std::string cleanTopic = topic;
    if (!cleanTopic.empty() && cleanTopic[0] == ':') {
        cleanTopic.erase(0, 1);
    }

    std::string topicMessage = ":" + clients[clientSocket]->getNickname() + "!" + clients[clientSocket]->getUsername() + "@localhost TOPIC " + channelName + " :" + cleanTopic + "\r\n";
    channel->broadcast(topicMessage, -1);

}


/**
 * @brief Gère la commande MODE pour changer les paramètres d'un channel.
 */
void Server::handleMode(int clientSocket, const std::string& channelName, const std::string& mode, const std::string& param) {
    if (channels.find(channelName) == channels.end()) {
        std::string errorMsg = ":irc.42server.com 403 " + clients[clientSocket]->getNickname() + " " + channelName + " :No such channel\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }
    Channel* channel = channels[channelName];

    // Vérifier que l'utilisateur est opérateur
    if (!channel->isOperator(clientSocket)) {
        std::string errorMsg = ":irc.42server.com 482 " + clients[clientSocket]->getNickname() + " " + channelName + " :You're not a channel operator\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    std::string response = ":" + clients[clientSocket]->getNickname() + " MODE " + channelName + " " + mode;

    // Gestion des modes
    if (mode == "+i") {
        channel->setInviteOnly(true);
        response += "\r\n";
    } else if (mode == "-i") {
        channel->setInviteOnly(false);
        response += "\r\n";
    } else if (mode == "+t") {
        channel->setTopicRestricted(true);
        response += "\r\n";
    } else if (mode == "-t") {
        channel->setTopicRestricted(false);
        response += "\r\n";
    } else if (mode == "+k") {
        if (param.empty()) {
            std::string errorMsg = ":irc.42server.com 461 " + clients[clientSocket]->getNickname() + " MODE :Not enough parameters\r\n";
            send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
            return;
        }
        channel->setPassword(param);
        response += " " + param + "\r\n";
    } else if (mode == "-k") {
        channel->setPassword("");
        response += "\r\n";
    } else if (mode == "+o") {
        int targetSocket = getClientSocketByNickname(param);
        if (targetSocket == -1 || !channel->isClientInChannel(targetSocket)) {
            std::string errorMsg = ":irc.42server.com 441 " + clients[clientSocket]->getNickname() + " " + param + " " + channelName + " :They aren't on that channel\r\n";
            send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
            return;
        }
        channel->addOperator(targetSocket);
        response += " " + param + "\r\n";
    } else if (mode == "-o") {
        int targetSocket = getClientSocketByNickname(param);
        if (targetSocket == -1 || !channel->isClientInChannel(targetSocket)) {
            std::string errorMsg = ":irc.42server.com 441 " + clients[clientSocket]->getNickname() + " " + param + " " + channelName + " :They aren't on that channel\r\n";
            send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
            return;
        }
        channel->removeOperator(targetSocket);
        response += " " + param + "\r\n";
    } else if (mode == "+l") {
        if (param.empty() || atoi(param.c_str()) <= 0) {
            std::string errorMsg = ":irc.42server.com 461 " + clients[clientSocket]->getNickname() + " MODE :Invalid user limit\r\n";
            send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
            return;
        }
        channel->setUserLimit(atoi(param.c_str()));
        response += " " + param + "\r\n";
    } else if (mode == "-l") {
        channel->setUserLimit(0);
        response += "\r\n";
    } else {
        std::string errorMsg = ":irc.42server.com 472 " + clients[clientSocket]->getNickname() + " " + mode + " :is unknown mode char to me\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    // Diffuser le changement de mode à tous les clients du canal
    channel->broadcast(response, -1);
    std::cout << "🔹 Mode appliqué : " << mode << " avec paramètre : " << param << " sur " << channelName << std::endl;
}

/**
 * @brief Gère la commande PING pour maintenir la connexion avec le client.
 *
 * - Reçoit un token du client avec la commande PING
 * - Retourne une réponse PONG avec le même token
 * - Confirme que la connexion est active
 *
 * @param clientSocket Le descripteur de fichier du client envoyant le PING.
 * @param token Le token envoyé par le client, qui doit être retourné dans la réponse PONG.
 */
void Server::handlePing(int clientSocket, const std::string& token) {
    std::string pongResponse = ":irc.42server.com PONG irc.42server.com :" + token + "\r\n";
    send(clientSocket, pongResponse.c_str(), pongResponse.length(), 0);
    std::cout << "✅ PING-PONG with token: " << token << std::endl;
}


/* -------------------------------------------------------------------------- */
/*                                Utilitaires                                 */
/* -------------------------------------------------------------------------- */

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

std::map<int, Client*>& Server::getClients() {
    return clients;
}


