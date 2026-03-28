<h1 align="center">IRC_42</h1>

<p align="center">
  A small multi-client IRC server built in C++98 with sockets and <code>poll()</code>.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/School-42-black?style=for-the-badge" alt="42 badge" />
  <img src="https://img.shields.io/badge/Language-C%2B%2B98-blue?style=for-the-badge" alt="Language badge" />
  <img src="https://img.shields.io/badge/Status-In%20Progress-yellow?style=for-the-badge" alt="Status badge" />
</p>

<p align="center">
  <strong>IRC • sockets • poll() • channels • C++98</strong>
</p>

---

## 📌 Overview

`IRC_42` is a project developed as part of the 42 curriculum.

The repository currently contains `ft_irc`, an IRC server that accepts multiple TCP clients, authenticates them with `PASS`, `NICK`, and `USER`, and implements a practical subset of IRC commands for messaging, channels, and operator actions.

This project focuses on:
- socket programming and TCP server lifecycle
- event-driven I/O with `poll()`
- IRC command parsing and reply handling
- channel and client state management

---

## ✨ Features

- ✅ multi-client TCP server loop based on `poll()`
- ✅ registration flow with `PASS`, `NICK`, and `USER`
- ✅ channel creation, join/leave flow, and channel listing with `JOIN`, `PART`, and `LIST`
- ✅ private and channel messaging with `PRIVMSG`
- ✅ operator-oriented commands and modes with `KICK`, `INVITE`, `TOPIC`, `MODE`, and `PING`
- ⚠️ automated command coverage is still incomplete: `ft_irc/tests/test_commands.sh` is currently empty

---

## 🧠 Concepts Covered

Through this project, I worked on the following concepts:

- TCP sockets with `socket()`, `bind()`, `listen()`, `accept()`, `send()`, and `recv()`
- multiplexed I/O with `poll()`
- C++98 class design and separation of responsibilities
- command parsing with `std::istringstream`
- incremental network-buffer handling for IRC messages
- manual resource cleanup for sockets, clients, and channels

---

## 🛠️ Build

Clone the repository and compile the project:

```bash
git clone git@github.com:Middle-555/IRC_42.git
cd IRC_42/ft_irc
make
```

Available Makefile rules:

```bash
make
make clean
make fclean
make re
```

Additional build notes:
- the Makefile is located in `ft_irc/`
- the current executable name is `ircserv`
- the current compile flags are `-Wall -Wextra -Werror -std=c++98 -g`

---

## 🚀 Usage

Run the program with:

```bash
./ircserv <port> <password>
```

### Examples

```bash
./ircserv 6667 mypassword
nc 127.0.0.1 6667
echo -e "PASS mypassword\nNICK user1\nUSER u1 0 * :User One\nJOIN #test\n" | nc 127.0.0.1 6667
```

Usage notes:
- `main.cpp` currently validates ports only in the `[1024, 65535]` range
- the repository also includes manual test scenarios in `documentation/testcommand.txt`
- some older helper scripts still refer to `./irc`; the current Makefile builds `./ircserv`

---

## 📂 Project Structure

```text
.
├── ft_irc/
│   ├── Makefile
│   ├── include/
│   │   ├── Channel.hpp
│   │   ├── Client.hpp
│   │   ├── CommandHandler.hpp
│   │   └── Server.hpp
│   ├── src/
│   │   ├── Channel.cpp
│   │   ├── Client.cpp
│   │   ├── CommandHandler.cpp
│   │   ├── Server.cpp
│   │   └── main.cpp
│   └── tests/
│       ├── killIRC.sh
│       ├── test_commands.sh
│       └── test_connection.sh
├── documentation/
│   ├── IRC ref Protocole.pdf
│   ├── Roadmap.txt
│   ├── fr.subject.pdf
│   └── testcommand.txt
├── template.md
└── README.md
```

### Structure Details

- `ft_irc/include/` : class declarations for the server, clients, channels, and command dispatcher
- `ft_irc/src/` : program entry point, server loop, command handling, and channel/client behavior
- `ft_irc/tests/` : helper scripts for build, connection smoke tests, and process cleanup
- `documentation/` : roadmap, subject files, protocol references, and manual command examples

---

## ⚙️ Project Constraints

This project follows the constraints visible in the repository and in the 42 `ft_irc` context:
- written in `C++98`
- compiled with `-Wall -Wextra -Werror`
- centered on low-level socket APIs and `poll()`
- includes explicit cleanup of allocated clients and channels
- implements a subset of IRC behavior around authentication, channels, messaging, and operator actions

The repository also contains the official subject PDF in `documentation/fr.subject.pdf`.

---

## 🧪 Testing

The repository currently provides or documents the following testing approaches:
- build and smoke testing with `ft_irc/tests/test_connection.sh`
- manual connection and registration checks with `nc`
- multi-client channel scenarios documented in `documentation/testcommand.txt`
- operator and channel-mode scenarios with `KICK`, `INVITE`, `TOPIC`, and `MODE`
- memory checking guidance with `valgrind`

### Manual test examples

```bash
cd ft_irc && ./ircserv 6667 mypassword
nc 127.0.0.1 6667
PASS mypassword
NICK user1
USER u1 0 * :User One
JOIN #test
```

### Memory checks

```bash
valgrind --leak-check=full --show-leak-kinds=all ./ircserv 6667 mypassword
```

Testing notes:
- `ft_irc/tests/test_commands.sh` is empty for now
- `ft_irc/tests/test_connection.sh` still checks for `./irc`, so it should be updated if you want the script to match the current Makefile output

---

## 📖 What I Learned

This project helped me improve in the following areas:
- designing an event-driven server around `poll()`
- structuring IRC command parsing into dedicated classes and handlers
- managing shared state between clients, channels, and operator permissions
- handling cleanup paths for disconnects, shutdown, and allocated resources

---

## 🚧 Possible Improvements

Although the project already has a working base, several improvements could be considered:
- complete the automated test suite in `ft_irc/tests/`
- align helper scripts and documentation around the current executable name `ircserv`
- improve RFC-level reply consistency and error coverage
- model multiple joined channels per client instead of a single `currentChannel`

---

## 👤 Authors

- Kevin Pourcel (`Middle`) - 42 student - [GitHub](https://github.com/Middle-555)
- Adrien Cabarbaye - 42 student
