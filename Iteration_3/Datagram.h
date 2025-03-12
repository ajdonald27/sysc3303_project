// Datagram.h
#pragma once

#include <vector>
#include <exception>
#include <cstring>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>  // for close()

class InetAddress
{
public:
    static in_addr_t getLocalHost() {
        return inet_addr("127.0.0.1"); 
    }
};

class DatagramPacket {
public:
    DatagramPacket(std::vector<uint8_t> &data,
                   size_t length,
                   in_addr_t address = INADDR_ANY,
                   in_port_t port = 0)
        : _data(data)
    {
        _address.sin_family = AF_INET;
        _address.sin_port = port;
        _address.sin_addr.s_addr = address;
        _length = (length < data.size()) ? length : data.size();
    }

    void *getData() const {
        return const_cast<void*>(static_cast<const void*>(_data.data()));
    }

    size_t getLength() const { return _length; }
    void   setLength(size_t length) {
        _length = (length < _data.size()) ? length : _data.size();
    }

    in_addr_t getAddress() const { return _address.sin_addr.s_addr; }
    in_port_t getPort()    const { return ntohs(_address.sin_port); }

    struct sockaddr* address() {
        return reinterpret_cast<struct sockaddr*>(&_address);
    }

private:
    std::vector<uint8_t> &_data;
    size_t               _length;
    struct sockaddr_in   _address;
};

class DatagramSocket {
public:
    DatagramSocket()
        : socket_fd(socket(AF_INET, SOCK_DGRAM, 0))
    {
        if (socket_fd < 0) {
            throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));
        }
    }

    DatagramSocket(in_port_t port)
        : socket_fd(socket(AF_INET, SOCK_DGRAM, 0))
    {
        if (socket_fd < 0) {
            throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));
        }
        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family      = AF_INET;
        addr.sin_port        = port;
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));
        }
    }

    ~DatagramSocket() {
        close(socket_fd);
    }

    ssize_t send(DatagramPacket &packet) {
        socklen_t len = sizeof(*packet.address());
        ssize_t bytesSent = sendto(socket_fd,
                                   packet.getData(),
                                   packet.getLength(),
                                   0,
                                   packet.address(),
                                   len);
        if (bytesSent < 0) {
            throw std::runtime_error("sendto() failed: " + std::string(strerror(errno)));
        }
        return bytesSent;
    }

    void receive(DatagramPacket &packet) {
        socklen_t len = sizeof(*packet.address());
        int bytesRead = recvfrom(socket_fd,
                                 packet.getData(),
                                 MAXLINE,
                                 MSG_WAITALL,
                                 packet.address(),
                                 &len);
        if (bytesRead < 0) {
            throw std::runtime_error("recvfrom() failed: " + std::string(strerror(errno)));
        }
        packet.setLength(bytesRead);
    }

private:
    int socket_fd;
    static const size_t MAXLINE = 1024;
};
