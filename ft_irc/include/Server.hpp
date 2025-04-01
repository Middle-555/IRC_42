/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kpourcel <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:07:11 by kpourcel          #+#    #+#             */
/*   Updated: 2025/04/01 15:34:47 by kpourcel         ###   ########.fr       */
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
    
        /*                                Gestion des Connexions                      */
        void    handleNewConnection();
        void    removeClient(int clientSocket);
        
        /*                                Gestion des Messages                        */
        void    handleClientMessage(int clientSocket);
    
    public:
        std::string     serverName;
    
        /*                                Constructeur / Destructeur                  */
        Server();
        Server(int port, std::string password);
        ~Server();
        
        /*                                Gestion du Serveur                          */
        void    run();
        void    shutdownServer();
    
        /*                                Gestion des Messages                        */
        void    handlePrivMsg(int clientSocket, const std::string& target, const std::string& message);
    
        /*                                Gestion des Commandes IRC                   */
        void    handlePass(int clientSocket, std::string& password);
        void    handleNick(int clientSocket, const std::string& nickname);
        void    handleUser(int clientSocket, const std::string& username, const std::string& realname);
        void    handleJoin(int clientSocket, const std::string& channelName, const std::string& password);
        void    handlePart(int clientSocket, const std::string& channelName);
        void    handleList(int clientSocket);
        void    handleQuit(int clientSocket, const std::string& quitMessage);
        void    handlePing(int clientSocket, const std::string& token);
        
        /*                                Gestion des Commandes Opérateurs            */
        void    handleKick(int clientSocket, const std::string& channelName, const std::string& targetNick);
        void    handleInvite(int clientSocket, const std::string& channelName, const std::string& targetNick);
        void    handleTopic(int clientSocket, const std::string& channelName, const std::string& topic);
        void    handleMode(int clientSocket, const std::string& channelName, const std::string& mode, const std::string& param);
        
        /*                                Utilitaires                                 */
        int     getClientSocketByNickname(const std::string& nickname) const;
        std::map<int, Client*>& getClients();
    };


#endif // SERVER_HPP
