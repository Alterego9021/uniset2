/*! $Id: ModbusRTUErrors.h,v 1.2 2008/01/01 00:26:23 vpashka Exp $ */
// -------------------------------------------------------------------------
#ifndef ModbusRTUErrors_H_
#define ModbusRTUErrors_H_
// -------------------------------------------------------------------------
#include <string>
#include <iostream>
// -------------------------------------------------------------------------
namespace ModbusRTU
{
	/*! ������ ������ 
		��� ������ > InternalErrorCode � ���� �� ����������...
	*/
	enum mbErrCode
	{
		erNoError 				= 0, /*!< ��� ������ */
	    erUnExpectedPacketType  = 1, /*!< ����������� ��� ������ (������ ���� �������) */
		erBadDataAddress 		= 2, /*!< ����� �������� � ������ ��� �� ���������� */
		erBadDataValue 			= 3, /*!< ������������ ��������  */
	    erHardwareError 		= 4, /*!< ������ ������������ */
		erAnknowledge			= 5, /*!< ������ ������ � ����������, �� �ݣ �� �������� */
		erSlaveBusy 			= 6, /*!< ���������� ����� ���������� ��������� (��������� ������ �����) */
		erOperationFailed		= 7, /*!< ������������� ������� ��������� ������������� ����������� */
		erMemoryParityError		= 8, /*!< ������ �������� ��� ������ ������ */

		erInternalErrorCode	= 10,	/*!< ���� ������ ������������ ��� ���������� ������ */
		erInvalidFormat 	= 11, 	/*!< ������������ ������ */
	    erBadCheckSum 		= 12, 	/*!< � ������ �� ������� ����������� ����� */
		erBadReplyNodeAddress = 13, /*!< ����� �� ������ ��������� �� ��� ��� �� �������,������� �� ���������� */
	    erTimeOut 			= 14,	/*!< ����-��� ��� ������ ������ */
    	erPacketTooLong 	= 15 	/*!< ����� ������ ������ ������ */
	};

	// ---------------------------------------------------------------------
	std::string mbErr2Str( mbErrCode e );
	// ---------------------------------------------------------------------
	class mbException
	{
		public:
			mbException():err(ModbusRTU::erNoError){}
			mbException( ModbusRTU::mbErrCode err):err(err){}
		
			ModbusRTU::mbErrCode err;

			friend std::ostream& operator<<(std::ostream& os, mbException& ex )
			{
				return os << "(" << ex.err << ") " << mbErr2Str(ex.err);
			}
	};
	// ---------------------------------------------------------------------
}
// -------------------------------------------------------------------------
#endif // ModbusRTUErrors_H_
// -------------------------------------------------------------------------
