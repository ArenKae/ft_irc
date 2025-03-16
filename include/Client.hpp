/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/04 12:26:44 by keschouf          #+#    #+#             */
/*   Updated: 2025/03/16 19:08:18 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>
#include <arpa/inet.h>	// inet_ntoa()

class Client
{
	public:
		Client();
		~Client();

		bool				passFlag;
		bool				nickFlag;
		bool				userFlag;
		std::map<std::string, bool>	chanList;

		void		setFD(int newFD);
		void		setIP(sockaddr_in &clientAdd);
		void		setNickname(std::string &str);
		void		setUsername(std::string &str);
		void		setHostname(std::string &str);
		void		setRealname(std::string &str);
		void		setServername(std::string &str);
		int&		getFd();
		std::string&	getIp();
		std::string&	getNickname();
		std::string&	getUsername();
		std::string&	getRealname();
		std::string&	getServername();
		std::string&	getHostname();
		std::string&	getBuffer();
		void		clearBuffer();

	private:
		int		_fd;
		std::string	_ip;
		std::string	_nickname;
		std::string	_username;
		std::string	_hostname;
		std::string	_realname;
		std::string	_servername;
		std::string	_buffer;
};