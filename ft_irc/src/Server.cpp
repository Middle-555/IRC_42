/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:24:18 by kpourcel          #+#    #+#             */
/*   Updated: 2025/03/12 00:04:07 by acabarba         ###   ########.fr       */
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
    if (clients.find(clientSocket) == clients.end()) return;

    Client *client = clients[clientSocket];

    if (!client->getNickname().empty()) {
        std::string quitMsg = ":" + client->getNickname() + "!" + client->getUsername()
                            + "@localhost QUIT :Client disconnected\r\n";

        // Informe tous les canaux auxquels appartient ce client
        for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
            if (it->second->isClientInChannel(clientSocket)) {
                it->second->broadcast(quitMsg, clientSocket);
                it->second->removeClient(clientSocket);
            }
        }
    }

    close(clientSocket);
    delete client;  
    clients.erase(clientSocket);

    for (size_t i = 0; i < pollFds.size(); ++i) {
        if (pollFds[i].fd == clientSocket) {
            pollFds.erase(pollFds.begin() + i);
            break;
        }
    }

    std::cout << "🔴 Client déconnecté proprement." << std::endl;
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
        send(clientSocket, "ERROR :Already registered\r\n", 26, 0);
        return;
    }

    // Vérifier si le mot de passe est correct
    if (password != this->password) {
        send(clientSocket, "ERROR :Incorrect password\r\n", 28, 0);
        removeClient(clientSocket);
        return;
    }

    clients[clientSocket]->authenticate();
    send(clientSocket, "    OK :Password accepted\r\n", 27, 0);
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
    std::string response = "    OK :Nickname set to " + nickname + "\r\n";
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

    // Vérification stricte des paramètres USER
    if (username.empty() 
        || username == "0" 
        || username == "*" 
        || username.find_first_not_of(" \t") == std::string::npos
        || realname.empty()
        || realname.find_first_not_of(" \t") == std::string::npos) {

        const char* errorMsg = "ERROR :Invalid USER parameters\r\n";
        send(clientSocket, errorMsg, strlen(errorMsg), 0);
        return;
    }

    clients[clientSocket]->setUsername(username);
    clients[clientSocket]->setRealname(realname);

    std::string response = "     OK :User " + clients[clientSocket]->getNickname()
                         + " set user name to : " + username
                         + " and real name to : " + realname + "\r\n";

    send(clientSocket, response.c_str(), response.size(), 0);

    // 🔹 Vérifier si le client est maintenant complètement enregistré (NICK + USER)
    if (clients[clientSocket]->isFullyRegistered()) {
        std::string welcomeMsg = ":irc.42server.com 001 " + clients[clientSocket]->getNickname() +
                                 " :Welcome to the Internet Relay Network " +
                                 clients[clientSocket]->getNickname() + "!" +
                                 clients[clientSocket]->getUsername() + "@localhost\r\n";

        send(clientSocket, welcomeMsg.c_str(), welcomeMsg.size(), 0);
    }
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

    std::string nick = clients[clientSocket]->getNickname();
    std::string serverName = "irc.42server.com"; // Définition du serveur

    // 1️⃣ Message JOIN
    std::string joinMsg = ":" + nick + " JOIN :" + channelName + "\r\n";
    channel->broadcast(joinMsg, -1);

    // 2️⃣ Réponse 332 (Topic du channel)
    std::string topicMsg = ":" + serverName + " 332 " + nick + " " + channelName + " :Welcome to " + channelName + "\r\n";
    send(clientSocket, topicMsg.c_str(), topicMsg.size(), 0);

    // 3️⃣ Réponse 353 (Liste des utilisateurs du channel)
    std::string userList = ":" + serverName + " 353 " + nick + " = " + channelName + " :";
    const std::set<int>& channelClients = channel->getClients();
    for (std::set<int>::iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
        if (it != channelClients.begin()) {
            userList += " ";  // Ajoute un espace uniquement entre les pseudos
        }
        userList += clients[*it]->getNickname();
    }    
    userList += "\r\n";
    send(clientSocket, userList.c_str(), userList.size(), 0);

    // 4️⃣ Réponse 366 (Fin de la liste des utilisateurs)
    std::string endOfListMsg = ":" + serverName + " 366 " + nick + " " + channelName + " :End of NAMES list\r\n";
    send(clientSocket, endOfListMsg.c_str(), endOfListMsg.size(), 0);

    // Log
    std::cout << "✅ [" << nick << "] a rejoint le canal " << channelName << std::endl;
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
    if (message.empty()) {
        send(clientSocket, "ERROR :No text to send\r\n", 25, 0);
        return;
    }

    // 📌 Suppression correcte du ":" en début de message uniquement si présent
    std::string cleanMessage = message;
    if (!cleanMessage.empty() && cleanMessage[0] == ':') {
        cleanMessage = cleanMessage.substr(1);
    }

    std::string fullMessage = ":" + clients[clientSocket]->getNickname() +
                              " PRIVMSG " + target + " :" + cleanMessage + "\r\n";

    // 📌 Vérifier si c'est un message envoyé à un canal
    if (channels.find(target) != channels.end()) {
        Channel* channel = channels[target];
        channel->broadcast(fullMessage, clientSocket);
        return;
    }

    // 📌 Vérifier si c'est un message envoyé à un utilisateur
    bool userFound = false;
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->getNickname() == target) {
            send(it->second->getSocketFd(), fullMessage.c_str(), fullMessage.size(), 0);
            userFound = true;
            break;
        }
    }

    // 📌 Si aucun utilisateur trouvé, renvoyer une erreur
    if (!userFound) {
        std::string errorMsg = "ERROR :No such nick/channel\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);
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
        channels[currentChannel]->broadcast(fullQuitMessage, clientSocket);
        channels[currentChannel]->removeClient(clientSocket);
    }

    // 🔹 Fermer la connexion
    send(clientSocket, fullQuitMessage.c_str(), fullQuitMessage.size(), 0);
    close(clientSocket);

    // 🔹 Supprimer le client de la liste
    delete clients[clientSocket];
    clients.erase(clientSocket);

    std::cout << "🚪 [" << nick << "] s'est déconnecté proprement.\n";
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
