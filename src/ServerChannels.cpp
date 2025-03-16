/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerChannels.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/22 22:51:51 by acosi             #+#    #+#             */
/*   Updated: 2025/01/07 12:37:47 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/utils.hpp"

void	Server::joinChannels(int &fd, std::vector<std::string> &chans, std::vector<std::string> &keys)
{
	int i = 0;
	std::vector<std::string>::iterator it = chans.begin();
	for (; it != chans.end(); ++it, ++i)
	{
		if (checkChannels(*it))
		{
			if (!_channels[*it].checkKey(keys[i], fd, _clients, *it))
				continue;
			if (!_channels[*it].checkLimit(fd, _clients, *it))
				continue;
			else {
				_channels[*it].populateUserList(_clients[fd], false);
				_channels[*it].addClient(&(_clients[fd]));
				_channels[*it].broadcastChannel(_clients[fd], "JOIN", "");
				_channels[*it].getClientList(fd, _clients[fd]); 
				continue; }
		}
		addChannel(*it);
		_channels[*it].populateUserList(_clients[fd], true);
		_channels[*it].addClient(&(_clients[fd]));
		_channels[*it].addOp(_clients[fd], _clients[fd].getServername());
		_channels[*it].broadcastChannel(_clients[fd], "JOIN", "");
		_channels[*it].getClientList(fd, _clients[fd]);
	}
}

// Check if the channel already exists
bool	Server::checkChannels(std::string &arg)
{
	std::map<std::string, Channel>::iterator it = _channels.find(arg);
	if (it != _channels.end())
		return (true);
	return (false);
}

void	Server::addChannel(std::string &name)
{
	_channels[name] = Channel(name, this);
}

void		Server::markForRemove(std::string name)
{
	// Avoid adding the channel multiple times
	std::vector<std::string>::iterator it = _emptyChannels.begin();
	for ( ; it != _emptyChannels.end(); ++it) {
		std::cout << *it << std::endl;
		if (*it == name)
			return ;}
	_emptyChannels.push_back(name);
}

// Remove channels with no clients on it from the server to free up memory.
void Server::removeEmptyChannels(void)
{
	// Iterate through the vector holding all empty channels that were marked for deletion
	std::vector<std::string>::iterator it = _emptyChannels.begin();
    while (it != _emptyChannels.end())
	{
		// Find the channel to delete in the map holding all server channels
        std::map<std::string, Channel>::iterator itm = _channels.find(*it);
        if (itm != _channels.end())
            _channels.erase(itm); // Remove the channel from the map
		
		// Erase the element from the vector and update the iterator.
		// This method is used to prevent errors from erasing elements while iterating the container.
        it = _emptyChannels.erase(it);
    }
}