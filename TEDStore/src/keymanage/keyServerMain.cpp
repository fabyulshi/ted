#include "keyServer.hpp"
#include "ssl.hpp"

Configure config("config.json");

#if SINGLE_THREAD_KEY_MANAGER == 1
int main()
{
    ssl* keySecurityChannelTemp = new ssl(config.getKeyServerIP(), config.getKeyServerPort(), SERVERSIDE);
    boost::thread* th;
    keyServer* server = new keyServer(keySecurityChannelTemp);
    while (true) {
        SSL* sslConnection = keySecurityChannelTemp->sslListen().second;
        th = new boost::thread(boost::bind(&keyServer::runKeyGen, server, sslConnection));
        th->detach();
    }
    return 0;
}
#else
int main()
{
    ssl* keySecurityChannelTemp = new ssl(config.getKeyServerIP(), config.getKeyServerPort(), SERVERSIDE);
    boost::thread* th;
    keyServer* server = new keyServer(keySecurityChannelTemp);
    th = new boost::thread(boost::bind(&keyServer::runOptimalSolver, server));
    while (true) {
        SSL* sslConnection = keySecurityChannelTemp->sslListen().second;
        if (OLD_VERSION){
            th = new boost::thread(boost::bind(&keyServer::runKeyGen, server, sslConnection));
        } else {
            if (ENABLE_SECRET_SHARE) {
                th = new boost::thread(boost::bind(&keyServer::runKeyGenSS, server, sslConnection));
            } else {
                th = new boost::thread(boost::bind(&keyServer::runKeyGenSimple, server, sslConnection));
            }    
        }
        th->detach();
    }
    return 0;
}
#endif
