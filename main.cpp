#include <iostream>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>


void pack_data(std::vector<char> &in)
{
	in.insert(in.begin(), in.size());
}

void pack_dataFin(std::vector<char> &in)
{
	pack_data(in);
}

std::vector<char> hand_shake(uint16_t port, std::string ip)
{
	std::vector<char> ret;
	ret.push_back(0);
	ret.push_back(0);
	std::vector<char> tmp(ip.begin(),ip.end());
	pack_data(tmp);
	std::copy(tmp.begin(), tmp.end(), std::back_inserter(ret));
	ret.push_back(static_cast<char>(port&0xff));
	ret.push_back(static_cast<char>(port>>8));
	ret.push_back(1);
	pack_dataFin(ret);
 	return ret;
}

int main(int argc, char *argv[])
{
	int port = 25565, sockfd = 0;
	std::string ip;
	struct addrinfo hints = {0}, *res, *tmp;
	hints.ai_family = AF_INET;

	if(argc >= 2 ){
		ip = argv[1];
		if(argc >= 3){
			port = atol(argv[2]);
			if(!port || port > 0xffff){
				std::cout << "invalid port" << std::endl;
				return 1;
			}
		}
	} else {
		std::cout << "invalid arg... ip port" << std::endl;
		return 1;
	}

  	if(getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &res) != 0)
	{
		std::cout << "Invalid ip"  << std::endl;
		return 1;
	}

	tmp = res;
	while(1)
	{
		if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			std::cout << "Could not create socket"  << std::endl;
			return 1;
		}

		if(connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
			close(sockfd);
		else
			break;

		res = res->ai_next;

		if(res == NULL){
			std::cout << "Could not Connect" << std::endl;
			freeaddrinfo(tmp);
			return 1;
		}
	}
	freeaddrinfo(tmp);

	auto hshake = hand_shake(port,ip);
	write(sockfd, reinterpret_cast<const char*>(hshake.data()), hshake.size());
	write(sockfd,(const char[]){1,0}, 2);

	close(sockfd);
	return 0;
}