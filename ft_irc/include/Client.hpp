/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:07:20 by kpourcel          #+#    #+#             */
/*   Updated: 2025/04/01 17:30:01 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>

class Client {
private:
    int             socketFd;
    std::string     nickname;
    std::string     username;
    std::string     hostname;
    std::string     realname;
    bool            authenticated;
    std::string     currentChannel;
    std::string     buffer;

public:
    Client(int fd);
    ~Client();

    int         getSocketFd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    void        setRealname(const std::string& name);
    bool        isAuthenticated() const;
    
    void        setNickname(const std::string& nick);
    void        setUsername(const std::string& user);
    void        authenticate();

    void        setCurrentChannel(const std::string& channel);
    std::string getCurrentChannel() const;

    bool        isFullyRegistered() const;

    std::string& getBufferRef();
    void appendToBuffer(const char* receiveBuffer, size_t length);
    std::string extractNextMessage();

};

#endif
