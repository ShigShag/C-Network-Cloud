# C-Network-Cloud

This is a small and simple cloud storage system designed to store single files on a remote server within the same network. 

## Table of contents

* [Overview](#overview)
* [Features](#features)
* [Requirements](#requirements)
* [Installation](#installation)

## Overview

The aim of this project is to provide an easy and fast way to store files on a server in the network. Therefore the server must run on a device which can be addressed by the client. 
  
The remote server saves files in its local file system. Because of that the data integrity is dependent on the device the server is running on.

The program was developed on [Ubuntu 20.04 LTS](https://ubuntu.com/download/desktop) and compiled with [gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0](https://gcc.gnu.org/).  

For Windows users [WSL 2](https://docs.microsoft.com/en-us/windows/wsl/install-win10) is recommended with [Ubuntu 20.04 LTS](https://www.microsoft.com/en-us/p/ubuntu-2004-lts/9n6svws3rx71?activetab=pivot:overviewtab).

## Features

* File storage on a remote system
* Password based authentication
* Remote access via TCP scokets
* Serverside logging
* Easy terminal use

## Requirements

* [gcc](https://gcc.gnu.org/)
* [OpenSSL](https://github.com/openssl/openssl)

## Installation

### Server

The server requires a **configuration file** to start. The path to this file needs to be passed as the first argument to the server. 

Example configuration file:
```
port 8080
receive_timeout 0
max_clients 500
cloud_directory cloud/
client_database_path config/client_database.txt
client_credentials_path config/credentials.txt
server_log_path Log/server.log
client_log_directory Log/
```

Example server start:

```bash
./server yourconfigfile
```

---
Keyword description:

| Keyword      | Description| Required |
|--------------|------------|----------|
| port | The port the srever should listen for clients| yes|
| receive_timeout      | The time for the server to wait for an answer from the client before removing it. 0 -> infinite   | No |
| max_clients | Maximum number of clients for the server to accept. If not given defaults to 10| No |
| cloud_directory | Path to the directory which holds all the clients directories | yes|
| client_database_path | Path of the file which holds client id`s and the directory paths for the clients | yes |
| client_credentials_path | Path of the file which holds clients passwords | yes |
| server_log_path | Path to the logging file for the server | no|
|client_log_directory| Path to the directory which holds the clients logs | no| 

### Client