/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 15:50:02 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/11 23:23:59 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

class Channel {
private:
    std::string name;
    std::set<int> clients;

public:
    Channel(const std::string& channelName);
    void addClient(int clientSocket);
    void removeClient(int clientSocket);
    bool isClientInChannel(int clientSocket) const;
    void broadcast(const std::string& message, int excludeSocket = -1);
    bool isEmpty() const;
    const std::set<int>& getClients() const;
};

#endif