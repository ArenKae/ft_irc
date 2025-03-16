/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/11 15:24:58 by keschouf          #+#    #+#             */
/*   Updated: 2025/03/16 19:09:05 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <limits.h>
#include <ctime>
#include "Client.hpp"

class Server;

class Channel
{
private:
	std::string	_name;
	std::string	_key;
	Server*		_server;
	std::time_t	_creationTime;

public:
	Channel();
	Channel(std::string &name, Server* server);
	~Channel();
	std::string	getChannelName();
	bool		addClient(Client *client);
	void		addOp(Client &client, std::string &target);
	void		removeOp(Client &client, std::string &target);
	void		broadcastChannel(Client &client, std::string type, std::string msg);
	void		getClientList(int &fd, Client &client);
	void		removeClient(std::string &name);
	bool		checkClient(std::string &name);
	void		setKey(std::string &key);
	void		unsetKey();
	bool		checkKey(std::string &key, int &fd, std::map<int, Client> &_clients, std::string &chan);
	bool		checkInvite(int &fd);
	bool		checkLimit(int &fd, std::map<int, Client> &_clients, std::string &chan);
	void		populateUserList(Client &client, bool opFlag);
	void		removeTopic(Client &client);
	void		setTopic(Client &client, std::string &msg);
	void		inviteMode(Client &client, std::vector<std::string> &arg);
	void		topicMode(Client &client, std::vector<std::string> &arg);
	void		keyMode(Client &client, std::vector<std::string> &arg);
	void		opsMode(Client &client, std::vector<std::string> &arg);
	void		limitMode(Client &client, std::vector<std::string> &arg);
	void		setMode(Client &client, std::vector<std::string> &arg);
	void		printMode(int &fd, Client &client);

	std::vector<Client *>	clients;
	std::map<int, bool>	userList;
	std::map<char, int>	modes;
	std::vector<int>	invites;
	std::string		topic;
	bool			isTopic;
};