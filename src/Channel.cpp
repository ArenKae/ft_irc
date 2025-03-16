/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/11 15:24:55 by keschouf          #+#    #+#             */
/*   Updated: 2025/01/06 15:51:57 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "utils.hpp"

Channel::Channel() : _name(), _key(), _server(), isTopic(false)
{
	std::time(&_creationTime);
	modes['i'] = 0;
	modes['t'] = 1;
	modes['k'] = 0;
	modes['l'] = 0;
}

Channel::Channel(std::string &name, Server* server) : 
	_name(name), _key(""), _server(server), isTopic(false)
{
	std::time(&_creationTime);
	modes['i'] = 0;
	modes['t'] = 1;
	modes['k'] = 0;
	modes['l'] = 0;
}

Channel::~Channel() {}

std::string	Channel::getChannelName() {return (_name);}

// Add a client to the channel
bool		Channel::addClient(Client *client)
{
	if (checkClient(client->getNickname()))
		return false;
	clients.push_back(client);
	client->chanList[getChannelName()] = false;
	if (isTopic == true) {
		std::string msg = "332 " + client->getNickname() + " " + getChannelName() + " :" + topic + "\r\n";
		send (client->getFd(), msg.c_str(), msg.size(), 0);
	}
	std::ostringstream oss;
	oss << _creationTime;
	std::string tmp = oss.str();
	std::string msg = "329 " + client->getNickname() + " " + getChannelName() + " " + tmp + "\r\n";
	send (client->getFd(), msg.c_str(), msg.size(), 0);
	return true;
}

// Add a client's name to the list of the channel's operators
void		Channel::addOp(Client &client, std::string &target)
{
	(void)client;
	for (std::vector<Client *>::iterator it = clients.begin(); it != clients.end(); it++){
		if ((*it)->getNickname() == target)
		userList[(*it)->getFd()] = true;}
	for (std::vector<Client *>::iterator it = clients.begin(); it != clients.end(); it++) {
			std::string msg = ":" + (*it)->getServername() + " MODE " + getChannelName() + " +o " + target + "\r\n";
			send((*it)->getFd(), msg.c_str(), msg.size(), 0); }
}


void		Channel::removeOp(Client &client, std::string &target)
{
	(void)client;
	for (std::vector<Client *>::iterator it = clients.begin(); it != clients.end(); it++){
		if ((*it)->getNickname() == target)
		userList[(*it)->getFd()] = false;}
	for (std::vector<Client *>::iterator it = clients.begin(); it != clients.end(); it++) {
		if ((*it)->getNickname() == target) {
			std::string msg = ":" + (*it)->getServername() + " MODE " + getChannelName() + " -o " + target + "\r\n";
			send((*it)->getFd(), msg.c_str(), msg.size(), 0); } }
}

// When joining a channel, add the client to a map storing all current users
// The opFlag is set to true if the client is an operator.
void		Channel::populateUserList(Client &client, bool opFlag)
{
	userList[client.getFd()] = opFlag;
}

void		Channel::broadcastChannel(Client &client, std::string type, std::string message)
{
	std::vector<Client *>::iterator it = clients.begin();
	for ( ; it != clients.end(); ++it)
	{
		std::string joinReply = ":" + client.getNickname() + "!" + client.getHostname() + " " + type + " " + getChannelName() + " :" + message + "\r\n";
		send((*it)->getFd(), joinReply.c_str(), joinReply.size(), 0); }
}

// When a client join a channel, parse the map storing all current users and send their names to the new client
// This is mostly useful to know which user are operators and regular users.
void		Channel::getClientList(int &fd, Client &client)
{
	std::string names;
	for (std::vector<Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (userList[(*it)->getFd()] == true)
			names += "@";
		names += (*it)->getNickname();
		names += " ";
	}
	std::string nameReply = ":" + client.getServername() + " 353 " + client.getNickname() + " = " + getChannelName()+ " :" + names + "\r\n";
	send(fd, nameReply.c_str(), nameReply.size(), 0);
	std::string endList =  ":" + client.getServername() + " 366 " + client.getNickname() + " " + getChannelName() + " :End of NAMES list\r\n";
	send(fd, endList.c_str(), endList.size(), 0);
}

void		Channel::removeClient(std::string &name)
{
	std::vector<Client *>::iterator it = clients.begin();
	for ( ; it != clients.end(); ++it) {
		if ((*it)->getNickname() == name) {
			userList.erase((*it)->getFd());
			clients.erase(it);
			break; } }
	if (clients.empty())
		_server->markForRemove(getChannelName());
}

bool		Channel::checkClient(std::string &name)
{
	std::vector<Client *>::iterator it = clients.begin();
	for ( ; it != clients.end(); ++it)
	{
		if ((*it)->getNickname() == name)
		return true;
	}
	return false;
}

void		Channel::setKey(std::string &key)
{
	_key = key;
	modes['k'] = 1;
}

void		Channel::unsetKey()
{
	_key = "";
	modes['k'] = 0;
}

bool		Channel::checkKey(std::string &key, int &fd, std::map<int, Client> &_clients, std::string &chan)
{
	if (_key == "" || key == _key)
		return (1);
	else {
	std::string error = 
		"475 " + _clients[fd].getNickname() + " " + chan + " :Cannot join channel (Bad channel key)\r\n";
	send(fd , error.c_str(), error.size(), 0);
	return (0); }
}

bool		Channel::checkInvite(int &fd)
{
	std::vector<int>::iterator it = invites.begin();
	for ( ; it != invites.end(); ++it)
	{
		if (*it == fd)
		{
			it = invites.erase(it);
			return true;
		}
	}
	return false;
}

bool		Channel::checkLimit(int &fd, std::map<int, Client> &_clients, std::string &chan)
{
	if ((modes['l'] > 0) && (userList.size() >= (size_t)modes['l']))
	{
		if (checkInvite(fd))
			return (true);
		std::string error = 
		"471 " + _clients[fd].getNickname() + " " + chan + " :Cannot join channel (+l)\r\n";
		send(fd , error.c_str(), error.size(), 0);
		return (false);
	}
	else if (modes['i'] == 1 && !checkInvite(fd))
	{
		std::string error = 
		"473 " + _clients[fd].getNickname() + " " + chan + " :Cannot join channel (+i)\r\n";
		send(fd , error.c_str(), error.size(), 0);
		return (false);
	}
	return (true);
}

void		Channel::removeTopic(Client &client)
{
	std::vector<Client *>::iterator it = clients.begin();
	for ( ; it != clients.end(); ++it) {
		if ((*it)->getNickname() == client.getNickname()) {
			break; } }
	if (it == clients.end() || (modes['t'] == 1 && userList[(*it)->getFd()] == false))
		return ;
	isTopic = false;
	broadcastChannel(client, "TOPIC", "");
}

void		Channel::setTopic(Client &client, std::string &msg)
{
	if (modes['t'] == 1 && userList[client.getFd()] == false)
	{
		std::string error = "482 " + client.getNickname() + " " + getChannelName() + " :You're not channel operator\r\n";
		send(client.getFd(), error.c_str(), error.size(),0);
		return; }
	std::vector<Client *>::iterator it = clients.begin();
	for ( ; it != clients.end(); ++it) {
		if ((*it)->getNickname() == client.getNickname()) {
			break; } }
	if (it == clients.end() || (modes['t'] == 1 && userList[(*it)->getFd()] == false))
		return ;
	isTopic = true;
	broadcastChannel(client, "TOPIC", msg);
}

void		Channel::inviteMode(Client &client, std::vector<std::string> &arg)
{
	(void)client;
	if (arg[2] == "+i") {
		modes['i'] = 1;
		invites.clear(); 
		broadcastChannel(client, "MODE", "+i");
	}
	else
	{
		modes['i'] = 0;
		broadcastChannel(client, "MODE", "-i");
	}
}

void		Channel::topicMode(Client &client, std::vector<std::string> &arg)
{
	(void)client;
	if (arg[2] == "+t") {
		modes['t'] = 1;
		broadcastChannel(client, "MODE", "+t");}
	else {
		modes['t'] = 0;
		broadcastChannel(client, "MODE", "-t");}
}

void		Channel::keyMode(Client &client, std::vector<std::string> &arg)
{
	(void)client;
	if (arg[2] == "+k" && arg.size() == 4 && !arg[3].empty()) {
		setKey(arg[3]);
		broadcastChannel(client, "MODE", "+k " + arg[3] );}
	else if (arg[2] == "-k" && arg.size() == 4 && arg[3] == _key) {
		unsetKey();
		broadcastChannel(client, "MODE", "-k *"); }
}

void		Channel::opsMode(Client &client, std::vector<std::string> &arg)
{
	std::string target = arg[3];
	if (arg[2] == "+o" && arg.size() == 4 && client.getNickname() == target && userList[client.getFd()])
		return;
	std::vector<Client *>::iterator it = clients.begin();
	for ( ; it != clients.end(); it++)
	{
		if ((*it)->getNickname() == target)
			break ;
	}
	if (it == clients.end())
	{
		std::string error = "441 " + client.getNickname() + " " + target + " " + getChannelName() + " :They aren't on that channel\r\n";
		send(client.getFd(), error.c_str(), error.size(),0);
	return; }
	if (arg[2] == "+o")
		addOp(client, target);
	else
		removeOp(client, target);
}

void		Channel::limitMode(Client &client, std::vector<std::string> &arg)
{
	if (arg[2] == "+l" && arg.size() < 4) {
		std::string error = "461 " + client.getNickname() + " MODE :Not enough parameters\r\n";
		send(client.getFd(), error.c_str(), error.size(),0);
		return; }
	
	if (arg[2] == "-l"){
		modes['l'] = 0;
		broadcastChannel(client, "MODE", "-l");
		return ;
	}
	long	check;
	std::istringstream iss(arg[3]);
	iss >> check;
	if (arg[3].size() > 10 || check > INT_MAX || check < 1)
		return ;
	modes['l'] = (int)check;
	broadcastChannel(client, "MODE", "+ l " + arg[3]);
}

void		Channel::setMode(Client &client, std::vector<std::string> &arg)
{
	std::string cmd[] = {"+i", "-i", "+t", "-t", "+k", "-k", "+o", "-o", "+l", "-l"};
	void (Channel::*func[])(Client &client, std::vector<std::string> &arg) = {&Channel::inviteMode, &Channel::topicMode, &Channel::keyMode, &Channel::opsMode, &Channel::limitMode};
	for (int i = 0; i < 10; ++i)
	{
		if (arg[2] == cmd[i])
		{
			if (userList.find(client.getFd()) != userList.end())
			{
				if (userList[client.getFd()] == false)
				{
					std::string reply =  "482 " + client.getNickname() + " " + getChannelName() + " :You're not channel operator\r\n";
					send(client.getFd(), reply.c_str(), reply.size(), 0);
					return ;
				}
				(this->*func[i / 2])(client, arg);
				break;
			}
			else
			{
				std::string reply = "442 " + client.getNickname() + " " + getChannelName() + " :You're not on that channel\r\n";
				send(client.getFd(), reply.c_str(), reply.size(), 0);
			}
		}
	}
}

void Channel::printMode(int &fd, Client &clientClass)
{
	std::string client = clientClass.getNickname();
	std::string server = clientClass.getServername();
	
	// Get the limit if there is any
	std::ostringstream oss;
	oss << modes['l'];
	std::string limit = oss.str();
	
	std::string modeStr = "+";
	std::map<char, int>::iterator it = modes.begin();
	for ( ; it != modes.end(); ++it) {
		if (it->second)
			modeStr += it->first; }
	std::string reply = 
		":" + server + " 324 " + client + " " + getChannelName() + " " + modeStr +
		((modes['l'] != 0) ? (" " + limit) : "") + ((_key != "") ? (" " + _key) : "") + "\r\n";
	send(fd, reply.c_str(), reply.size(), 0);
}