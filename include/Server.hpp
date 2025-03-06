/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kpourcel <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:07:11 by kpourcel          #+#    #+#             */
/*   Updated: 2025/03/06 15:31:00 by kpourcel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef 	SERVER_HPP
# define	SERVER_HPP

# include <iostream>
# include <sys/socket.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdlib.h>

class Server
{
private:
	std::string	_password;
	int	_sockfd;
	int	_pollfd;
public:
	Server();
	~Server();
};





#endif