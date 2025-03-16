/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channels.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/29 23:39:06 by acosi             #+#    #+#             */
/*   Updated: 2025/01/07 09:57:15 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/utils.hpp"

void Server::PRIVMSG(int &fd, std::string &msg)
{
	if (msg.substr(0, 8) != "PRIVMSG ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }

	// Command parsing
	std::string client = _clients[fd].getNickname();
	std::vector<std::string> msgArgs;
	std::istringstream ss(msg);
	std::string arg;
	std::getline(ss, arg, ' '); // Skip the PRIVMSG tag
	std::getline(ss, arg, ' ');
	msgArgs.push_back(arg); // Store #channel or the recipient name
	std::getline(ss, arg);
	msgArgs.push_back(arg); // Store the message starting with ':'
	std::string reply =	":" + client + "!~" + _clients[fd].getHostname() + 
						" PRIVMSG " + msgArgs[0] + " " + msgArgs[1] + "\r\n";
	
	// Error handling for missing arguments
	if (msgArgs.size() < 2 || msgArgs[0].empty() || msgArgs[1].empty()) {
		reply = _clients[fd].getNickname() + " :No text to send\r\n";
			if (msgArgs[0].empty())
				reply = ":" + _clients[fd].getServername() + " 431 " + client +  " :No nickname given\r\n";
		send(fd, reply.c_str(), reply.size(), 0); 
		return; }
	
	std::cout << GREEN "Replying to " << msg << RESET << std::endl;
	
	// Check that the client is a member of the channel before sending it
		
	// Send the reply in the channel if a '#' or '&' character was found
	if (msg[8] == '#' || msg[8] == '&')
	{
		std::vector<Client *>::iterator it1 = _channels[msgArgs[0]].clients.begin();
		for ( ; it1 != _channels[msgArgs[0]].clients.end(); ++it1)
		{
			if ((*it1)->getNickname() == _clients[fd].getNickname())
				break; 
		}
		if (it1 == _channels[msgArgs[0]].clients.end())
		{
			std::string errReply = ":" + _clients[fd].getNickname() + " 404 " + msgArgs[0] + " :Cannot send to channel\r\n";
			send(fd, errReply.c_str(), errReply.size(), 0);
			return; 
		}
	// Sending reply to every clients in the channel
		std::vector<Client *>::iterator it = _channels[msgArgs[0]].clients.begin();
		for ( ; it != _channels[msgArgs[0]].clients.end(); ++it) 
		{
			if (it != _channels[msgArgs[0]].clients.end() && (*it)->getFd() != fd) 
				send((*it)->getFd(), reply.c_str(), reply.size(), 0);
		} 
	}
		
	// Send the reply to a specific client fd (private message)
	else 
	{
		std::map<int, Client>::iterator it2 = _clients.begin();
		for ( ; it2 != _clients.end(); ++it2)
		{
			if (it2->second.getNickname() == msgArgs[0])
			{
				send(it2->second.getFd(), reply.c_str(), reply.size(), 0);
				return ;
			}
		}
		if (it2 == _clients.end())
		{
			reply = msgArgs[0] + " :No such nickname/channel\r\n";
			send(fd, reply.c_str(), reply.size(), 0);
		} 
	}
}


void Server::JOIN(int &fd, std::string &msg)
{
	if (msg.substr(0, 5) != "JOIN ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }

	// Ensure correct format, storing channels names and keys
	std::vector<std::string> chans;
	std::vector<std::string> keys;
	if (!joinParsing(msg, &chans, &keys))
		return ;
	
	std::cout << GREEN "Replying to " << msg << RESET << std::endl;
	joinChannels(fd, chans, keys);
}

void Server::PART(int &fd, std::string &msg)
{
	if (msg.substr(0, 5) != "PART ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }
	std::string client, chan, reason, error;
	client = _clients[fd].getNickname();
	reason = "";
	std::string tmp = msg.substr(5); // Skip the PART tag
	size_t chanPos = tmp.find('#');
	if (chanPos == std::string::npos || !std::isprint(tmp[chanPos + 1])) {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }
	size_t spPos = tmp.find(' ');
	if (spPos != std::string::npos) {
		chan = tmp.substr(chanPos, spPos);
		reason = tmp.substr(spPos); }
	else {
		chan = tmp.substr(chanPos); }
	
	// Check that the channel exists
	if (_channels.find(chan) == _channels.end()) {
		error = "403 " + client + " " + chan + " :No such channel\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	// Check that the client has access to the channel
	if (_channels[chan].userList.find(fd) == _channels[chan].userList.end()) {
		error = "442 " + client + " " + chan + " :You're not on that channel\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	_channels[chan].broadcastChannel(_clients[fd], "PART", reason);
	_channels[chan].removeClient(_clients[fd].getNickname());
	// Remove from chanList
	std::map<std::string, bool>::iterator itr = _clients[fd].chanList.find(chan);
	if (itr != _clients[fd].chanList.end())
		_clients[fd].chanList.erase(itr);
}