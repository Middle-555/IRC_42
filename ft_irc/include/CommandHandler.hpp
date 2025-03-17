/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 15:41:17 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/17 11:25:15 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>

class Server;

class CommandHandler {
private:
    Server& server;

public:
    CommandHandler(Server& srv);
    void handleCommand(int clientSocket, const std::string& command);
    
    // Ajout des nouvelles commandes
    void handleKick(int clientSocket, const std::string& channelName, const std::string& targetNick);
    void handleInvite(int clientSocket, const std::string& channelName, const std::string& targetNick);
    void handleTopic(int clientSocket, const std::string& channelName, const std::string& topic);
    void handleMode(int clientSocket, const std::string& channelName, const std::string& mode, const std::string& param);
};

#endif
