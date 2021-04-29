#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "utils/utils.h"

/**
@notitle

Net is the networking part of vtoolbox.
To compile this folder, you might need to link against ssl and crypto (-lssl, -lcrypto),
as those are required for some protocols like HTTPS for example.

Example of stuff that you can do with net:

@code
std::string ip = "192.168.0.2";
bool reachable = ping(ip);
debug("Is IP %s reachable : %b",ip.c_str(),reachable);
@endcode
 */


// #pragma comment(lib, "iphlpapi.lib")
// #pragma comment(lib, "ws2_32.lib")

// note that ping can be also used to send data bypassing firewall rules.
// this can be used to exfiltrate sensitive data from a network.
bool ping(std::string ip,std::string data){

	HANDLE hIcmpFile;
	unsigned long ipaddr = INADDR_NONE;
	DWORD dwRetVal = 0;
	LPVOID ReplyBuffer = NULL;
	DWORD ReplySize = 0;
	

	if(data.size() == 0){
		// generate 32 bytes of random data.
		for(int i = 0;i < 32;i++){
			data += (char) rand();
		}
	}

	ipaddr = inet_addr(ip.c_str());
	if (ipaddr == INADDR_NONE) {
		debug("bad ip");
		return false;
	}

	hIcmpFile = IcmpCreateFile();
	ReplySize = sizeof(ICMP_ECHO_REPLY) + data.size();
	ReplyBuffer = (VOID*) malloc(ReplySize);
	char * dataPtr =  (char*)data.c_str();

	dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, dataPtr, data.size(), NULL, ReplyBuffer, ReplySize, 1000);

	if (dwRetVal != 0) {
		// PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
		// struct in_addr ReplyAddr;
		// ReplyAddr.S_un.S_addr = pEchoReply->Address;
		// pEchoReply->Status, pEchoReply->RoundTripTime, inet_ntoa( ReplyAddr )
		return true;
	}else{
		return false;
	}
}

int main(){
//	cout << ping("192.168.5.8","") << endl;
//	cout << ping("10.50.5.3","") << endl;
//	cout << ping("172.17.193.81","") << endl;

	return 0;
}
