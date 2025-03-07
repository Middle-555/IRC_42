/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 23:47:29 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/06 23:47:32 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

Client::Client(int fd) : socketFd(fd), authenticated(false) {
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
}
