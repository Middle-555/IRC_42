/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 15:41:17 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/10 16:21:20 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

class Server;

class CommandHandler {
private:
    Server &server;

public:
    CommandHandler(Server &srv);
    void handleCommand(int clientSocket, const std::string &command);
};

#endif