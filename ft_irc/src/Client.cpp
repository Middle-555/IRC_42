/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 23:47:29 by acabarba          #+#    #+#             */
/*   Updated: 2025/04/01 17:19:55 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

/**
 * Constructeur & destructeurs
 */
Client::Client(int fd) : socketFd(fd), authenticated(false), buffer("") {
    std::cout << "👤 Création d'un nouveau client (fd: " << fd << ")" << std::endl;
}

Client::~Client() {
    std::cout << "👋 Déconnexion du client (fd: " << socketFd << ")" << std::endl;
}

/**
 * Gueteurs et seteurs
 */
int Client::getSocketFd() const {
    return socketFd;
}

std::string Client::getNickname() const {
    return nickname;
}

std::string Client::getUsername() const {
    return username;
}


void Client::setNickname(const std::string& nick) {
    nickname = nick;
}

void Client::setUsername(const std::string& user) {
    username = user;
}

void Client::setRealname(const std::string& name) {
    realname = name;
}

/**
 * Authentification
 */
bool Client::isAuthenticated() const {
    return authenticated;
}

void Client::authenticate() {
    authenticated = true;
    std::cout << "✅ Client (fd: " << socketFd << ") authentifié !" << std::endl;
    std::cout << std::endl;
}

bool Client::isFullyRegistered() const {
    return !nickname.empty() && !username.empty() && !realname.empty() && isAuthenticated();
}

void Client::setCurrentChannel(const std::string& channel) {
    currentChannel = channel;
}

/**
 * Gestion des autorisation et messages incomplet
 */
std::string Client::getCurrentChannel() const {
    return currentChannel;
}


std::string& Client::getBufferRef() {
    return this->buffer;
}

void Client::appendToBuffer(const char* receiveBuffer, size_t length) {
    this->buffer.append(receiveBuffer, length);
}

std::string Client::extractNextMessage() {
    size_t pos = this->buffer.find("\r\n");
    bool foundCRLF = true;
    if (pos == std::string::npos) {
        pos = this->buffer.find("\n");
        foundCRLF = false;
    }
    if (pos == std::string::npos) {
        return "";
    }
    std::string message = this->buffer.substr(0, pos);
    if (foundCRLF) {
        this->buffer.erase(0, pos + 2);
    } else {
        this->buffer.erase(0, pos + 1);
    }
    return message;
}