#ifndef LogReader_H_
#define LogReader_H_
// -------------------------------------------------------------------------
#include <string>
#include <queue>
#include <cc++/socket.h>
#include "UTCPStream.h"
#include "DebugStream.h"
#include "LogServerTypes.h"
// -------------------------------------------------------------------------
class LogReader
{
    public:

        LogReader();
        ~LogReader();

        void readlogs( const std::string& addr, ost::tpport_t port, 
                       LogServerTypes::Command c = LogServerTypes::cmdNOP, 
                       int data = 0, 
                       const std::string& logname="", 
                       bool verbose = false );

        void readlogs( const std::string& addr, ost::tpport_t port, LogServerTypes::lsMessage& m, bool verbose = false );

        bool isConnection();

        inline void setCommandOnlyMode( bool s ){ cmdonly = s; }

    protected:

        void connect( const std::string& addr, ost::tpport_t port, timeout_t tout=TIMEOUT_INF );
        void connect( ost::InetAddress addr, ost::tpport_t port, timeout_t tout=TIMEOUT_INF );
        void disconnect();

    private:
        UTCPStream* tcp;
        std::string iaddr;
        ost::tpport_t port;
        bool cmdonly;

        DebugStream rlog;
};
// -------------------------------------------------------------------------
#endif // LogReader_H_
// -------------------------------------------------------------------------