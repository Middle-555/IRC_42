/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:24:18 by kpourcel          #+#    #+#             */
/*   Updated: 2025/03/07 01:20:18 by acabarba         ###   ########.fr       */
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
Server::Server(int port, std::string password) : port(port), password(password) {
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
        // Attente d'événements sur les sockets
        int ret = poll(pollFds.data(), pollFds.size(), -1);
        if (ret < 0) {
            perror("Erreur poll()");
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
        perror("Erreur accept()");
        return;
    }

    std::cout << "🟢 Nouveau client connecté : " << inet_ntoa(clientAddr.sin_addr) << std::endl;

    struct pollfd clientPollFd;
    clientPollFd.fd = clientSocket;
    clientPollFd.events = POLLIN;
    pollFds.push_back(clientPollFd);

    // Création et stockage du client
    clients[clientSocket] = new Client(clientSocket);

    // Envoi d'un message d'accueil au client
    std::string welcomeMessage = "Bienvenue sur le serveur IRC !\n";
    welcomeMessage += "Veuillez entrer les informations demandées :\n";
    welcomeMessage += "Mot de passe du serveur : ";
    send(clientSocket, welcomeMessage.c_str(), welcomeMessage.length(), 0);
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
    std::cout << "📩 Message reçu : " << message << std::endl;

    // Nettoyer le message des éventuels caractères parasites (\r\n)
    message.erase(message.find_last_not_of("\r\n") + 1);

    // Vérifie si le client existe encore avant d'agir
    if (clients.find(clientSocket) == clients.end()) {
        return;
    }

    // Étape 1 : Vérification du mot de passe
    if (!clients[clientSocket]->isAuthenticated()) {
        if (message.substr(0, 5) != "PASS ") {
            message = "PASS " + message;
        }
        handlePass(clientSocket, message);

        // Vérifie que le client n'a pas été supprimé avant d'afficher la suite
        if (clients.find(clientSocket) == clients.end()) {
            return;
        }
    }

    // Étape 2 : Vérification du pseudo (NICK)
    if (clients[clientSocket]->getNickname().empty()) {
        if (message.substr(0, 5) != "NICK ") {
            message = "NICK " + message;
        }
        handleNick(clientSocket, message.substr(5));

        send(clientSocket, "✅ Pseudo enregistré\r\nVotre nom : \r\n", 38, 0);
        return;
    }

    // Étape 3 : Vérification du nom d'utilisateur (USER)
    if (clients[clientSocket]->getUsername().empty()) {
        if (message.substr(0, 5) != "USER ") {
            message = "USER " + message + " 0 * :";
        }
        handleUser(clientSocket, message.substr(5), message.substr(5));

        send(clientSocket, "✅ Inscription terminée ! 🎉\r\n", 34, 0);
        return;
    }
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
void Server::handlePass(int clientSocket, const std::string& message) {
    // Vérifie que le client existe avant d'accéder à lui
    if (clients.find(clientSocket) == clients.end()) {
        return;
    }

    if (clients[clientSocket]->isAuthenticated()) {
        send(clientSocket, "ERROR :Already registered\r\n", 27, 0);
        return;
    }

    // Vérifier si le message commence par "PASS "
    if (message.substr(0, 5) != "PASS ") {
        send(clientSocket, "ERROR :Invalid PASS command\r\n", 29, 0);
        return;
    }

    // Extraire le mot de passe après "PASS "
    std::string receivedPassword = message.substr(5);
    receivedPassword.erase(receivedPassword.find_last_not_of("\r\n") + 1);

    if (receivedPassword != this->password) {
        send(clientSocket, "❌ ERROR :Incorrect password\r\n", 30, 0);
        
        // Vérifie avant suppression
        if (clients.find(clientSocket) != clients.end()) {
            removeClient(clientSocket);
        }

        return;
    }

    clients[clientSocket]->authenticate();
    send(clientSocket, "✅ OK :Password accepted\r\nVotre pseudo : \r\n", 42, 0);
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

