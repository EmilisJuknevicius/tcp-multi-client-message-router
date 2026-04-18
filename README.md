# TCP Multi-Client Message Routing System (C / Winsock)

## Overview
This is a TCP-based message routing system written in C using Winsock. It consists of a server and 4 clients that communicate over IPv6 loopback. Clients can send messages to specific other clients using a simple custom protocol.

## How It Works
- Each client connects to a dedicated port (20001–20004)
- Clients send messages in the format:
  @<client_numbers> <message>
- Example:
  @13 Hello clients 1 and 3

- The server routes messages only to the specified clients
- The server logs all message routing activity

## Features
- Multi-client TCP communication
- Message routing between selected clients
- IPv6 loopback support
- `select()` based I/O handling
- Basic message validation and logging

## Requirements
- Windows OS
- C compiler (MinGW or MSVC)
- Winsock2 (ws2_32.lib)

## Build
gcc client.c -o client -lws2_32  
gcc server.c -o server -lws2_32  

## Run
Start server first:
server.exe  

Start clients:
client.exe 1  
client.exe 2  
client.exe 3  
client.exe 4  

## Notes
- Uses plain TCP (no encryption)
- Educational project for learning socket programming and message routing
