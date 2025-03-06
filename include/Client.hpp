/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kpourcel <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 15:07:20 by kpourcel          #+#    #+#             */
/*   Updated: 2025/03/06 15:30:58 by kpourcel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef 	CLIENT_HPP
# define	CLIENT_HPP

# include <iostream>
# include <string>

class Client
{
private:
	std::string _nickname;
	std::string _username;
	std::string _hostname;
	int	_id;
public:
	Client();
	~Client();
};





#endif