# C-Network-Cloud

This is a small and simple cloud storage system designed to store single files on a remote server within the same network. 

## Table of contents
---
* [Overview](#overview)
* [Features](#features)
* [Requirements](#requirements)
* [Installation](#installation)

## Overview
---
The aim of this project is to provide an easy and fast way to store files on a server in the network. Therefore the server must run on a device which can be addressed by the client. 
  
The remote server saves files in its local file system. Therefore the data integrity is dependent on the device the server is running on.

The program was developed on [Ubuntu 20.04 LTS](https://ubuntu.com/download/desktop) and compiled with [gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0](https://gcc.gnu.org/).  

For Windows users [WSL 2](https://docs.microsoft.com/en-us/windows/wsl/install-win10) is recommended with [Ubuntu 20.04 LTS](https://www.microsoft.com/en-us/p/ubuntu-2004-lts/9n6svws3rx71?activetab=pivot:overviewtab).


## Features
---
* File storage on a remote system
* Password based authentication
* Remote access via TCP scokets
* Easy terminal use

## Requirements
* [gcc](https://gcc.gnu.org/)
* [OpenSSL](https://github.com/openssl/openssl)

## Installation