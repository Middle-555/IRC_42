/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 15:49:43 by acabarba          #+#    #+#             */
/*   Updated: 2025/04/01 17:16:13 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"
#include <sys/socket.h>
#include <iostream>

Channel::Channel(const std::string& channelName)
    : name(channelName), userLimit(0), inviteOnly(false), topicRestricted(false) {}

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

/**
 * Gestion des opérateurs
 */
void Channel::addOperator(int clientSocket) {
    operators.insert(clientSocket);
}

void Channel::removeOperator(int clientSocket) {
    operators.erase(clientSocket);
}

bool Channel::isOperator(int clientSocket) const {
    return operators.find(clientSocket) != operators.end();
}

/**
 * Gestion des modes
 * Gueteurs et Seteurs
 */
void Channel::setInviteOnly(bool state) {
    inviteOnly = state;
}

bool Channel::getInviteOnly() const {
    return inviteOnly;
}

void Channel::setTopicRestricted(bool state) {
    topicRestricted = state;
}

bool Channel::getTopicRestricted() const {
    return topicRestricted;
}

void Channel::setPassword(const std::string& pass) {
    password = pass;
}

std::string Channel::getPassword() const {
    return password;
}

void Channel::setUserLimit(int limit) {
    userLimit = limit;
}

int Channel::getUserLimit() const {
    return userLimit;
}

void Channel::setTopic(const std::string& newTopic) {
    topic = newTopic;
}

std::string Channel::getTopic() const {
    return topic;
}

bool Channel::isInvited(int clientSocket) const {
    return (invitedClients.find(clientSocket) != invitedClients.end());
}


void Channel::inviteClient(int clientSocket) {
    invitedClients.insert(clientSocket);
}

void Channel::removeInvitation(int clientSocket) {
    invitedClients.erase(clientSocket);
}

