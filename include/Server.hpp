/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/04 12:21:14 by keschouf          #+#    #+#             */
/*   Updated: 2025/03/16 19:07:04 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Channel.hpp"
#include <sys/socket.h>		// for socket()
#include <netinet/in.h>		// sockaddr_in structure
#include <fcntl.h>		// fcntl()
#include <poll.h>		// poll()
#include <csignal>		// signal()
#include <arpa/inet.h>		// inet_ntoa()
#include <cstdlib>		// atoi()
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cctype>
#include <vector>
#include <map>

class Server
{
	public:
		// Constructor, destructor
		Server(int port, std::string password);
		~Server();
		
		// Public member functions
		void 		serverInit();
		static void signalHandler(int signum);
		void 		markForRemove(std::string name);

	private:
		// Attributes
		int					_port;
		std::string				_password;
		static bool				_signal;		// Used to stop the server when a signal is caught
		int					_servSocketFd;		// Store the server's main socket fd
		std::vector<pollfd>			_fds;			// Store all the pollfd struct monitored by poll()
		std::map<int, Client>			_clients;		// Store all the client instances, accessed by their fd
		std::map<std::string, Channel>		_channels;		// Store all the channel instances, accessed by their name
		std::vector<std::string>		_emptyChannels;		// Store channels with no clients left for later deletion

		// Private member functions
		bool firstHandshake(int &fd, std::string &msg, bool &resetCommand);
		void removeEmptyChannels(void);
		void broadcastServer(Client &client, std::string oldname, std::string type);
		void createSocket();
		void createClient();
		void handleData(size_t &i);
		void commands(int &i, std::string &message);
		void commandHandler(int &i, std::string &message);
		bool checkPassword(int &fd, std::string &msg, bool &resetCommand);
		bool checkUser(int &fd, std::string &msg, bool &resetCommand);
		bool checkNickname(int &fd, std::string &msg, bool &resetCommand);
		void sendMessages(int &fd, std::string &msg);
		void addChannel(std::string &name);
		bool checkChannels(std::string &arg);
		void joinChannels(int &fd, std::vector<std::string> &chans, std::vector<std::string> &keys);
		void setMode(int &fd, std::vector<std::string> &arg);

		// Commands functions
		void CAP(int &fd, std::string &msg);
		void PASS(int &fd, std::string &msg);
		void NICK(int &fd, std::string &msg);
		void USER(int &fd, std::string &msg);
		void WHOIS(int &fd, std::string &msg);
		void WHO(int &fd, std::string &msg);
		void MODE(int &fd, std::string &msg);
		void PING(int &fd, std::string &msg);
		void QUIT(int &fd, std::string &msg);
		void PRIVMSG(int &fd, std::string &msg);
		void JOIN(int &fd, std::string &msg);
		void PART(int &fd, std::string &msg);
		void TOPIC(int &fd, std::string &msg);
		void KICK(int &fd, std::string &msg);
		void INVITE(int &fd, std::string &msg);

	// Alias to improve command map readability
	typedef void (Server::*commandFunction)(int &, std::string &);
};