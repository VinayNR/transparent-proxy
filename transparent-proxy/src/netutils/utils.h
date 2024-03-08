int getServerInfo(const char *, const char *, struct addrinfo *&);
void getHostAndPort(const char *, char *&, char *&);
int setupListeningSocket(int, int);
int connectToServer(const char *, const char *);