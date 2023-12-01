// WARNING: if you know how to code, please dont check this code
// you will have a heart attack

#include "base/system.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

#include "ddts.h"
#include <protos/request.pb.h>

const int PORT = 42069;
const std::string HOST = "127.0.0.1";

const std::string SHM_NAME = "/ddts2";
const size_t SHM_SIZE = 10;

DDTS::DDTS(unsigned int happeningId) :
	happeningId(happeningId)
{
	fd = shm_open(SHM_NAME.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR);

	ftruncate(fd, SHM_SIZE);

	if(fd == -1)
	{
		// not gonna lie, we are fucked
	}

	sharedMemory = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(sharedMemory == MAP_FAILED)
	{
		// not gonna lie, we are fucked
	}
}

bool DDTS::CheckShutdownSignal()
{
	char *mem = static_cast<char *>(sharedMemory);

	if(mem[happeningId] == 1)
	{
		return true;
	}

	return false;
}

void DDTS::Shutdown()
{
	sockaddr_in serv_addr;
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	inet_pton(AF_INET, HOST.data(), &serv_addr.sin_addr);

	(void)connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	Request request;
	request.set_action(::Action::SHUTDOWN);
	request.set_id(happeningId);

	const size_t size = request.ByteSizeLong();
	char *bytes = new char[size];

	request.SerializeToArray(bytes, size);

	send(sock, bytes, size, 0);

	std::vector<char> gottem(1024);

	munmap(sharedMemory, SHM_SIZE);
	close(fd);

	// wait for the response owo. works for now
	recv(sock, gottem.data(), gottem.size(), 0);
}