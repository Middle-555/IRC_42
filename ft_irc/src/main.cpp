/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 23:10:56 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/11 23:48:16 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

Server* globalServerPtr = NULL;  // Pointeur global vers le serveur

void signalHandler(int signum) {
    if (globalServerPtr) {
        std::cout << "\n🛑 Signal reçu (" << signum << "), arrêt du serveur...\n";
        globalServerPtr->shutdownServer();
    }
}

int main(int argc, char **argv) {
    if (argc != 3 || !is_valid_port(argv[1])) {
        std::cerr << "Usage: ./ircserv <port(1024-65535)> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    try {
        Server server(port, password);
        globalServerPtr = &server;  // Associer le serveur au pointeur global

        // ✅ Utilisation de `sigaction` pour capturer `CTRL + C`
        struct sigaction sigIntHandler;
        sigIntHandler.sa_handler = signalHandler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, NULL);

        std::cout << "IRC Server started on port " << port << std::endl;
        server.run();
    }
    catch (std::exception &e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
