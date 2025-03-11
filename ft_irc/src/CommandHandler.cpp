/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 23:11:20 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/11 23:22:10 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/CommandHandler.hpp"
#include "../include/Server.hpp"

/**
 * @brief Constructeur du gestionnaire de commandes.
 *
 * @param srv Référence vers le serveur IRC.
 */
CommandHandler::CommandHandler(Server &srv) : server(srv) {}

/**
 * @brief Gère une commande envoyée par un client.
 *
 * @param clientSocket Descripteur de fichier du client.
 * @param command La commande complète envoyée par le client.
 */
void CommandHandler::handleCommand(int clientSocket, const std::string &command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd; // Extrait la commande

    std::cout << "📌 CommandHandler : [" << cmd << "] reçue du client " << clientSocket << std::endl;
	std::cout << std::endl;

    // Vérifier si le client existe
    if (server.getClients().find(clientSocket) == server.getClients().end()) {
        send(clientSocket, "ERROR :Client not found\r\n", 26, 0);
        return;
    }

    Client* client = server.getClients()[clientSocket];

    // 🔒 **Obliger NICK et USER avant toute autre commande**
    if (!client->isFullyRegistered() && cmd != "NICK" && cmd != "USER" && cmd != "PASS") {
        send(clientSocket, "ERROR :You must register with NICK and USER first\r\n", 50, 0);
        return;
    }

    // Traitement des commandes
    if (cmd == "PASS") {
        std::string password;
        iss >> password;
        server.handlePass(clientSocket, password);
    } else if (cmd == "NICK") {
        std::string nickname;
        iss >> nickname;
        server.handleNick(clientSocket, nickname);
    } else if (cmd == "USER") {
        std::string username, mode, unused, realname;
        iss >> username >> mode >> unused;
        std::getline(iss, realname);
        realname.erase(0, realname.find_first_not_of(" \t"));
        if (!realname.empty() && realname[0] == ':') {
            realname.erase(0, 1);
        }
        realname.erase(0, realname.find_first_not_of(" \t"));
        server.handleUser(clientSocket, username, realname);
    } else if (cmd == "JOIN") {
        std::string channel;
        iss >> channel;
        server.handleJoin(clientSocket, channel);
    } else if (cmd == "QUIT") {
        std::string quitMessage;
        std::getline(iss, quitMessage);
        if (!quitMessage.empty() && quitMessage[0] == ':') {
            quitMessage.erase(0, 1);
        }
        server.handleQuit(clientSocket, quitMessage);
    } else if (cmd == "PART") {
        std::string channel;
        iss >> channel;
        server.handlePart(clientSocket, channel);
    } else if (cmd == "LIST") {
        server.handleList(clientSocket);
    } else if (cmd == "PRIVMSG") {
        std::string target, message;
        iss >> target;
        std::getline(iss, message);
        if (!message.empty() && message[0] == ':') message.erase(0, 1);
        server.handlePrivMsg(clientSocket, target, message);
    } else {
        std::cout << "❌ Commande inconnue : [" << cmd << "]\n";
        std::string errorMsg = "ERROR :Unknown command\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
    }
}

