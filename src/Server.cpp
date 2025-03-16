/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosi <acosi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/04 12:21:28 by keschouf          #+#    #+#             */
/*   Updated: 2025/01/06 23:53:18 by acosi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/utils.hpp"

// Constructor
Server::Server(int port, std::string password) : _port(port), _password(password) {}

// Destructor
Server::~Server() {}

// Initialize the static signal variable
bool	Server::_signal = false;


/*	Initialize the server's main socket, then enter a loop to handle
 *	client connections, and process incoming data using the `poll()` system call.
 *	This loop continues until a signal is sent to interupt the server. */
void	Server::serverInit()
{
	createSocket();
	while (_signal == false)
	{
		/* Use poll() to monitor events on all file descriptors in the
		 * _fds vector, through a pointer to the first element. */
		if (poll(&_fds[0],_fds.size(),-1) == -1 && _signal == false)
			throw (std::runtime_error("poll() failed"));
			
		// Iterate through all file descriptors
		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].revents & POLLIN) {
				// If and event happens on the server fd that listens for 
				// incoming connections, create a new socket for the client.
				if (_fds[i].fd == _servSocketFd)
					createClient();
				else
					handleData(i); }
		}
		// At the end of each loop, check if there are any empty channels to clean up.
		if (!_emptyChannels.empty())
			removeEmptyChannels();
	}
}

//	Create and configure the main server socket, making it listen for incoming connections.
void	Server::createSocket()
{
	// Create and fill a sockaddr_in structure with important server infos
	struct sockaddr_in add;
	add.sin_family = AF_INET; // Set the address family to ipv4
	add.sin_port = htons(_port); // Convert the port to the required format (network byte order)
	add.sin_addr.s_addr = INADDR_ANY; // Set the address to any local machine address

	/* Create the server socket
	 * AF_INET : ipv4 address family for the communication domain
	 * SOCK_STREAM : TCP socket type
	 * 0 : let the system choose the protocol */
	int sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFD == -1)
		throw (std::runtime_error("failed to create socket"));

	/* Set the SO_REUSEADDR option to the socket
	 * SOL_SOCKET : indicates that this is a socket-level option
	 * SO_REUSEADDR : tells the socket to immediately reuse the same address without delay
	 * &en : the function expects a pointer to the value that needs to be set for the option */
	int en = 1;
	if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw (std::runtime_error("failed to set option SO_REUSEADDR on socket"));
	
	/* fcnt() performs various control operations on file descriptors.
	 * Here it is used to set the socket fd in non-blocking mode, which
	 * means operations such as read() and write() return immediately. */
	if (fcntl(sockFD, F_SETFL, O_NONBLOCK) == -1)
		throw (std::runtime_error("failed to set option O_NONBLOCK on socket"));
	
	// Bind the socket to the address using the previously created structure
	if (bind(sockFD, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw (std::runtime_error("failed to bind socket"));
	
	/* Make the socket passive to listen for incoming connections.
	 * SOMAXCONN : The maximum length of the queue of pending connections. */
	if (listen(sockFD, SOMAXCONN) == -1)
		throw (std::runtime_error("listen() failed"));
	
	/* Create a pollfd structure with the socket in it, so that
	 * the poll() function can be used to monitor events on this 
	 * socket and accept new clients. */
	struct pollfd pollFD;
	pollFD.fd = sockFD;
	pollFD.events = POLLIN;
	pollFD.revents = 0;
	_fds.push_back(pollFD); // Add the structure to a vector holding all fd to monitor
	_servSocketFd = sockFD; // Store the server main socket fd for easy access
	std::cout << GREEN "Server socket successfully created\n" RESET
	"Server listening..."  << std::endl;
}

// Create the client class, client socket, and accept the connection.
void	Server::createClient()
{
	Client	client; // Instantiate the Client class
	struct sockaddr_in clientAdd; // Create a structure to store client infos
	socklen_t len = sizeof(clientAdd);

	// Accept the client and assign it a new fd 
	int	newFD = accept(_servSocketFd, (sockaddr *)&(clientAdd), &len);
	if (newFD == -1) {
		std::cerr << RED "Could not accept new client" RESET << std::endl;
		return ; }

	// Set the client fd to non-blocking mode as well
	if (fcntl(newFD, F_SETFL, O_NONBLOCK) == -1)
		throw (std::runtime_error("failed to set option O_NONBLOCK on socket"));
	
	// Create and fill a pollfd struct for the client
	struct pollfd pollFD;
	pollFD.fd = newFD;
	pollFD.events = POLLIN;
	pollFD.revents = 0;
	_fds.push_back(pollFD); // Add the struct to the vector for poll()
	
	// Store the client instance in map of all clients, accessed by fd
	client.setFD(newFD);
	_clients[newFD] = client;
	_clients[newFD].setFD(newFD);
	_clients[newFD].setIP(clientAdd);
	std::cout << GREEN "Client successfully added" RESET
	"\nIP: "<< _clients[newFD].getIp() << " | fd = " << _clients[newFD].getFd() << std::endl;
}

// Process incoming data from an existing client fd.
void	Server::handleData(size_t &i)
{
	int clientFD = _fds[i].fd;
	char buffer[1024];

	// Receive data to read from client fd and store it in the buffer.
	ssize_t bytesRead = recv(clientFD, buffer, sizeof(buffer) - 1, 0);

	// Error handling
	if (bytesRead == 0) {
		std::string error = "QUIT :client interrupted by signal SIGINT";
		QUIT(clientFD, error);
		return; }
	else if (bytesRead < 0) {
		std::cerr << YELLOW "Error reading from FD " << clientFD << RESET << std::endl;
		std::string error = "QUIT :unexpected error";
		QUIT(clientFD, error);
		return; }
	
	buffer[bytesRead] = '\0';
	std::string data(buffer);
	std::cout << "Message from client (fd" << clientFD << "): " BLUE << data << RESET;
	(data[data.size() - 1] == '\n') ? std::cout.flush() : std::cout << std::endl;
	
	// Add the received data to the client buffer, to account for partial commands 
	std::string &clientBuffer = _clients[clientFD].getBuffer();
	clientBuffer += data;
	
	// When a '\n' is found, extract the command line from the client buffer and process it
	size_t pos;
	while ((pos = clientBuffer.find('\n')) != std::string::npos) {
		std::string command = clientBuffer.substr(0, pos);
		clientBuffer.erase(0, pos + 1); // Remove processed command from buffer
		if (!command.empty() && command[command.size() - 1] == '\r')
        	command.erase(command.size() - 1); // Trim any trailing carriage return
		commandHandler(clientFD, command);
		
		// Protection against out-of-bound access in the while () if the client was removed by QUIT command
		if (_clients.find(clientFD) == _clients.end())
			return ; }
}

// Parse the whole command, checking for errors, and execute the corresponding function.
void Server::commandHandler(int &fd, std::string &msg)
{
	// Check for consecutives spaces
	if (!checkSpaces(msg))
		return;
	
	// Initialize a map of commands associated with a pointer to the corresponding function.
	std::map<std::string, commandFunction> cmd;
	cmd["CAP"] = &Server::CAP;
	cmd["PASS"] = &Server::PASS;
	cmd["NICK"] = &Server::NICK;
	cmd["USER"] = &Server::USER;
	cmd["WHOIS"] = &Server::WHOIS;
	cmd["WHO"] = &Server::WHO;
	cmd["MODE"] = &Server::MODE;
	cmd["PING"] = &Server::PING;
	cmd["QUIT"] = &Server::QUIT;
	cmd["PRIVMSG"] = &Server::PRIVMSG;
	cmd["JOIN"] = &Server::JOIN;
	cmd["PART"] = &Server::PART;
	cmd["TOPIC"] = &Server::TOPIC;
	cmd["KICK"] = &Server::KICK;
	cmd["INVITE"] = &Server::INVITE;

	// Ensure the client starts by sending a valid password, nickname, and user infos.
	bool resetCommand = false;
	if (!firstHandshake(fd, msg, resetCommand))
        return;

	size_t space_pos = msg.find(' ');
		if (space_pos != std::string::npos) {
			std::string tag = msg.substr(0, space_pos);
			std::map<std::string, commandFunction>::iterator it = cmd.find(tag);
			
			// If a match was found in the map, execute the corresponding command function
			if (it != cmd.end())
				(this->*it->second)(fd, msg);
			else 
				std::cout << YELLOW "Unkown command : \"" << msg << "\"" RESET << std::endl; }
			
		// No space found
		else
			std::cout << RED "Error: invalid command format : \"" << msg << "\"" RESET << std::endl;
}