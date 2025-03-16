/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   operator.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/29 00:22:29 by acosi             #+#    #+#             */
/*   Updated: 2025/01/07 12:30:03 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/utils.hpp"

void Server::TOPIC(int &fd, std::string &msg)
{
	(void)fd;
	if (msg.substr(0, 6) != "TOPIC ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }
	msg = msg.substr(6); // Skip the PART tag
	size_t chanPos = msg.find('#');
	if (chanPos == std::string::npos || !std::isprint(msg[chanPos + 1])) {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }
	std::string chan = msg.substr(chanPos);
	size_t chanEnd = chan.find(' ');
	if (chanEnd == std::string::npos || !std::isprint(chan[chanEnd + 1])) {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }
	chan = chan.substr(chanPos, chanEnd);
	msg = msg.substr(chanEnd);
	size_t topicPos = msg.find(':');
	if (topicPos == std::string::npos){
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }
	msg = msg.substr(topicPos + 1);
	if (_channels.find(chan) != _channels.end())
	{
		if (!std::isprint(msg[0]))
			_channels[chan].removeTopic(_clients[fd]);
		else
			_channels[chan].setTopic(_clients[fd], msg);
	}
}

void Server::KICK(int &fd, std::string &msg)
{
	if (msg.substr(0, 5) != "KICK ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }

	std::string client = _clients[fd].getNickname();
	std::string host = _clients[fd].getHostname();
	std::vector<std::string> arg;
	std::istringstream ss(msg);
	std::string buffer, chan, target, reply, reason, error;
	int targetFD;
	
	// Command parsing
	for (int i = 0; i < 3; ++i) {
		std::getline(ss, buffer, ' ');
		arg.push_back(buffer); }
	std::getline(ss, buffer); // The reason starts with ':' and might contain spaces
	arg.push_back(buffer);
	
	if (arg.size() < 3) {
		error = "461 " + client + " KICK :Not enough parameters\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	
	// Check if a reason for the kick was provided
	if (arg.size() > 3 && !arg[3].empty() && arg[3][0] == ':' && std::isprint(arg[3][1]))
		reason = arg[3];
	else // Assign a default reason
		reason = "no lollygagging";
	
	chan = arg[1];
	target = arg[2];
	std::cout << GREEN "Replying to " << msg << RESET << std::endl;
	
	// Check that the channel exists
	if (_channels.find(chan) == _channels.end()) {
		error = "403 " + client + " " + chan + " :No such channel\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	// Check that the client is on the channel
	if (_channels[chan].userList.find(fd) == _channels[chan].userList.end()) {
		error = "442 " + client + " " + chan + " :You're not on that channel\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	// Check that the client has operator privileges
	if (_channels[chan].userList[fd] == false) {
		error = "482 " + client + " " + chan + " :You're not channel operator\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	// Check that the target user is on the channel
	std::vector<Client *>::iterator it = _channels[chan].clients.begin(); 
	for ( ; it != _channels[chan].clients.end(); ++it){
		if ((*it)->getNickname() == target) {
			targetFD = (*it)->getFd();
			break ; }
	}
	if (it == _channels[chan].clients.end()) {
		error = "441 " + client + " " + target + " " + chan + " :They aren't on that channel\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	// Send the kick notice to every client in the channel
	reply =	":" + client + "!" + host + " KICK " + chan + " " + target + " " + reason + "\r\n";
	std::vector<Client *>::iterator its = _channels[chan].clients.begin();
	for ( ; its != _channels[chan].clients.end(); its++)
		send((*its)->getFd(), reply.c_str(), reply.size(), 0);
	// Remove the target from the channel's client list
	_channels[chan].removeClient(target);
	std::map<std::string, bool>::iterator itr = _clients[targetFD].chanList.find(chan);
	if (itr != _clients[targetFD].chanList.end())
		_clients[targetFD].chanList.erase(itr);
}

void	Server::INVITE(int &fd, std::string &msg)
{
	if (msg.substr(0, 7) != "INVITE ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }
	std::vector<std::string> arg;
	std::istringstream ss(msg);
	std::string buffer, target, chan, client, reply, error;
	client = _clients[fd].getNickname();
	while (std::getline(ss, buffer, ' '))
	{
		arg.push_back(buffer);
	}
	if (arg.size() != 3){
		error = "461 " + client + " INVITE :Not enough parameters\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	target = arg[1];
	std::map<int, Client>::iterator	it = _clients.begin();
	for ( ; it != _clients.end(); ++it){
		if (it->second.getNickname() == target)
			break;}
	if (it == _clients.end()){
		std::string noSuchNick = 
		":" + _clients[fd].getServername() + " 401 " + _clients[fd].getNickname() + 
		" " + target + " :No such nick/channel\r\n";
		send(fd, noSuchNick.c_str(), noSuchNick.size(), 0);
		return ; }
	chan = arg[2];
	if (_channels.find(chan) == _channels.end()) {
		error = "403 " + client + " " + chan + " :No such channel\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	if (_channels[chan].userList.find(fd) == _channels[chan].userList.end()) {
		error = "442 " + client + " " + chan + " :You're not on that channel\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	if (_channels[chan].userList[fd] == false) {
		error = "482 " + client + " " + chan + " :You're not channel operator\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	reply = ":" + client + "!" + _clients[fd].getHostname() + " INVITE " + target + " " + chan + "\r\n";
	send (it->second.getFd(), reply.c_str(), reply.size(), 0);
	std::map<int, Client>::iterator ite = _clients.begin();
	for ( ; ite != _clients.end(); ++ite)
	{
		if (ite->second.getNickname() == target)
		{
			_channels[chan].invites.push_back(ite->second.getFd());
			break ;
		}
	}
}

void Server::MODE(int &fd, std::string &msg)
{
	if (msg.substr(0, 5) != "MODE ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }
	
	std::string client = _clients[fd].getNickname();
	std::vector<std::string> arg;
	std::istringstream ss(msg);
	std::string buffer, target, param, error;	

	// Command parsing
	while (std::getline(ss, buffer, ' '))
		arg.push_back(buffer);
	
	if (arg.size() < 2) {
		error = "461 " + client + " MODE :Not enough parameters\r\n";
		send(fd, error.c_str(), error.size(),0);
		return; }
	
	target = arg[1];
	if (arg.size() >= 2)
		param = arg[1];
	int targetType;
	if (target[0] == '#' || target[0] == '&')
		targetType = 1;
	else
		targetType = 0;
	std::cout << GREEN "Replying to " << msg << RESET << std::endl;
		
	// Check that target user exists on the server
	if (targetType == 0) {
		std::map<int, Client>::iterator	it = _clients.begin();
		for ( ; it != _clients.end(); ++it){
			if (it->second.getNickname() == target)
				break;}
		if (it == _clients.end()){
			std::string noSuchNick = 
			":" + _clients[fd].getServername() + " 401 " + _clients[fd].getNickname() + 
			" " + target + " :No such nick/channel\r\n";
			send(fd, noSuchNick.c_str(), noSuchNick.size(), 0);
			return ; }
		if (arg.size() >= 2 && param == "+i") {
			std::string reply = ':' + _clients[fd].getServername() + " MODE " + _clients[fd].getNickname() + " +i\r\n";
			send(fd, reply.c_str(), reply.size(), 0); } }
	if (targetType == 1) {
			// Check that the channel exists
			if (_channels.find(target) == _channels.end()) {
				error = "403 " + client + " " + target + " :No such channel\r\n";
				send(fd, error.c_str(), error.size(),0);
				return; }
			if (arg.size() == 2)
				_channels[arg[1]].printMode(fd, _clients[fd]);
			else
				_channels[arg[1]].setMode(_clients[fd], arg);
	}
}