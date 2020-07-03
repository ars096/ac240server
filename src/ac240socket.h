
long socket_init(long *sockfd, long port);
long socket_wait_connection(const long server_sockfd, long *client_sockfd);
long socket_check_connection(long client);
