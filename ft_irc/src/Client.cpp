/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kpourcel <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 23:47:29 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/19 19:19:59 by kpourcel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

Client::Client(int fd) : socketFd(fd), authenticated(false), buffer("") {
    std::cout << "👤 Création d'un nouveau client (fd: " << fd << ")" << std::endl;
}

Client::~Client() {
    std::cout << "👋 Déconnexion du client (fd: " << socketFd << ")" << std::endl;
}

int Client::getSocketFd() const {
    return socketFd;
}

std::string Client::getNickname() const {
    return nickname;
}

std::string Client::getUsername() const {
    return username;
}

bool Client::isAuthenticated() const {
    return authenticated;
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

void Client::authenticate() {
    authenticated = true;
    std::cout << "✅ Client (fd: " << socketFd << ") authentifié !" << std::endl;
    std::cout << std::endl;
}

void Client::setCurrentChannel(const std::string& channel) {
    currentChannel = channel;
}

std::string Client::getCurrentChannel() const {
    return currentChannel;
}

bool Client::isFullyRegistered() const {
    return !nickname.empty() && !username.empty() && !realname.empty() && isAuthenticated();
}

std::string& Client::getBufferRef() {
    return this->buffer;
}

void Client::appendToBuffer(const char* receiveBuffer, size_t length) {
    // Concatène les données reçues au buffer interne
    this->buffer.append(receiveBuffer, length);
}

std::string Client::extractNextMessage() {
    // On cherche d'abord la séquence CR+LF, qui est le délimiteur standard IRC
    size_t pos = this->buffer.find("\r\n");
    bool foundCRLF = true;

    // Si CR+LF n'est pas trouvé, on cherche simplement LF
    if (pos == std::string::npos) {
        pos = this->buffer.find("\n");
        foundCRLF = false;
    }

    // Si aucun délimiteur n'est trouvé, on retourne une chaîne vide (commande incomplète)
    if (pos == std::string::npos) {
        return "";
    }

    // Extraire la commande (sans le délimiteur)
    std::string message = this->buffer.substr(0, pos);

    // Supprimer la commande et le délimiteur du buffer
    if (foundCRLF) {
        // Supprime "\r\n" (2 caractères)
        this->buffer.erase(0, pos + 2);
    } else {
        // Supprime "\n" (1 caractère)
        this->buffer.erase(0, pos + 1);
    }

    return message;
}