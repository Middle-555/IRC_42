/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:07:11 by kpourcel          #+#    #+#             */
/*   Updated: 2025/03/06 23:41:43 by acabarba         ###   ########.fr       */
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
#include "Client.hpp"

#define MAX_CLIENTS 100  // Limite de clients simultanés

class Client;

class Server {
private:
    int                 serverSocket;       // Socket du serveur
    int                 port;               // Port d'écoute
    std::string         password;           // Mot de passe du serveur
    std::vector<struct pollfd> pollFds;     // Poll pour gérer les connexions
    std::map<int, Client*> clients;         // Liste des clients connectés

    void    handleNewConnection();
    void    handleClientMessage(int clientSocket);
    void    removeClient(int clientSocket);

public:
    Server(int port, std::string password);
    ~Server();

    void    run();
    void    handlePass(int clientSocket, const std::string& password);
    void    handleNick(int clientSocket, const std::string& nickname);
    void    handleUser(int clientSocket, const std::string& username, const std::string& realname);

};

#endif
