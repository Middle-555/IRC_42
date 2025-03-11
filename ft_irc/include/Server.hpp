/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:07:11 by kpourcel          #+#    #+#             */
/*   Updated: 2025/03/12 00:09:04 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sstream>
#include <csignal>

#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"
#include "Utils.hpp"

#define MAX_CLIENTS 100  // Limite de clients simultanés

class Client;
class CommandHandler;

class Server {
private:
    int                             serverSocket;       // Socket du serveur
    int                             port;               // Port d'écoute
    std::string                     password;           // Mot de passe du serveur
    std::vector<struct pollfd>      pollFds;            // Poll pour gérer les connexions
    std::map<int, Client*>          clients;            // Liste des clients connectés (clé = socket)
    std::map<std::string, Channel*> channels;           // Map des channels (clé = nom du channel)
    CommandHandler                  commandHandler;     // Gestionnaire de commandes

    
    bool                            running;


    void    handleNewConnection();
    void    handleClientMessage(int clientSocket);
    void    removeClient(int clientSocket);

public:
    Server(int port, std::string password);
    ~Server();

    std::string     serverName;


    void    run();
    
    // Gestion de l'authentification
    void    handlePass(int clientSocket, const std::string& password);
    void    handleNick(int clientSocket, const std::string& nickname);
    void    handleUser(int clientSocket, const std::string& username, const std::string& realname);

    // Gestion des channels
    void    handleJoin(int clientSocket, const std::string& channelName);
    void    handlePart(int clientSocket, const std::string& channelName);
    void    handleList(int clientSocket);

    // Messagerie
    void    handlePrivMsg(int clientSocket, const std::string& target, const std::string& message);

    // Utilitaire
    int                     getClientSocketByNickname(const std::string& nickname) const;
    std::map<int, Client*>& getClients();
    void handleQuit(int clientSocket, const std::string& quitMessage);
    void shutdownServer();
    void stopServer();
};


#endif // SERVER_HPP
