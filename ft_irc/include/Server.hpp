/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:07:11 by kpourcel          #+#    #+#             */
/*   Updated: 2025/04/01 17:31:33 by acabarba         ###   ########.fr       */
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

#define MAX_CLIENTS 100

class Client;
class CommandHandler;

class Server {
    private:
        int                             serverSocket;
        int                             port;
        std::string                     password;
        std::vector<struct pollfd>      pollFds;
        std::map<int, Client*>          clients;
        std::map<std::string, Channel*> channels;
        CommandHandler                  commandHandler;

        /**
         * Gestion des Connexions
         */
        void    handleNewConnection();
        void    removeClient(int clientSocket);
        
        /**
         * Gestion des Messages
         */
        void    handleClientMessage(int clientSocket);
    
    public:
        std::string     serverName;
    
        /**
         * Constructeur / Destructeur
         */
        Server();
        Server(int port, std::string password);
        ~Server();
    
        /**
         * Gestion du Serveur
         */
        void    run();
        void    shutdownServer();

        /**
         * Gestion des Messages
         */
        void    handlePrivMsg(int clientSocket, const std::string& target, const std::string& message);

        /**
         * Gestion des Commandes IRC 
         */
        void    handlePass(int clientSocket, std::string& password);
        void    handleNick(int clientSocket, const std::string& nickname);
        void    handleUser(int clientSocket, const std::string& username, const std::string& realname);
        void    handleJoin(int clientSocket, const std::string& channelName, const std::string& password);
        void    handlePart(int clientSocket, const std::string& channelName);
        void    handleList(int clientSocket);
        void    handleQuit(int clientSocket, const std::string& quitMessage);
        void    handlePing(int clientSocket, const std::string& token);

        /**
         * Gestion des Commandes Opérateurs
         */
        void    handleKick(int clientSocket, const std::string& channelName, const std::string& targetNick);
        void    handleInvite(int clientSocket, const std::string& channelName, const std::string& targetNick);
        void    handleTopic(int clientSocket, const std::string& channelName, const std::string& topic);
        void    handleMode(int clientSocket, const std::string& channelName, const std::string& mode, const std::string& param);

        /**
         * Utilitaires
         */
        int     getClientSocketByNickname(const std::string& nickname) const;
        std::map<int, Client*>& getClients();
    };


#endif
