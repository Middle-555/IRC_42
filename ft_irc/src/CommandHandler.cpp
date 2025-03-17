/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kpourcel <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 23:11:20 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/17 16:01:13 by kpourcel         ###   ########.fr       */
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
    iss >> cmd;

    std::cout << "📌 CommandHandler : [" << cmd << "] reçue du client " << clientSocket << std::endl;
    std::cout << std::endl;

    if (server.getClients().find(clientSocket) == server.getClients().end()) {
        send(clientSocket, "ERROR :Client not found\r\n", 26, 0);
        return;
    }

    Client* client = server.getClients()[clientSocket];

    if (!client->isFullyRegistered() && cmd != "NICK" && cmd != "USER" && cmd != "PASS") {
        send(clientSocket, "ERROR :You must register with NICK and USER first\r\n", 50, 0);
        return;
    }
    if (cmd == "PASS")
        handlePassCmd(clientSocket, iss);
    else if (cmd == "NICK") 
        handleNickCmd(clientSocket, iss);
    else if (cmd == "USER")
        handleUserCmd(clientSocket, iss);
    else if (cmd == "JOIN")
        handleJoinCmd(clientSocket, iss);
    else if (cmd == "QUIT")
        handleQuitCmd(clientSocket, iss);
    else if (cmd == "PART")
        handlePartCmd(clientSocket, iss);
    else if (cmd == "LIST")
        handleListCmd(clientSocket);
    else if (cmd == "PRIVMSG")
        handlePrivMsgCmd(clientSocket, iss);
    else if (cmd == "KICK")
        handleKickCmd(clientSocket, iss);
    else if (cmd == "INVITE")
        handleInviteCmd(clientSocket, iss);
    else if (cmd == "TOPIC")
        handleTopicCmd(clientSocket, iss);
    else if (cmd == "MODE")
        handleModeCmd(clientSocket, iss);
    else {
        std::cout << "❌ Commande inconnue : [" << cmd << "]\n";
        std::string errorMsg = "ERROR :Unknown command\r\n";
        send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
    }
}


void CommandHandler::handlePassCmd(int clientSocket, std::istringstream &iss) {
    std::string password;
    iss >> password;
    server.handlePass(clientSocket, password);
}

void CommandHandler::handleNickCmd(int clientSocket, std::istringstream &iss) {
    std::string nickname;
    iss >> nickname;
    server.handleNick(clientSocket, nickname);
}

void CommandHandler::handleUserCmd(int clientSocket, std::istringstream &iss) {
    std::string username, mode, unused, realname;
    iss >> username >> mode >> unused;
    std::getline(iss, realname);

    // Nettoyage
    realname.erase(0, realname.find_first_not_of(" \t"));
    if (!realname.empty() && realname[0] == ':') {
        realname.erase(0, 1);
    }
    realname.erase(0, realname.find_first_not_of(" \t"));

    server.handleUser(clientSocket, username, realname);
}

void CommandHandler::handleJoinCmd(int clientSocket, std::istringstream &iss) {
    std::string channel, password;
    iss >> channel;
    if (iss >> password) {
        server.handleJoin(clientSocket, channel, password);
    } else {
        server.handleJoin(clientSocket, channel, "");
    }
}

void CommandHandler::handleQuitCmd(int clientSocket, std::istringstream &iss) {
    std::string quitMessage;
    std::getline(iss, quitMessage);
    if (!quitMessage.empty() && quitMessage[0] == ':') {
        quitMessage.erase(0, 1);
    }
    server.handleQuit(clientSocket, quitMessage);
}

void CommandHandler::handlePartCmd(int clientSocket, std::istringstream &iss) {
    std::string channel;
    iss >> channel;
    server.handlePart(clientSocket, channel);
}

void CommandHandler::handleListCmd(int clientSocket) {
    server.handleList(clientSocket);
}

void CommandHandler::handlePrivMsgCmd(int clientSocket, std::istringstream &iss) {
    std::string target, message;
    iss >> target;
    std::getline(iss, message);
    if (!message.empty() && message[0] == ':') message.erase(0, 1);

    server.handlePrivMsg(clientSocket, target, message);
}

void CommandHandler::handleKickCmd(int clientSocket, std::istringstream &iss) {
    std::string channel, targetNick;
    iss >> channel >> targetNick;
    server.handleKick(clientSocket, channel, targetNick);
}

void CommandHandler::handleInviteCmd(int clientSocket, std::istringstream &iss) {
    std::string channel, targetNick;
    iss >> channel >> targetNick;
    server.handleInvite(clientSocket, channel, targetNick);
}

void CommandHandler::handleTopicCmd(int clientSocket, std::istringstream &iss) {
    std::string channel, topic;
    iss >> channel;
    std::getline(iss, topic);
    if (!topic.empty() && topic[0] == ':') topic.erase(0, 1);

    server.handleTopic(clientSocket, channel, topic);
}

void CommandHandler::handleModeCmd(int clientSocket, std::istringstream &iss) {
    std::string channel, mode, param;
    iss >> channel >> mode;
    if (iss >> param) {
        server.handleMode(clientSocket, channel, mode, param);
    } else {
        server.handleMode(clientSocket, channel, mode, "");
    }
}
