/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kpourcel <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/10 15:50:02 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/18 16:46:03 by kpourcel         ###   ########.fr       */
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
    std::set<int> operators;
    std::string topic;
    std::string password;
    int userLimit;
    bool inviteOnly;
    bool topicRestricted;
    std::set<int> invitedClients;


public:
    Channel(const std::string& channelName);
    
    void addClient(int clientSocket);
    void removeClient(int clientSocket);
    bool isClientInChannel(int clientSocket) const;
    void broadcast(const std::string& message, int excludeSocket);
    bool isEmpty() const;
    const std::set<int>& getClients() const;

    // Gestion des opérateurs
    void addOperator(int clientSocket);
    void removeOperator(int clientSocket);
    bool isOperator(int clientSocket) const;

    // Gestion des modes
    void setInviteOnly(bool state);
    bool getInviteOnly() const;

    void setTopicRestricted(bool state);
    bool getTopicRestricted() const;

    void setPassword(const std::string& pass);
    std::string getPassword() const;

    void setUserLimit(int limit);
    int getUserLimit() const;

    void setTopic(const std::string& newTopic);
    std::string getTopic() const;

    void inviteClient(int clientSocket);
    bool isInvited(int clientSocket) const;
    void removeInvitation(int clientSocket);

};

#endif
