/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acabarba <acabarba@42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/11 22:06:20 by acabarba          #+#    #+#             */
/*   Updated: 2025/03/11 23:48:39 by acabarba         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

bool is_valid_port(const char* str) {
    char* end;
    long port = strtol(str, &end, 10);
    return (*end == '\0' && port >= 1024 && port <= 65535);
}