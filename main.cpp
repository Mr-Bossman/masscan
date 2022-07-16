#include <iostream>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

unsigned char* hand_shake(uint16_t port, const char* ip,size_t ip_len)
{
	size_t tlen = 7+ip_len;
	unsigned char * ret = (unsigned char *)calloc(1,tlen);
	ret[0] = 6+ip_len;
	ret[3] = ip_len;
	memcpy(ret+4,ip,ip_len);
	ret[tlen-3] = (unsigned char)(port&0xff);
	ret[tlen-2] = (unsigned char)(port>>8);
	ret[tlen-1] = 1;
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

	auto hshake = hand_shake(port,ip.c_str(),ip.length());
	write(sockfd, hshake, hshake[0]+1);
	write(sockfd,(const char[]){1,0}, 2);

	close(sockfd);
	return 0;
}
