/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 23:10:56 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/11 22:07:05 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

int main(int argc, char **argv) {
    if (argc != 3 || !is_valid_port(argv[1])) {
        std::cerr << "Usage: ./ircserv <port(1024-65535)> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    try {
        Server server(port, password);
        std::cout << "IRC Server started on port " << port << std::endl;
        server.run();
    }
    catch (std::exception &e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
