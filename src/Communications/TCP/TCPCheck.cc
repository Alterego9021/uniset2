// -----------------------------------------------------------------------------
#include <sstream>
#include <cstdlib>
#include "UniSetTypes.h"
#include "PassiveTimer.h"
#include "TCPCheck.h"
#include "UTCPStream.h"
// -----------------------------------------------------------------------------
using namespace std;
// -----------------------------------------------------------------------------
TCPCheck::TCPCheck():
	tout_msec(0)
{
}
// -----------------------------------------------------------------------------
TCPCheck::~TCPCheck()
{

}
// -----------------------------------------------------------------------------
bool TCPCheck::check( const std::string& _iaddr, timeout_t tout, timeout_t sleep_msec )
{
	auto v = UniSetTypes::explode_str(_iaddr, ':');

	if( v.size() < 2 )
		return false;

	return check( v[0], UniSetTypes::uni_atoi(v[1]), tout, sleep_msec );
}
// -----------------------------------------------------------------------------
bool TCPCheck::check( const std::string& _ip, int _port, timeout_t tout, timeout_t sleep_msec )
{
	ip = _ip;
	port = _port;
	tout_msec = tout;

	setResult(false);

	ThreadCreator<TCPCheck> t(this, &TCPCheck::check_thread);
	t.setCancel(ost::Thread::cancelDeferred);
	t.start();

	PassiveTimer pt(tout);

	while( !pt.checkTime() && t.isRunning() )
		msleep(sleep_msec);

	if( t.isRunning() ) // !getResult() )
		t.stop();

	return result;
}
// -----------------------------------------------------------------------------
void TCPCheck::check_thread()
{
	setResult(false);

	try
	{
		ost::Thread::setException(ost::Thread::throwException);
		UTCPStream t;
		t.create(ip, port, true, tout_msec);
		t.setKeepAliveParams( (tout_msec > 1000 ? tout_msec / 1000 : 1) );
		setResult(true);
		t.disconnect();
	}
	catch( ost::Exception& e ) {}
}
// -----------------------------------------------------------------------------
bool TCPCheck::ping( const std::string& _ip, timeout_t tout, timeout_t sleep_msec, const std::string& _ping_args )
{
	ip = _ip;
	tout_msec = tout;
	ping_args = _ping_args;

	setResult(false);

	ThreadCreator<TCPCheck> t(this, &TCPCheck::ping_thread);
	t.setCancel(ost::Thread::cancelDeferred);
	t.start();

	PassiveTimer pt(tout);

	while( !pt.checkTime() && t.isRunning() )
		msleep(sleep_msec);

	if( t.isRunning() ) // !getResult() )
		t.stop();

	return result;
}
// -----------------------------------------------------------------------------
void TCPCheck::ping_thread()
{
	setResult(false);

	ostringstream cmd;
	cmd << "ping " << ping_args << " " << ip << " 2>/dev/null 1>/dev/null";

	int ret = system(cmd.str().c_str());
	int res = WEXITSTATUS(ret);

	setResult((res == 0));
}
// -----------------------------------------------------------------------------
