/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/04 12:36:17 by keschouf          #+#    #+#             */
/*   Updated: 2024/12/08 01:28:48 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"
#include "../include/Server.hpp"
#include "../include/utils.hpp"

int main(int ac, char **av)
{
	if (ac != 3) {
		std::cerr << RED << "Error: wrong number of arguments\n" YELLOW "Usage: " RESET
		 "./ircserv <" BLUE "port" RESET "> <" BLUE "password" RESET ">"<< std::endl;
		return 1; }

	try {
		Server irc(atoi(av[1]), av[2]);
		signals();
		irc.serverInit(); }
	catch (const std::exception &e) {
		std::cerr << RED << e.what() << std::endl;
		return 1; }
}