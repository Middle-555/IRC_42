/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kpourcel <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 15:41:17 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/17 15:57:34 by kpourcel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>

class Server;

class CommandHandler {
private:
    Server& server;
    void handlePassCmd(int clientSocket, std::istringstream &iss);
    void handleNickCmd(int clientSocket, std::istringstream &iss);
    void handleUserCmd(int clientSocket, std::istringstream &iss);
    void handleJoinCmd(int clientSocket, std::istringstream &iss);
    void handleQuitCmd(int clientSocket, std::istringstream &iss);
    void handlePartCmd(int clientSocket, std::istringstream &iss);
    void handleListCmd(int clientSocket);
    void handlePrivMsgCmd(int clientSocket, std::istringstream &iss);
    void handleKickCmd(int clientSocket, std::istringstream &iss);
    void handleInviteCmd(int clientSocket, std::istringstream &iss);
    void handleTopicCmd(int clientSocket, std::istringstream &iss);
    void handleModeCmd(int clientSocket, std::istringstream &iss);
    void handlePingCmd(int clientSocket, std::istringstream &iss);
public:
    CommandHandler(Server& srv);
    void handleCommand(int clientSocket, const std::string& command);
};

#endif
