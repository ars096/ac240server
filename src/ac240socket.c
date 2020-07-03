
#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>


extern volatile sig_atomic_t do_finalize;


long socket_init(long *sockfd, long port)
{
  struct sockaddr_in addr;
  long reuse = 1;
  
  // create new socket
  //------------------
  // PF_INET: IPv4, SOCK_STREAM: TCP
  printf("socket(PF_INET, SOCK_STREAM, 0)\n");
  if((*sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("\n ERROR: socket\n\n");
      return -1;
    }
  
  // set socket options
  //-------------------
  // SOL_SOCKET: Socket API level, SO_REUSEADDR: (1)Reuse Address, (0)Not
  printf("setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))\n");
  if((setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) < 0)
    {
      printf("\nERROR: setsockope\n\n");
      return -1;
    }
  
  // sockaddr config
  //----------------
  // memset: set 0 to addr. if not, error can occur in bind()
  memset(&addr, 0, sizeof(addr));
  // set address family: AF_INET (IPv4)
  addr.sin_family = AF_INET;
  // set 
  addr.sin_addr.s_addr = INADDR_ANY;
  // set using port. htons(): host to network shotr integer
  addr.sin_port = htons(port);

  // bind
  //----------------
  printf("bind(*sockfd, (struct sockaddr *)&addr, sizeof(addr))\n");
  if((bind(*sockfd, (struct sockaddr *)&addr, sizeof(addr))) < 0)
    {
      printf("\nERROR: bind\n\n");

    }

  // listen
  //----------------
  printf("listen(*sockfd, 1)\n");
  if((listen(*sockfd, 1)) < 0)
    {
      printf("\nERROR: listen\n\n");
      return -1;
    }

  return 0;
}


long socket_wait_connection(const long server_sockfd, long *client_sockfd)
{
  struct sockaddr_in client;
  int len = sizeof(client);

  fd_set fds;
  struct timeval tv;
  
  tv.tv_sec = 0;
  tv.tv_usec = 100000;
  
  FD_ZERO(&fds);
  FD_SET(server_sockfd, &fds);
  
  if(select(server_sockfd+1, &fds, NULL, NULL, &tv) == 0)
    {
      return -2;
    }
  
  if(FD_ISSET(server_sockfd, &fds)){}
  else
    {
      return -3;
    }

  if(do_finalize)
    {
      return -1;
    }
    
  if((*client_sockfd = accept(server_sockfd, (struct sockaddr *)&client, &len)) < 0)
    {
      printf("WARNING: accept() failure\n");
      return -1;
    }
  
  printf("accepted connection from %s:%ld\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
  
  return 0;
}


long socket_check_connection(long client)
{
  fd_set fds;
  struct timeval tv;
  char buf[1];
  int ret;
  
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&fds);
  FD_SET(client, &fds);
  
  ret = select(client+1, &fds, NULL, NULL, &tv);
  if(ret == 0)
    {
      return 0;
    }
  
  if(FD_ISSET(client, &fds))
    {
      ret = recv(client, buf, sizeof(buf), 0);
      if(ret <= 0)
	{
	  return -1;
	}
    }

  return 0;
}
