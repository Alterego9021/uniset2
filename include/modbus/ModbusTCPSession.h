// -------------------------------------------------------------------------
#ifndef ModbusTCPSession_H_
#define ModbusTCPSession_H_
// -------------------------------------------------------------------------
#include <string>
#include <queue>
#include <unordered_map>
#include <cc++/socket.h>
#include "ModbusServerSlot.h"
#include "ModbusServer.h"
#include "PassiveTimer.h"
// -------------------------------------------------------------------------
class ModbusTCPSession:
	public ModbusServerSlot,
	public ModbusServer,
	public ost::TCPSession
{
	public:

		ModbusTCPSession( ost::TCPSocket& server, const std::unordered_set<ModbusRTU::ModbusAddr>& vmbaddr, timeout_t timeout );
		virtual ~ModbusTCPSession();

		void cleanInputStream();

		virtual void cleanupChannel()
		{
			cleanInputStream();
		}
		virtual void terminate();

		virtual ModbusRTU::mbErrCode receive( const std::unordered_set<ModbusRTU::ModbusAddr>& vmbaddr, timeout_t msecTimeout );

		typedef sigc::slot<void, ModbusTCPSession*> FinalSlot;

		void connectFinalSession( FinalSlot sl );

		unsigned int getAskCount();

		inline std::string getClientAddress()
		{
			return caddr;
		}

		void setKeepAliveParams( timeout_t timeout_sec = 3, int conn_keepcnt = 2, int keepintvl = 2 );

	protected:
		virtual void run();
		virtual void final();

		virtual int getNextData( unsigned char* buf, int len );
		virtual void setChannelTimeout( timeout_t msec );
		virtual ModbusRTU::mbErrCode sendData( unsigned char* buf, int len );
		virtual ModbusRTU::mbErrCode tcp_processing( ost::TCPStream& tcp, ModbusTCP::MBAPHeader& mhead );
		virtual ModbusRTU::mbErrCode pre_send_request( ModbusRTU::ModbusMessage& request );
		virtual ModbusRTU::mbErrCode post_send_request( ModbusRTU::ModbusMessage& request );


		virtual ModbusRTU::mbErrCode readCoilStatus( ModbusRTU::ReadCoilMessage& query,
				ModbusRTU::ReadCoilRetMessage& reply );

		virtual ModbusRTU::mbErrCode readInputStatus( ModbusRTU::ReadInputStatusMessage& query,
				ModbusRTU::ReadInputStatusRetMessage& reply );

		virtual ModbusRTU::mbErrCode readOutputRegisters( ModbusRTU::ReadOutputMessage& query,
				ModbusRTU::ReadOutputRetMessage& reply );

		virtual ModbusRTU::mbErrCode readInputRegisters( ModbusRTU::ReadInputMessage& query,
				ModbusRTU::ReadInputRetMessage& reply );

		virtual ModbusRTU::mbErrCode forceSingleCoil( ModbusRTU::ForceSingleCoilMessage& query,
				ModbusRTU::ForceSingleCoilRetMessage& reply );

		virtual ModbusRTU::mbErrCode writeOutputSingleRegister( ModbusRTU::WriteSingleOutputMessage& query,
				ModbusRTU::WriteSingleOutputRetMessage& reply );

		virtual ModbusRTU::mbErrCode forceMultipleCoils( ModbusRTU::ForceCoilsMessage& query,
				ModbusRTU::ForceCoilsRetMessage& reply );

		virtual ModbusRTU::mbErrCode writeOutputRegisters( ModbusRTU::WriteOutputMessage& query,
				ModbusRTU::WriteOutputRetMessage& reply );

		virtual ModbusRTU::mbErrCode diagnostics( ModbusRTU::DiagnosticMessage& query,
				ModbusRTU::DiagnosticRetMessage& reply );

		virtual ModbusRTU::mbErrCode read4314( ModbusRTU::MEIMessageRDI& query,
											   ModbusRTU::MEIMessageRetRDI& reply );

		virtual ModbusRTU::mbErrCode journalCommand( ModbusRTU::JournalCommandMessage& query,
				ModbusRTU::JournalCommandRetMessage& reply );

		virtual ModbusRTU::mbErrCode setDateTime( ModbusRTU::SetDateTimeMessage& query,
				ModbusRTU::SetDateTimeRetMessage& reply );

		virtual ModbusRTU::mbErrCode remoteService( ModbusRTU::RemoteServiceMessage& query,
				ModbusRTU::RemoteServiceRetMessage& reply );

		virtual ModbusRTU::mbErrCode fileTransfer( ModbusRTU::FileTransferMessage& query,
				ModbusRTU::FileTransferRetMessage& reply );

	private:
		std::queue<unsigned char> qrecv;
		ModbusTCP::MBAPHeader curQueryHeader;
		std::unordered_set<ModbusRTU::ModbusAddr> vaddr;
		PassiveTimer ptTimeout;
		timeout_t timeout = { 0 };
		ModbusRTU::ModbusMessage buf;

		bool ignoreAddr = { false };
		std::string peername = { "" };

		std::string caddr = { "" };

		FinalSlot slFin;

		std::atomic_bool cancelled = { false };

		// статистика
		UniSetTypes::uniset_rwmutex mAsk;
		unsigned int askCount = { 0 };
};
// -------------------------------------------------------------------------
#endif // ModbusTCPSession_H_
// -------------------------------------------------------------------------
