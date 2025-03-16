/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/12 17:57:14 by acosi             #+#    #+#             */
/*   Updated: 2024/12/29 22:19:59 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/utils.hpp"

void Server::signalHandler(int signum)
{
	if (signum == SIGINT)
		std::cerr << RED "\nircserv interrupted by signal SIGINT" RESET << std::endl;
	else if (signum == SIGQUIT)
		std::cerr << RED "\nircserv core dump caused by signal SIGQUIT" RESET << std::endl;
	Server::_signal = true;
}

// Prevent nc from bypassing server password
bool Server::checkPassword(int &fd, std::string &msg, bool &resetCommand)
{
	if (_clients.find(fd) == _clients.end()) {
        std::cout << RED "Error: invalid fd" RESET << std::endl;
        return false; }
	if (_clients[fd].passFlag != true) {
		if (msg.substr(0, 6) == "CAP LS") {
			CAP(fd, msg);
			return false; }
		else if (msg.substr(0, 5) == "PASS ") {
			PASS(fd, msg);
			if (_clients[fd].passFlag == true) {
				resetCommand = true;
				return true; }
			else
				return false; }
		else {
			std::string pswdReply = "Client :Password required\r\n";
			send(fd, pswdReply.c_str(), pswdReply.size(), 0);
			return false; } }
	else
		return true;
}

// Check that the USER command is complete and valid to prevent empty data alter on
bool Server::checkUser(int &fd, std::string &msg, bool &resetCommand)
{
	if (_clients[fd].userFlag != true) {
		if (msg.substr(0, 5) == "USER ") {
			USER(fd, msg);
			if (_clients[fd].userFlag == true) {
				resetCommand = true;
				return true; }
			else
				return false; }
		else {
			std::string userReply = "Error: user infos required\r\n";
			send(fd, userReply.c_str(), userReply.size(), 0);
			return false; } }
	else
		return true;
}

// Check that the client has set a valid nickname before proceeding
bool Server::checkNickname(int &fd, std::string &msg, bool &resetCommand)
{
	if (_clients[fd].nickFlag != true)
		if (msg.substr(0, 5) == "NICK ") {
			NICK(fd, msg);
			if (_clients[fd].nickFlag == true) {
				resetCommand = true;
				return true; }
			else
				return false; }
		else {
			std::string nickReply = "Error: nickname required\r\n";
			send(fd, nickReply.c_str(), nickReply.size(), 0);
			return false; }
	else
		return true;
}

// Check that the client sends the appropriate commands during the first messages
bool Server::firstHandshake(int &fd, std::string &msg, bool &resetCommand)
{
	if (!checkPassword(fd, msg, resetCommand) || resetCommand)
		return false;
	if (!checkNickname(fd, msg, resetCommand) || resetCommand)
		return false;
	if (!checkUser(fd, msg, resetCommand) || resetCommand)
		return false;
	return true;
}

void Server::broadcastServer(Client &client, std::string oldname, std::string type)
{
	std::string joinReply = ":" + oldname + "!" + client.getNickname() + "@" + client.getUsername() + " " + type + " " + client.getNickname() + "\r\n";
	std::map<int, Client>::iterator it = _clients.begin();
	for ( ; it != _clients.end(); ++it)
		send(it->first, joinReply.c_str(), joinReply.size(), 0);
}