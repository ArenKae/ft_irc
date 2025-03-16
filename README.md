# ft_irc

<p align="center">
  <img src="https://github.com/ArenKae/ArenKae/blob/main/42%20badges/ft_irce.png" alt="ft_irc 42 project badge"/>
</p>

## A small Internet Relay Chat (IRC) server made in C++.
Before Discord, Slack, or modern instant messaging, there was IRC — one of the earliest ways for people to communicate online in real time. Created in 1988, IRC was a revolutionary protocol that allowed users to join channels, chat with others, and even send private messages across the internet.

At its core, an IRC server acts as the backbone of this communication network. It manages connections, routes messages between users, and ensures that everyone in the right channel receives the right messages. Though IRC may seem outdated, its influence is still visible today in modern chat applications and online communities.

The ft_irc project is a lightweight C++ implementation of an IRC server, built from scratch to better understand how real-time messaging works. It allows us to better understand the concept of sockets that is at the foundation of online communication as we know it today.


<p align="center">
  <img src="https://github.com/ArenKae/ArenKae/blob/main/screens/IRC.PNG" alt="IRC demo">
</p>


## Status
Finished 01/07/2024.

Grade: 100/100

## Usage
### 💻 This program was developed on and for Ubuntu 24.04.2 LTS.

Clone this repository and ```cd``` into it. Use ```make``` to compile, then launch the server with :
```
./ircserv <port> <password>
```
### Connecting to the Server Using IRSSI
This project was developed for the [IRSSI](https://irssi.org/) client version 1.2.3. You can install it using :
```
sudo apt install irssi -y
```

After launching ```irssi``` in a separate terminal, run the following command:
```
/connect localhost <server_port> <server_password>
```

You can then join or create a channel to start chatting using :
```
/join <channel_name>
```

To leave the IRSSI client, use ```/quit``` or ```/exit```.

### Connecting to the Server Using NetCat

Alternatively, connect to the server using `nc` or `netcat` :
```
nc -C localhost <server_port>
```

Then, you can input your IRC commands. To properly register on the server, use the following commands in this order:

```
PASS <server_password>
NICK <nickname>
USER <username> <hostname(same as username)> <servername(localhost)> :<realname>
```

## Implemented Commands

- `CAP`: Partial implementation to inform the client about the server capabilities.
- `INVITE <nickname> <channel_name>`: Invites a user to a channel.
- `JOIN <channel_name>`: Joins an existing channel or creates one.
- `KICK <nickname> <channel_name>`: Kicks a user from a channel (this is a channel operator command)
- `LIST`: Lists all channels on the server.
- `MODE <channel_name> <mode(s)>`: Sets/removes channel modes. Implemented channel modes:
    - `i`: set/remove invite-only mode.
    - `k`: set/remove the channel key (password)
    - `l`: set/remove a channel's user limit
    - `o`: give/take channel operator privileges
    - `t`: set/remove the restrictions of the TOPIC command to channel operators
- `NICK <nickname>`: sets the user's nickname.
- `PART <channel_name> <message>`: removes user from channel(s) and notifies the other users in the channel.
- `PASS <password>`: Used for registration to the server. The client must supply the server password as part of the registration process.
- `PING`: Reply to ping requests by the client.
- `PRIVMSG <nickname_or_channel_name> <message>`: sends a message to the specified user or channel.
- `QUIT <message>`: Removes the user from the server and notifies other users in the channels the quitting user was in.
- `WHO <target>`: Returns informations about a user or a channel.
- `WHOIS <nickname>`: Returns informations about a particular user.
- `TOPIC <channel_name> <message>`: sets message as the channel topic.
- `USER <username> <hostname> <servername> :<realname>`: sets a new user's information during registration.

---

This project was made by acosi in collaboration with keschouf