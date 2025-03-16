/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/12 18:52:39 by acosi             #+#    #+#             */
/*   Updated: 2025/01/07 09:57:34 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/utils.hpp"

// As part of the IRC protocol, the client may ask for a list of the server capabilities (features).
void Server::CAP(int &fd, std::string &msg)
{
	if (msg.substr(0,6) == "CAP LS") {
		std::cout << GREEN "Replying to " << msg << RESET << std::endl;
		std::string capMsg = "CAP * LS :\r\n"; // No additionnal features supported
		send(fd, capMsg.c_str(), capMsg.size(), 0);}
	else if (msg.substr(0,7) == "CAP END")
		return;
}

void Server::PASS(int &fd, std::string &msg)
{
	if (msg.substr(0, 5) != "PASS ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }

	std::cout << GREEN "Replying to " << msg << RESET << std::endl;
	std::istringstream ss(msg);
	std::string buffer;
	std::getline(ss, buffer, ' '); // Skip the PASS tag
	std::getline(ss, buffer);
	if (buffer != _password) {
		std::cerr << RED "Invalid password from FD " << fd << RESET << std::endl;
		std::string errorMsg = ":Client 464 * :Password incorrect\r\n";
		send(fd, errorMsg.c_str(), errorMsg.size(), 0);
		close(fd);
		std::vector<pollfd>::iterator it = _fds.begin();
		for ( ; it != _fds.end(); ++it)
			if (it->fd == fd) {
				_fds.erase(it);
				break; } 
		std::cout << YELLOW "Client FD " << fd << " disconected" RESET << std::endl; }
	else {
		_clients[fd].passFlag = true;
		return; }
}

void Server::NICK(int &fd, std::string &msg)
{
	if (msg.substr(0, 5) != "NICK ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }
	
	std::cout << GREEN "Replying to " << msg << RESET << std::endl;
	
	// No nickname error handling
	size_t nicknameStart = msg.find(' ');
	if (nicknameStart == std::string::npos || !std::isprint(msg[5]) || std::isspace(msg[5])) {
		std::string noNick = ":" + _clients[fd].getServername() + " 431 :No nickname given\r\n";
		send(fd, noNick.c_str(), noNick.size(), 0);
		return; }
	
	// Exctract the requested nickname
	std::string nickname = msg.substr(nicknameStart + 1);
	nickname.erase(nickname.find_last_not_of("\n\r") + 1);

	// Parse the client map and search if the nickname is already taken
	// We use a flag to ensure the nickname is checked repeatedely against all suffixed nicknames_ until it is unique.
	bool uniqueFlag = false; 
	while (!uniqueFlag) {
		uniqueFlag = true;
		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			if (it->second.getNickname() == nickname) {
				nickname += '_';
				uniqueFlag = false;
				break; } } }

	// Assign the unique nickname to the client
	if (_clients[fd].getNickname() != "")
	{
		std::string oldname = _clients[fd].getNickname();
		_clients[fd].setNickname(nickname);
		_clients[fd].nickFlag = true;
		broadcastServer(_clients[fd], oldname, "NICK");
	}
	_clients[fd].setNickname(nickname);
	_clients[fd].nickFlag = true;

}

void Server::USER(int &fd, std::string &msg)
{
	if (msg.substr(0, 5) != "USER ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }
	
	std::vector<std::string> args;
	std::istringstream ss(msg);
	std::string buffer;
	std::getline(ss, buffer, ' '); // Skip the USER tag
	for (int count = 0; std::getline(ss, buffer, ' ') && count <= 3; ++count)
		args.push_back(buffer);
	if (args.size() == 3) {
		getline(ss, buffer);
		buffer.erase(buffer.find_last_not_of("\n\r") + 1);
		args.push_back(buffer);}
	
	if (args.size() != 4) {
		std::string errParam = "Error: USER 461 :Not enough parameters\r\n";
		send(fd, errParam.c_str(), errParam.size(), 0);
		return; }
	
    // Extracting username
    std::string username = args[0];
    _clients[fd].setUsername(username);

    // Extracting hostname
    std::string hostname = args[1] + "@" + _clients[fd].getIp();
    _clients[fd].setHostname(hostname);
    
    // Extracting servername
    std::string servername = args[2];
    _clients[fd].setServername(servername);

    // Extracting realname
    std::string realname = args[3];
    realname.erase(realname.find_last_not_of("\n\r") + 1);
	if (realname[0] == ':')
		realname.erase(0,1);
	else {
		std::cerr << RED "Error: malformed command. Realname must start with ':'" RESET << std::endl;
		return; }
    _clients[fd].setRealname(realname);

	_clients[fd].userFlag = true;
	std::cout << GREEN "Replying to " << msg << RESET << std::endl;
    // Send the first handshake welcome message
    std::string nickname = _clients[fd].getNickname();
    std::string welcomeMsg = "001 " + nickname + " :Welcome to the IRC Server, " + nickname + "[!" + hostname + "]\r\n";
	send(fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
}

// The WHOIS command is sent when a user connects with a nickname that is already taken.
// This is used by the IRC client to tell the user about the person owning the nickname.
void Server::WHOIS(int &fd, std::string &msg)
{
	if (msg.substr(0, 6) != "WHOIS ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }

	std::cout << GREEN "Replying to " << msg << RESET << std::endl;
	size_t nicknameStart = msg.find(' ');
	std::string nickname = msg.substr(nicknameStart + 1);
	nickname.erase(nickname.find_last_not_of("\n\r") + 1);

	// Search for the client with the requested nickname
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second.getNickname() == nickname) {
			// Build and send WHOIS response
			std::string whoisReply = 
				":" + _clients[fd].getServername() + " 311 " + _clients[fd].getNickname() + " " + 
				nickname + " " + it->second.getUsername() + " " +
				it->second.getHostname() + " * :" + it->second.getRealname() + "\r\n";
			send(fd, whoisReply.c_str(), whoisReply.size(), 0);

			// End of WHOIS response
			std::string endOfWhois = 
				":" + _clients[fd].getServername() + " 318 " + _clients[fd].getNickname() + 
				" " + nickname + " :End of /WHOIS list.\r\n";
			send(fd, endOfWhois.c_str(), endOfWhois.size(), 0);
			return; } }

	// If nickname not found
	std::string noSuchNick = 
		":" + _clients[fd].getServername() + " 401 " + _clients[fd].getNickname() + 
		" " + nickname + " :No such nick/channel\r\n";
	send(fd, noSuchNick.c_str(), noSuchNick.size(), 0);
}

// This command is used by the client to retrieve infos about another user or a channel.
void Server::WHO(int &fd, std::string &msg)
{
	if (msg.substr(0, 4) != "WHO ") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }

	std::string client = _clients[fd].getNickname();
	std::istringstream ss(msg);
	std::string target, error, reply;
	int targetFd;

	// Command parsing
	std::getline(ss, target, ' '); // Skip the WHO tag
	std::getline(ss, target); // Store the target in a string
	
	// Error handling for missing argument
	if (target.empty()) {
		error = "431 " + client + " :No nickname given\r\n";
		send(fd, error.c_str(), error.size(), 0); 
		return; }
	
	std::cout << GREEN "Replying to " << msg << RESET << std::endl;
	// Target is a channel
	if (target[0] == '#' || target[0] == '&') { 
			// Check that the channel exists
			if (_channels.find(target) == _channels.end()) {
				error = "403 " + client + " " + target + " :No such channel\r\n";
				send(fd, error.c_str(), error.size(),0);
				return; } 
			
			// Parse the userlist for this channel and send the list of WHO reply with their infos
			Channel &chan = _channels[target];
			std::map<int, bool>::iterator it1 = chan.userList.begin();
			for ( ; it1 != chan.userList.end(); ++it1) {
				std::map<int, Client>::iterator it2 = _clients.begin();
				for ( ; it2 != _clients.end(); ++it2) {
					if (it2->second.getNickname() == _clients[it1->first].getNickname()) {
						targetFd = it2->second.getFd();
						std::string user = _clients[targetFd].getUsername();
						std::string host = _clients[targetFd].getHostname();
						std::string server = _clients[targetFd].getServername();
						std::string nick = _clients[targetFd].getNickname();
						std::string realname = _clients[targetFd].getRealname();
						reply = "352 " + client + " "  + target + " " + user + " " + 
								host + " " + server + " " + nick + " H :1 " + realname + "\r\n";
						send(fd, reply.c_str(), reply.size(), 0); } } } }
	
	// Taget is a nickname
	else {
		// Check that the target nickname exists
		std::map<int, Client>::iterator	it = _clients.begin();
		for ( ; it != _clients.end(); ++it) {
			if (it->second.getNickname() == target) {
				targetFd = it->second.getFd();
				break; } }
		if (it == _clients.end()) {
			error =	"401 " + client + " " + target + " :No such nickname\r\n";
			send(fd, error.c_str(), error.size(), 0);
			return ; }

		// Send the WHO reply with target infos to the client
		std::string user = _clients[targetFd].getUsername();
		std::string host = _clients[targetFd].getHostname();
		std::string server = _clients[targetFd].getServername();
		std::string nick = _clients[targetFd].getNickname();
		std::string realname = _clients[targetFd].getRealname();
		reply =	"352 " + client + " "  + target + " " + user + " " + 
				host + " " + server + " " + nick + " H :1 " + realname + "\r\n";
		send(fd, reply.c_str(), reply.size(), 0); }

	// End of WHO reply
	reply = "315 " + client + " " + target + " :End of WHO list.\r\n";
	send(fd, reply.c_str(), reply.size(), 0);
}

void Server::PING(int &fd, std::string &msg)
{
	if (msg.substr(0, 4) == "PING") {
		std::cout << GREEN "Replying to " << msg << RESET << std::endl;
		std::string pingMsg = "PONG " + _clients[fd].getServername() + "\r\n";
		send(fd, pingMsg.c_str(), pingMsg.size(), 0); }
}

void Server::QUIT(int &fd, std::string &msg)
{
	if (msg.substr(0, 4) != "QUIT") {
		std::cerr << RED "Error: malformed command." RESET << std::endl;
		return; }

	std::string reason = "";
	size_t pos = msg.find(':');
	if (pos != std::string::npos) {
		std::istringstream ss(msg);
		std::getline(ss, reason, ':');
		std::getline(ss, reason);}
	std::cout << GREEN "Replying to " << msg << RESET << std::endl;
	std::string quitMsg = ':' + _clients[fd].getNickname() + "!" + _clients[fd].getHostname() + " QUIT " + reason + "\r\n";
	send(fd, quitMsg.c_str(), quitMsg.size(), 0);
	
	
	// Remove client from all channels
	std::map<std::string, bool>::iterator it = _clients[fd].chanList.begin();
	for ( ; it != _clients[fd].chanList.end(); ++it) {
		std::string chan = _channels[it->first].getChannelName();
		
		// Make a copy of the clients vector to avoid iterator invalidation
  		std::vector<Client *> copy = _channels[chan].clients;
		
		// Notify other clients in the channel
		std::string reply = ":" + _clients[fd].getNickname() + "!" + _clients[fd].getHostname() + 
			" PART " + chan + ((reason != "") ? (" :" + reason + "\r\n") : " :disconnected\r\n");
		for (std::vector<Client *>::iterator cIt = copy.begin(); cIt != copy.end(); ++cIt)
			if ((*cIt)->getFd() != fd)
				send((*cIt)->getFd(), reply.c_str(), reply.size(), 0);
		
		 // Remove the quitting client from the channel
		_channels[it->first].removeClient(_clients[fd].getNickname()); }
	
	// Remove from client list
	_clients.erase(fd);

	// Close fd and remove from fd list
	close(fd);
	for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it) {
		if (it->fd == fd) {
			_fds.erase(it);
			break; } }
	
	std::cout << YELLOW "Client FD " << fd << " disconected" RESET << std::endl;
}