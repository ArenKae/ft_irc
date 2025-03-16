/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 09:19:36 by acosi             #+#    #+#             */
/*   Updated: 2025/01/07 09:39:14 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/utils.hpp"

void signals(void)
{
	signal(SIGINT, Server::signalHandler);
	signal(SIGQUIT, Server::signalHandler);
}

// Pre-parsing the command to detect consecutive spaces
bool checkSpaces(std::string &msg)
{
	for (size_t i = 0; i < msg.size(); ++i) {
		if (std::isspace(msg[i]) && std::isspace(msg[i + 1])) {
				std::cerr << RED "Error: malformed command." RESET << std::endl;
				return false;} }
	return true;
}

// Parse the JOIN command, ensuring the following format: JOIN #channel1,#channel2 key1,key2
bool joinParsing(std::string &msg, std::vector<std::string>* chanList, std::vector<std::string>* keyList)
{
	// Skip the "JOIN " part
	std::string tmp = msg.substr(5);

	// Find the split point where channels and keys are separated
	size_t pos = tmp.find(' ');
	std::string chan;
	std::string keys;

	if ((tmp.size() == 0 &&  pos == std::string::npos)) {
		std::cerr << RED "Error: invalid channel name" RESET << std::endl;
		return false; }

	if (pos != std::string::npos) {
		// If there's a space, separate channels and keys
		chan = tmp.substr(0, pos);
		keys = tmp.substr(pos + 1); }
	else
		// No space means no keys, just channels
		chan = tmp;
	// Split the channels by commas
	pos = 0;
	while ((pos = chan.find(',')) != std::string::npos) {
		std::string channel = chan.substr(0, pos);
		if (channel[0] != '#' && channel[0] != '&' && !std::isprint(channel[1])) {
			std::cerr << RED "Error: Channel must start with # or &." RESET << std::endl;
			return false; }
		(*chanList).push_back(channel);
		chan.erase(0, pos + 1);  // Remove processed part
	}
	// Add last channel
	if (!chan.empty()) {
		if ((chan[0] != '#' || chan[0] != '&' ) && !std::isprint(chan[1])) {
			std::cerr << RED "Error: Channel must start with # or &." RESET << std::endl;
			return false; }
		(*chanList).push_back(chan); }

	// Split the keys by commas, but only if keys are provided
	pos = 0;
	while ((pos = keys.find(',')) != std::string::npos) {
		(*keyList).push_back(keys.substr(0, pos));
		keys.erase(0, pos + 1); } // Remove processed part
	// Add last key, if any
	if (!keys.empty())
		(*keyList).push_back(keys);

	// If there are more channels than keys, assume empty keys for the extra channels
	while ((*keyList).size() < (*chanList).size())
		(*keyList).push_back("");  // No key for this channel
	return true;
}