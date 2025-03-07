/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:07:20 by kpourcel          #+#    #+#             */
/*   Updated: 2025/03/07 00:06:58 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>

class Client {
private:
    int         socketFd;       // Socket du client
    std::string nickname;       // Pseudo du client
    std::string username;       // Nom d'utilisateur
    std::string hostname;       // Nom d'hôte (adresse IP)
    std::string realname;       // Nom réel du client
    bool        authenticated;  // Vérifie si le client est authentifié

public:
    Client(int fd);
    ~Client();

    int         getSocketFd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    void setRealname(const std::string& name);
    bool        isAuthenticated() const;
    
    void        setNickname(const std::string& nick);
    void        setUsername(const std::string& user);
    void        authenticate();
};

#endif
