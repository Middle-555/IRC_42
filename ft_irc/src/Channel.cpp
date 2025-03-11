/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 15:49:43 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/11 23:24:22 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"
#include <sys/socket.h>
#include <iostream>

Channel::Channel(const std::string& channelName) : name(channelName) {}

void Channel::addClient(int clientSocket) {
    clients.insert(clientSocket);
}

void Channel::removeClient(int clientSocket) {
    clients.erase(clientSocket);
}

bool Channel::isClientInChannel(int clientSocket) const {
    return clients.find(clientSocket) != clients.end();
}

void Channel::broadcast(const std::string& message, int excludeSocket) {
    for (std::set<int>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (*it != excludeSocket) {
            send(*it, message.c_str(), message.length(), 0);
        }
    }
}

bool Channel::isEmpty() const {
    return clients.empty();
}

const std::set<int>& Channel::getClients() const {
    return clients;
}
