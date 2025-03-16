/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/04 12:26:40 by keschouf          #+#    #+#             */
/*   Updated: 2024/12/27 02:36:20 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"
#include "../include/utils.hpp"

Client::Client() : passFlag(false), nickFlag(false), userFlag(false) {}

Client::~Client() {}

/**********************************/
/************[ SETTERS ]***********/
/**********************************/

void	Client::setFD(int newFD) { _fd = newFD; }

void	Client::setIP(sockaddr_in &clientAdd) { _ip = (inet_ntoa(clientAdd.sin_addr)); }

void	Client::setNickname(std::string &str) { _nickname = str; }

void	Client::setUsername(std::string &str) { _username = str; }

void	Client::setHostname(std::string &str) { _hostname = str;}

void	Client::setRealname(std::string &str) { _realname = str; }

void	Client::setServername(std::string &str) { _servername = str; }

/**********************************/
/************[ GETTERS ]***********/
/**********************************/

std::string&	Client::getIp() { return _ip; }

int&			Client::getFd() { return _fd; }

std::string&	Client::getNickname() { return (_nickname); }

std::string&	Client::getUsername() { return (_username); }

std::string&	Client::getRealname() { return (_realname); }

std::string&	Client::getServername() { return (_servername); }

std::string&	Client::getHostname() { return (_hostname); }

std::string&	Client::getBuffer() { return (_buffer); }

/**********************************/
/************[ METHODS ]***********/
/**********************************/

void	Client::clearBuffer() { _buffer.clear(); }

