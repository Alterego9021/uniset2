/*! $Id: ModbusServerSlot.h,v 1.1 2008/11/22 23:22:24 vpashka Exp $ */
// -------------------------------------------------------------------------
#ifndef ModbusServerSlot_H_
#define ModbusServerSlot_H_
// -------------------------------------------------------------------------
#include <sigc++/sigc++.h>
#include "ModbusTypes.h"
#include "ModbusServer.h"
// -------------------------------------------------------------------------
/*! */
class ModbusServerSlot
{
	public:
		ModbusServerSlot();
		virtual ~ModbusServerSlot();
		
		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::ReadCoilMessage&,
							ModbusRTU::ReadCoilRetMessage&> ReadCoilSlot;

		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::ReadInputStatusMessage&,
							ModbusRTU::ReadInputStatusRetMessage&> ReadInputStatusSlot;
		
		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::ReadOutputMessage&,
							ModbusRTU::ReadOutputRetMessage&> ReadOutputSlot;
		
		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::ReadInputMessage&,
							ModbusRTU::ReadInputRetMessage&> ReadInputSlot;

		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::ForceSingleCoilMessage&,
							ModbusRTU::ForceSingleCoilRetMessage&> ForceSingleCoilSlot;

		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::WriteSingleOutputMessage&,
							ModbusRTU::WriteSingleOutputRetMessage&> WriteSingleOutputSlot;

		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::ForceCoilsMessage&,
							ModbusRTU::ForceCoilsRetMessage&> ForceCoilsSlot;

		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::WriteOutputMessage&,
							ModbusRTU::WriteOutputRetMessage&> WriteOutputSlot;

		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::JournalCommandMessage&,
							ModbusRTU::JournalCommandRetMessage&> JournalCommandSlot;

		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::SetDateTimeMessage&,
							ModbusRTU::SetDateTimeRetMessage&> SetDateTimeSlot;

		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::RemoteServiceMessage&,
							ModbusRTU::RemoteServiceRetMessage&> RemoteServiceSlot;

		typedef sigc::slot<ModbusRTU::mbErrCode,
							ModbusRTU::FileTransferMessage&,
							ModbusRTU::FileTransferRetMessage&> FileTransferSlot;

		/*! ����������� ����������� '��������� ������' 0x01 */
		void connectReadCoil( ReadCoilSlot sl );

		/*! ����������� ����������� '��������� ������' 0x02 */
		void connectReadInputStatus( ReadInputStatusSlot sl );

		/*! ����������� ����������� '��������� ������' 0x03 */
		void connectReadOutput( ReadOutputSlot sl );

		/*! ����������� ����������� '��������� ������' 0x04 */
		void connectReadInput( ReadInputSlot sl );

		/*! ����������� ����������� '������ ������' 0x05 */
		void connectForceSingleCoil( ForceSingleCoilSlot sl );

		/*! ����������� ����������� '������ ������' 0x06 */
		void connectWriteSingleOutput( WriteSingleOutputSlot sl );

		/*! ����������� ����������� '������ ������' 0x0F */
		void connectForceCoils( ForceCoilsSlot sl );

		/*! ����������� ����������� '������ ������' 0x10 */
		void connectWriteOutput( WriteOutputSlot sl );

		/*! ����������� ����������� '������ ������' 0x65 */
		void connectJournalCommand( JournalCommandSlot sl );

		/*! ����������� ����������� '��������� �������' 0x50 */
		void connectSetDateTime( SetDateTimeSlot sl );

		/*! ����������� ����������� '���̣���� ������' 0x53 */
		void connectRemoteService( RemoteServiceSlot sl );

		/*! ����������� ����������� '�������� �����' 0x66 */
		void connectFileTransfer( FileTransferSlot sl );

	protected:
		ReadCoilSlot slReadCoil;
		ReadInputStatusSlot slReadInputStatus;
		ReadOutputSlot slReadOutputs;
		ReadInputSlot slReadInputs;
		ForceCoilsSlot slForceCoils;
		WriteOutputSlot slWriteOutputs;
		ForceSingleCoilSlot slForceSingleCoil;
		WriteSingleOutputSlot slWriteSingleOutputs;
		JournalCommandSlot slJournalCommand;
		SetDateTimeSlot slSetDateTime;
		RemoteServiceSlot slRemoteService;
		FileTransferSlot slFileTransfer;
};
// -------------------------------------------------------------------------
#endif // ModbusServerSlot_H_
// -------------------------------------------------------------------------
