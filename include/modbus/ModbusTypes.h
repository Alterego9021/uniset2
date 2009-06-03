/*! $Id: ModbusTypes.h,v 1.11 2008/11/22 23:22:24 vpashka Exp $ */
// -------------------------------------------------------------------------
#ifndef ModbusTypes_H_
#define ModbusTypes_H_
// -------------------------------------------------------------------------
#include <ostream>
#include <bitset>
#include "ModbusRTUErrors.h"
// -------------------------------------------------------------------------
/* �������� �������������:	
 * - ������� � ������� ���� ���������������� ������ � CRC
 * - � ������ ������������� ������� ������(�������), ���������� ������ � �.�
 * 		����� ������ �� ����������, � ����� �������������...
 * - CRC ��������� �� ���� ������� (� ��������� �������)
 * - CRC ���������������� ��������� 0xffff
 * - CRC �� ����������������
 * - ��� ������������ ����� ����������������. ������� ����: ������� �������
*/
// -------------------------------------------------------------------------
namespace ModbusRTU
{
	// ������� ���� 
	typedef unsigned char ModbusByte;	/*!< modbus-���� */
	const int BitsPerByte = 8;
	typedef unsigned char ModbusAddr;	/*!< ����� ���� � modbus-���� */
	typedef unsigned short ModbusData;	/*!< ������ ������ � modbus-���������� */
	const int BitsPerData = 16;
	typedef unsigned short ModbusCRC;	/*!< ������ CRC16 � modbus-���������� */

	// ---------------------------------------------------------------------
	/*! ���� ������������ ������� (�������� �������� modbus) */
	enum SlaveFunctionCode
	{
		fnUnknown				= 0x00,
		fnReadCoilStatus		= 0x01, /*!< read coil status */
		fnReadInputStatus		= 0x02, /*!< read input status */
		fnReadOutputRegisters	= 0x03, /*!< read register outputs or memories or read word outputs or memories */
		fnReadInputRegisters	= 0x04, /*!< read input registers or memories or read word outputs or memories */
		fnForceSingleCoil		= 0x05, /*!< forces a single coil to either ON or OFF */
		fnWriteOutputSingleRegister = 0x06,	/*!< write register outputs or memories */
		fnForceMultipleCoils	= 0x0F,	/*!< force multiple coils */
		fnWriteOutputRegisters	= 0x10,	/*!< write register outputs or memories */
		fnReadFileRecord		= 0x14,	/*!< read file record */
		fnWriteFileRecord		= 0x15,	/*!< write file record */
		fnSetDateTime			= 0x50, /*!< set date and time */
		fnRemoteService			= 0x53,	/*!< call remote service */
		fnJournalCommand		= 0x65,	/*!< read,write,delete alarm journal */
		fnFileTransfer			= 0x66	/*!< file transfer */
	};

	/*! ��������� ������� ��������� */
	enum
	{
		/*! ������������ ���������� ������ � ������ (c �ޣ��� ����������� �����) */
		MAXLENPACKET 	= 251,	/*!< ������������ ����� ������ 255 - header(2) - CRC(2) */
		BroadcastAddr	= 255	/*!< ����� ��� ����������������� ��������� */
	};

	const unsigned char MBErrMask = 0x80;

	// ---------------------------------------------------------------------
	/*! ���ޣ� ����������� ����� */
	ModbusCRC checkCRC( ModbusByte* start, int len );
	const int szCRC = sizeof(ModbusCRC); /*!< ������ ������ ��� ����������� ����� */
	// ---------------------------------------------------------------------
	/*! ����� ��������� */
	std::ostream& mbPrintMessage( std::ostream& os, ModbusByte* b, int len );
	// -------------------------------------------------------------------------
	ModbusAddr str2mbAddr( const std::string val );
	ModbusData str2mbData( const std::string val );
	std::string dat2str( const ModbusData dat );
	std::string addr2str( const ModbusAddr addr );
	std::string b2str( const ModbusByte b );
	// -------------------------------------------------------------------------
	float dat2f( const ModbusData dat1, const ModbusData dat2 );
	// -------------------------------------------------------------------------
	/*! ��������� ��������� */
	struct ModbusHeader
	{
		ModbusAddr addr;		/*!< ����� � ���� */
		ModbusByte func;		/*!< ��� ������� */

		ModbusHeader():addr(0),func(0){}
	}__attribute__((packed));

	const int szModbusHeader = sizeof(ModbusHeader);
	std::ostream& operator<<(std::ostream& os, ModbusHeader& m );
	std::ostream& operator<<(std::ostream& os, ModbusHeader* m );
	// -----------------------------------------------------------------------

	/*! ������� (�����) ��������� 
		\todo ����� ������������� ModbusMessage � TransportMessage?
	*/
	struct ModbusMessage:
		public ModbusHeader
	{
		ModbusMessage();
		ModbusByte data[MAXLENPACKET+szCRC]; 	/*!< ������ */

		// ��� ���� ��������������� � ������������ ��� ���������
		int len;	/*!< ����������� ����� */
	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, ModbusMessage& m );
	std::ostream& operator<<(std::ostream& os, ModbusMessage* m );
	// -----------------------------------------------------------------------
	/*! ����� ���������� �� ������ */	
	struct ErrorRetMessage:
		public ModbusHeader
	{
		ModbusByte ecode;
		ModbusCRC crc;
		
		// ------- from slave -------
		ErrorRetMessage( ModbusMessage& m );
		ErrorRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );
		
		// ------- to master -------
		ErrorRetMessage( ModbusAddr _from, ModbusByte _func, ModbusByte ecode );

		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();

		/*! ������ ������(����� ���������) � ������� ���� ��������� 
			��� ������� ���� �� ����������..
		*/
		inline static int szData(){ return sizeof(ModbusByte)+szCRC; }
	};

	std::ostream& operator<<(std::ostream& os, ErrorRetMessage& m ); 
	std::ostream& operator<<(std::ostream& os, ErrorRetMessage* m ); 
	// -----------------------------------------------------------------------
	struct DataBits
	{	
		DataBits( ModbusByte b );
		DataBits( std::string s ); // example "10001111"
		DataBits();
		
		const DataBits& operator=(const ModbusByte& r);

		operator ModbusByte();
		ModbusByte mbyte();
		
		bool operator[]( const int i ){ return b[i]; }
		
		std::bitset<BitsPerByte> b;
	};

	std::ostream& operator<<(std::ostream& os, DataBits& m );
	std::ostream& operator<<(std::ostream& os, DataBits* m ); 
	// -----------------------------------------------------------------------
	struct DataBits16
	{	
		DataBits16( ModbusData d );
		DataBits16( std::string s ); // example "1000111110001111"
		DataBits16();
		
		const DataBits16& operator=(const ModbusData& r);

		operator ModbusData();
		ModbusData mdata();
		
		bool operator[]( const int i ){ return b[i]; }
		void set( int n, bool s ){ b.set(n,s); }
		
		std::bitset<BitsPerData> b;
	};

	std::ostream& operator<<(std::ostream& os, DataBits16& m );
	std::ostream& operator<<(std::ostream& os, DataBits16* m ); 
	// -----------------------------------------------------------------------
	/*! ������ 0x01 */	
	struct ReadCoilMessage:
		public ModbusHeader
	{
		ModbusData start;
		ModbusData count;
		ModbusCRC crc;
		
		// ------- to slave -------
		ReadCoilMessage( ModbusAddr addr, ModbusData start, ModbusData count );
		/*! �������������� ��� ������� � ���� */
		ModbusMessage transport_msg();

		// ------- from master -------
		ReadCoilMessage( ModbusMessage& m );
		ReadCoilMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		inline static int szData(){ return sizeof(ModbusData)*2 + szCRC; }

	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, ReadCoilMessage& m ); 
	std::ostream& operator<<(std::ostream& os, ReadCoilMessage* m ); 

	// -----------------------------------------------------------------------
	
	/*! ����� �� 0x01 */	
	struct ReadCoilRetMessage:
		public ModbusHeader
	{
		ModbusByte bcnt;				/*!< numbers of bytes */
		ModbusByte data[MAXLENPACKET];	/*!< ������ */

		// ------- from slave -------
		ReadCoilRetMessage( ModbusMessage& m );
		ReadCoilRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );
		/*! ������ ���������������� ��������� 
		 * (����� ��������� �� ����������� ������) 
		*/
		static inline int szHead()
		{
			return sizeof(ModbusByte); // bcnt
		}

		/*! ������ ����� ������ ��������� �� ��������������� ���������� ( � ������ ) */
		static int getDataLen( ModbusMessage& m );
		ModbusCRC crc;
		
		// ------- to master -------
		ReadCoilRetMessage( ModbusAddr _from );

		/*! ���������� ������.
		 * \return TRUE - ���� �������
		 * \return FALSE - ���� �� �������
		*/
		bool addData( DataBits d );

		/*! ���������� ���.
		 * \param dnum  - ����� �����
		 * \param bnum  - ����� ����
		 * \param state - ���������
		 * \return TRUE - ���� ����
		 * \return FALSE - ���� �� �������
		*/
		bool setBit( unsigned char dnum, unsigned char bnum, bool state );

		/*! ��������� ������.
		 * \param dnum  - ����� �����
		 * \param d     - ��������� ������
		 * \return TRUE - ���� ����
		 * \return FALSE - ���� �� �������
		*/
		bool getData( unsigned char dnum, DataBits& d );

		/*! ������� ������ */
		void clear();
		
		/*! �������� �� ������������ */	
		inline bool isFull() 		
		{
			return ( bcnt >= MAXLENPACKET );
		}

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();
		
		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
	};

	std::ostream& operator<<(std::ostream& os, ReadCoilRetMessage& m );
	std::ostream& operator<<(std::ostream& os, ReadCoilRetMessage* m );
	// -----------------------------------------------------------------------
	/*! ������ 0x02 */	
	struct ReadInputStatusMessage:
		public ModbusHeader
	{
		ModbusData start;
		ModbusData count;
		ModbusCRC crc;
		
		// ------- to slave -------
		ReadInputStatusMessage( ModbusAddr addr, ModbusData start, ModbusData count );
		/*! �������������� ��� ������� � ���� */
		ModbusMessage transport_msg();

		// ------- from master -------
		ReadInputStatusMessage( ModbusMessage& m );
		ReadInputStatusMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		inline static int szData(){ return sizeof(ModbusData)*2 + szCRC; }

	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, ReadInputStatusMessage& m ); 
	std::ostream& operator<<(std::ostream& os, ReadInputStatusMessage* m ); 
	// -----------------------------------------------------------------------
	/*! ����� �� 0x02 */	
	struct ReadInputStatusRetMessage:
		public ModbusHeader
	{
		ModbusByte bcnt;				/*!< numbers of bytes */
		ModbusByte data[MAXLENPACKET];	/*!< ������ */

		// ------- from slave -------
		ReadInputStatusRetMessage( ModbusMessage& m );
		ReadInputStatusRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );
		/*! ������ ���������������� ��������� 
		 * (����� ��������� �� ����������� ������) 
		*/
		static inline int szHead()
		{
			return sizeof(ModbusByte); // bcnt
		}

		/*! ������ ����� ������ ��������� �� ��������������� ���������� ( � ������ ) */
		static int getDataLen( ModbusMessage& m );
		ModbusCRC crc;
		
		// ------- to master -------
		ReadInputStatusRetMessage( ModbusAddr _from );

		/*! ���������� ������.
		 * \return TRUE - ���� �������
		 * \return FALSE - ���� �� �������
		*/
		bool addData( DataBits d );

		/*! ���������� ���.
		 * \param dnum  - ����� �����
		 * \param bnum  - ����� ����
		 * \param state - ���������
		 * \return TRUE - ���� ����
		 * \return FALSE - ���� �� �������
		*/
		bool setBit( unsigned char dnum, unsigned char bnum, bool state );

		/*! ��������� ������.
		 * \param dnum  - ����� �����
		 * \param d     - ��������� ������
		 * \return TRUE - ���� ����
		 * \return FALSE - ���� �� �������
		*/
		bool getData( unsigned char dnum, DataBits& d );

		/*! ������� ������ */
		void clear();
		
		/*! �������� �� ������������ */	
		inline bool isFull() 		
		{
			return ( bcnt >= MAXLENPACKET );
		}

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();
		
		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
	};

	std::ostream& operator<<(std::ostream& os, ReadInputStatusRetMessage& m );
	std::ostream& operator<<(std::ostream& os, ReadInputStatusRetMessage* m );
	// -----------------------------------------------------------------------

	/*! ������ 0x03 */	
	struct ReadOutputMessage:
		public ModbusHeader
	{
		ModbusData start;
		ModbusData count;
		ModbusCRC crc;
		
		// ------- to slave -------
		ReadOutputMessage( ModbusAddr addr, ModbusData start, ModbusData count );
		/*! �������������� ��� ������� � ���� */
		ModbusMessage transport_msg();

		// ------- from master -------
		ReadOutputMessage( ModbusMessage& m );
		ReadOutputMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		inline static int szData(){ return sizeof(ModbusData)*2 + szCRC; }

	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, ReadOutputMessage& m ); 
	std::ostream& operator<<(std::ostream& os, ReadOutputMessage* m ); 
	// -----------------------------------------------------------------------
	/*! ����� ��� 0x03 */	
	struct ReadOutputRetMessage:
		public ModbusHeader
	{
		ModbusByte bcnt;									/*!< numbers of bytes */
		ModbusData data[MAXLENPACKET/sizeof(ModbusData)];	/*!< ������ */

		// ------- from slave -------
		ReadOutputRetMessage( ModbusMessage& m );
		ReadOutputRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );
		/*! ������ ���������������� ��������� 
		 * (����� ��������� �� ����������� ������) 
		*/
		static inline int szHead()
		{
			// bcnt
			return sizeof(ModbusByte);
		}

		/*! ������ ����� ������ ��������� �� ��������������� ���������� ( � ������ ) */
		static int getDataLen( ModbusMessage& m );
		ModbusCRC crc;
		
		// ------- to master -------
		ReadOutputRetMessage( ModbusAddr _from );

		/*! ���������� ������.
		 * \return TRUE - ���� �������
		 * \return FALSE - ���� �� �������
		*/
		bool addData( ModbusData d );

		/*! ������� ������ */
		void clear();
		
		/*! �������� �� ������������ */	
		inline bool isFull() 		
		{
			return ( count*sizeof(ModbusData) >= MAXLENPACKET );
		}

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();
		
		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
		
		// ��� ���� �� ������ � �������� modbus
		// ��� ��������������� � ������������ ��� 
		// �������������� � ModbusMessage.
		// ������ ���-���� memcpy(buf,this,sizeof(*this)); ����� �� �����. 
		// ����������� ����������� ������� transport_msg()
		int	count;	/*!< ����������� ���������� ������ � ��������� */
	};

	std::ostream& operator<<(std::ostream& os, ReadOutputRetMessage& m );
	std::ostream& operator<<(std::ostream& os, ReadOutputRetMessage* m );
	// -----------------------------------------------------------------------
	/*! ������ 0x04 */	
	struct ReadInputMessage:
		public ModbusHeader
	{
		ModbusData start;
		ModbusData count;
		ModbusCRC crc;
		
		// ------- to slave -------
		ReadInputMessage( ModbusAddr addr, ModbusData start, ModbusData count );
		/*! �������������� ��� ������� � ���� */
		ModbusMessage transport_msg();

		// ------- from master -------
		ReadInputMessage( ModbusMessage& m );
		ReadInputMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		inline static int szData(){ return sizeof(ModbusData)*2 + szCRC; }

	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, ReadInputMessage& m ); 
	std::ostream& operator<<(std::ostream& os, ReadInputMessage* m ); 
	// -----------------------------------------------------------------------

	/*! ����� ��� 0x04 */
	struct ReadInputRetMessage:
		public ModbusHeader
	{
		ModbusByte bcnt;									/*!< numbers of bytes */
		ModbusData data[MAXLENPACKET/sizeof(ModbusData)];	/*!< ������ */

		// ------- from slave -------
		ReadInputRetMessage( ModbusMessage& m );
		ReadInputRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );
		/*! ������ ���������������� ��������� 
		 * (����� ��������� �� ����������� ������) 
		*/
		static inline int szHead()
		{
			// bcnt
			return sizeof(ModbusByte);
		}

		/*! ������ ����� ������ ��������� �� ��������������� ���������� ( � ������ ) */
		static int getDataLen( ModbusMessage& m );
		ModbusCRC crc;
		
		// ------- to master -------
		ReadInputRetMessage( ModbusAddr _from );

		/*! ���������� ������.
		 * \return TRUE - ���� �������
		 * \return FALSE - ���� �� �������
		*/
		bool addData( ModbusData d );

		/*! ������� ������ */
		void clear();
		
		/*! �������� �� ������������ */	
		inline bool isFull() 		
		{
			return ( count*sizeof(ModbusData) >= MAXLENPACKET );
		}

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();
		
		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
		
		// ��� ���� �� ������ � �������� modbus
		// ��� ��������������� � ������������ ��� 
		// �������������� � ModbusMessage.
		// ������ ���-���� memcpy(buf,this,sizeof(*this)); ����� �� �����. 
		// ����������� ����������� ������� transport_msg()
		int	count;	/*!< ����������� ���������� ������ � ��������� */
	};

	std::ostream& operator<<(std::ostream& os, ReadInputRetMessage& m );
	std::ostream& operator<<(std::ostream& os, ReadInputRetMessage* m );
	// -----------------------------------------------------------------------
	/*! ������ �� ������ 0x0F */	
	struct ForceCoilsMessage:
		public ModbusHeader
	{
		ModbusData start;	/*!< ��������� ����� ������ */
		ModbusData quant;	/*!< ���������� ���� ������ */ 
		ModbusByte bcnt;	/*!< ���������� ���� ������ */
		/*! ������ */
		ModbusData data[MAXLENPACKET/sizeof(ModbusData)-sizeof(ModbusData)*2-sizeof(ModbusByte)];
		ModbusCRC crc;		/*!< ����������� ����� */

		// ------- to slave -------
		ForceCoilsMessage( ModbusAddr addr, ModbusData start );
		/*! �������������� ��� ������� � ���� */
		ModbusMessage transport_msg();

		/*! ���������� ������.
		 * \return TRUE - ���� �������
		 * \return FALSE - ���� �� �������
		*/
		bool addData( DataBits16 d );

		/*! ���������� ���.
		 * \param dnum  - ����� �����
		 * \param bnum  - ����� ����
		 * \param state - ���������
		 * \return TRUE - ���� ����
		 * \return FALSE - ���� �� �������
		*/
		bool setBit( unsigned char dnum, unsigned char bnum, bool state );

		/*! ��������� ������.
		 * \param dnum  - ����� �����
		 * \param d     - ��������� ������
		 * \return TRUE - ���� ����
		 * \return FALSE - ���� �� �������
		*/
		bool getData( unsigned char dnum, DataBits16& d );

		void clear();
		inline bool isFull() 		
		{
			return ( quant*sizeof(ModbusData) >= MAXLENPACKET );
		}

		// ------- from master -------	
		ForceCoilsMessage( ModbusMessage& m );
		ForceCoilsMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();

		/*! ������ ���������������� ��������� 
		 * (����� ��������� �� ����������� ������) 
		*/
		static inline int szHead()
		{
			// start + quant + count
			return sizeof(ModbusData)*2+sizeof(ModbusByte);
		}
		
		/*! ������ ����� ������ ��������� �� ��������������� ���������� ( � ������ ) */
		static int getDataLen( ModbusMessage& m );

		/*! �������� ������������ ������ 
			��� quant � bcnt - ���������...
		*/
		bool checkFormat();
		
	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, ForceCoilsMessage& m );
	std::ostream& operator<<(std::ostream& os, ForceCoilsMessage* m );
	// -----------------------------------------------------------------------
	/*! ����� ��� ������� �� ������ 0x0F */	
	struct ForceCoilsRetMessage:
		public ModbusHeader
	{
		ModbusData start; 	/*!< ���������� ��������� ����� */
		ModbusData quant;	/*!< ���������� ���������� ���� ������ */

		// ------- from slave -------
		ForceCoilsRetMessage( ModbusMessage& m );
		ForceCoilsRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );
		ModbusCRC crc;

		// ------- to master -------
		/*! 
		 * \param _from - ����� �����������
		 * \param start	- ���������� �������
		 * \param quant	- ���������� ���������� ����
		*/
		ForceCoilsRetMessage( ModbusAddr _from, ModbusData start=0, ModbusData quant=0 );

		/*! �������� ������ */
		void set( ModbusData start, ModbusData quant );

		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
		
		/*! ������ ������(����� ���������) � ������� ���� ��������� 
			��� ������� ���� �� ����������..
		*/
		inline static int szData(){ return sizeof(ModbusData)*2+sizeof(ModbusCRC); }
	};
	
	std::ostream& operator<<(std::ostream& os, ForceCoilsRetMessage& m );
	std::ostream& operator<<(std::ostream& os, ForceCoilsRetMessage* m );
	// -----------------------------------------------------------------------

	/*! ������ �� ������ 0x10 */	
	struct WriteOutputMessage:
		public ModbusHeader
	{
		ModbusData start;	/*!< ��������� ����� ������ */
		ModbusData quant;	/*!< ���������� ���� ������ */ 
		ModbusByte bcnt;	/*!< ���������� ���� ������ */
		/*! ������ */
		ModbusData data[MAXLENPACKET/sizeof(ModbusData)-sizeof(ModbusData)*2-sizeof(ModbusByte)];
		ModbusCRC crc;		/*!< ����������� ����� */

		// ------- to slave -------
		WriteOutputMessage( ModbusAddr addr, ModbusData start );
		/*! �������������� ��� ������� � ���� */
		ModbusMessage transport_msg();

		bool addData( ModbusData d );
		void clear();
		inline bool isFull() 		
		{
			return ( quant*sizeof(ModbusData) >= MAXLENPACKET );
		}

		// ------- from master -------	
		WriteOutputMessage( ModbusMessage& m );
		WriteOutputMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();

		/*! ������ ���������������� ��������� 
		 * (����� ��������� �� ����������� ������) 
		*/
		static inline int szHead()
		{
			// start + quant + count
			return sizeof(ModbusData)*2+sizeof(ModbusByte);
		}
		
		/*! ������ ����� ������ ��������� �� ��������������� ���������� ( � ������ ) */
		static int getDataLen( ModbusMessage& m );

		/*! �������� ������������ ������ 
			��� quant � bcnt - ���������...
		*/
		bool checkFormat();
		
	}__attribute__((packed));


	std::ostream& operator<<(std::ostream& os, WriteOutputMessage& m );
	std::ostream& operator<<(std::ostream& os, WriteOutputMessage* m );

	/*! ����� ��� ������� �� ������ 0x10 */	
	struct WriteOutputRetMessage:
		public ModbusHeader
	{
		ModbusData start; 	/*!< ���������� ��������� ����� */
		ModbusData quant;	/*!< ���������� ���������� ���� ������ */

		// ------- from slave -------
		WriteOutputRetMessage( ModbusMessage& m );
		WriteOutputRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );
		ModbusCRC crc;

		// ------- to master -------
		/*! 
		 * \param _from - ����� �����������
		 * \param start	- ���������� �������
		 * \param quant	- ���������� ���������� ����
		*/
		WriteOutputRetMessage( ModbusAddr _from, ModbusData start=0, ModbusData quant=0 );

		/*! �������� ������ */
		void set( ModbusData start, ModbusData quant );

		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
		
		/*! ������ ������(����� ���������) � ������� ���� ��������� 
			��� ������� ���� �� ����������..
		*/
		inline static int szData(){ return sizeof(ModbusData)*2+sizeof(ModbusCRC); }
	};
	
	std::ostream& operator<<(std::ostream& os, WriteOutputRetMessage& m );
	std::ostream& operator<<(std::ostream& os, WriteOutputRetMessage* m );
	// -----------------------------------------------------------------------
	/*! ������ 0x05 */	
	struct ForceSingleCoilMessage:
		public ModbusHeader
	{
		ModbusData start;	/*!< ��������� ����� ������ */
		ModbusData data;	/*!< ������� ON - true | OFF - false */
		ModbusCRC crc;		/*!< ����������� ����� */

		/*! �������� �������� ������� */
		inline bool cmd()
		{
			return (data & 0xFF00);
		}


		// ------- to slave -------
		ForceSingleCoilMessage( ModbusAddr addr, ModbusData reg, bool state );
		/*! �������������� ��� ������� � ���� */
		ModbusMessage transport_msg();
	
		// ------- from master -------
		ForceSingleCoilMessage( ModbusMessage& m );
		ForceSingleCoilMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();

		/*! ������ ���������������� ��������� 
		 * (����� ��������� �� ����������� ������) 
		*/
		static inline int szHead()
		{
			return sizeof(ModbusData);
		}
		
		/*! ������ ����� ������ ��������� �� 
			��������������� ���������� ( � ������ ) 
		*/
		static int getDataLen( ModbusMessage& m );

		/*! �������� ������������ ������ 
			��� quant � bcnt - ���������...
		*/
		bool checkFormat();
	}__attribute__((packed));


	std::ostream& operator<<(std::ostream& os, ForceSingleCoilMessage& m );
	std::ostream& operator<<(std::ostream& os, ForceSingleCoilMessage* m );
	// -----------------------------------------------------------------------

	/*! ����� ��� ������� 0x05 */	
	struct ForceSingleCoilRetMessage:
		public ModbusHeader
	{
		ModbusData start; 	/*!< ���������� ��������� ����� */
		ModbusData data; 	/*!< ������ */
		ModbusCRC crc;


		// ------- from slave -------
		ForceSingleCoilRetMessage( ModbusMessage& m );
		ForceSingleCoilRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		// ------- to master -------
		/*! 
		 * \param _from - ����� �����������
		 * \param start	- ���������� �������
		*/
		ForceSingleCoilRetMessage( ModbusAddr _from );

		/*! �������� ������ */
		void set( ModbusData start, bool cmd );

		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
		
		/*! ������ ������(����� ���������) � ������� ���� ��������� 
			��� ������� ���� �� ����������..
		*/
		inline static int szData(){ return 2*sizeof(ModbusData)+sizeof(ModbusCRC); }
	};
	
	std::ostream& operator<<(std::ostream& os, ForceSingleCoilRetMessage& m );
	std::ostream& operator<<(std::ostream& os, ForceSingleCoilRetMessage* m );
	// -----------------------------------------------------------------------

	/*! ������ �� ������ ������ �������� 0x06 */	
	struct WriteSingleOutputMessage:
		public ModbusHeader
	{
		ModbusData start;	/*!< ��������� ����� ������ */
		ModbusData data;	/*!< ������ */
		ModbusCRC crc;		/*!< ����������� ����� */


		// ------- to slave -------
		WriteSingleOutputMessage( ModbusAddr addr, ModbusData reg=0, ModbusData data=0 );
		/*! �������������� ��� ������� � ���� */
		ModbusMessage transport_msg();
	
		// ------- from master -------
		WriteSingleOutputMessage( ModbusMessage& m );
		WriteSingleOutputMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();

		/*! ������ ���������������� ��������� 
		 * (����� ��������� �� ����������� ������) 
		*/
		static inline int szHead()
		{
			return sizeof(ModbusData);
		}
		
		/*! ������ ����� ������ ��������� �� 
			��������������� ���������� ( � ������ ) 
		*/
		static int getDataLen( ModbusMessage& m );

		/*! �������� ������������ ������ 
			��� quant � bcnt - ���������...
		*/
		bool checkFormat();
	}__attribute__((packed));


	std::ostream& operator<<(std::ostream& os, WriteSingleOutputMessage& m );
	std::ostream& operator<<(std::ostream& os, WriteSingleOutputMessage* m );
	// -----------------------------------------------------------------------

	/*! ����� ��� ������� �� ������ */	
	struct WriteSingleOutputRetMessage:
		public ModbusHeader
	{
		ModbusData start; 	/*!< ���������� ��������� ����� */
		ModbusData data; 	/*!< ���������� ��������� ����� */
		ModbusCRC crc;


		// ------- from slave -------
		WriteSingleOutputRetMessage( ModbusMessage& m );
		WriteSingleOutputRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		// ------- to master -------
		/*! 
		 * \param _from - ����� �����������
		 * \param start	- ���������� �������
		*/
		WriteSingleOutputRetMessage( ModbusAddr _from, ModbusData start=0 );

		/*! �������� ������ */
		void set( ModbusData start, ModbusData data );

		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
		
		/*! ������ ������(����� ���������) � ������� ���� ��������� 
			��� ������� ���� �� ����������..
		*/
		inline static int szData(){ return 2*sizeof(ModbusData)+sizeof(ModbusCRC); }
	};
	
	std::ostream& operator<<(std::ostream& os, WriteSingleOutputRetMessage& m );
	std::ostream& operator<<(std::ostream& os, WriteSingleOutputRetMessage* m );
	// -----------------------------------------------------------------------

	/*! ������ ���������� �� ������ */	
	struct JournalCommandMessage:
		public ModbusHeader
	{
		ModbusData cmd;			/*!< ��� �������� */
		ModbusData num;			/*!< ����� ������ */
		ModbusCRC crc;
		
		// -------------
		JournalCommandMessage( ModbusMessage& m );
		JournalCommandMessage& operator=( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		inline static int szData(){ return sizeof(ModbusByte)*4 + szCRC; }

	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, JournalCommandMessage& m ); 
	std::ostream& operator<<(std::ostream& os, JournalCommandMessage* m ); 
	// -----------------------------------------------------------------------
	/*! ����� ��� ������� �� ������ ������ */	
	struct JournalCommandRetMessage:
		public ModbusHeader
	{
		ModbusByte bcnt;					/*!< numbers of bytes */
//		ModbusByte data[MAXLENPACKET-1];	/*!< ������ */

		// � ����� �� ���������� ���������� �������� ����� (�.�. modbus master)
		// ������ ���������� ������ �� �������� �������, � "�������"
		// ������� � ���� ������� ����� ����ף����� ��� �������...
		ModbusData data[MAXLENPACKET/sizeof(ModbusData)];	/*!< ������ */

		// -------------
		JournalCommandRetMessage( ModbusAddr _from );

		/*! ���������� ������
			\warning ������ ������ ����� ��ԣ���
			\warning ������������ ��������� ModbusByte*
				�.�. ���������� ���� ������ ����� ����
				�� ��������� �� ������!
		*/
		bool setData( ModbusByte* b, int len );

		/*! ������� ������ */
		void clear();
		
		/*! �������� �� ������������ */	
		inline bool isFull() 		
		{
			return ( count*sizeof(ModbusData) >= MAXLENPACKET );
		}

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();
		
		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
		
		// ��� ���� �� ������ � �������� modbus
		// ��� ��������������� � ������������ ��� 
		// �������������� � ModbusMessage.
		// ������ ���-���� memcpy(buf,this,sizeof(*this)); ����� �� �����. 
		// ����������� ����������� ������� transport_msg()
		int	count;	/*!< ����������� ���������� ������ � ��������� */
	};

	std::ostream& operator<<(std::ostream& os, JournalCommandRetMessage& m );
	std::ostream& operator<<(std::ostream& os, JournalCommandRetMessage* m );
	// -----------------------------------------------------------------------
	/*! ����� � ������ ������������� ������������� ������� 
		(������ ������ � JournalCommandRetMessage ��� ������� � ������ )
	*/
	struct JournalCommandRetOK:
		public JournalCommandRetMessage
	{
		// -------------
		JournalCommandRetOK( ModbusAddr _from );
		void set( ModbusData cmd, ModbusData ecode );
		static void set( JournalCommandRetMessage& m, ModbusData cmd, ModbusData ecode );
	};

	std::ostream& operator<<(std::ostream& os, JournalCommandRetOK& m ); 
	std::ostream& operator<<(std::ostream& os, JournalCommandRetOK* m ); 
	// -----------------------------------------------------------------------

	/*! ��������� ������� */	
	struct SetDateTimeMessage:
		public ModbusHeader
	{
		ModbusByte hour;	/*!< ���� [0..23] */
		ModbusByte min;		/*!< ������ [0..59] */
		ModbusByte sec;		/*!< ������� [0..59] */
		ModbusByte day;		/*!< ���� [1..31] */
		ModbusByte mon;		/*!< ����� [1..12] */
		ModbusByte year;	/*!< ��� [0..99] */
		ModbusByte century;	/*!< �������� [19-20] */

		ModbusCRC crc;
		
		// ------- to slave -------
		SetDateTimeMessage( ModbusAddr addr );
		/*! �������������� ��� ������� � ���� */
		ModbusMessage transport_msg();
		
		// ------- from master -------
		SetDateTimeMessage( ModbusMessage& m );
		SetDateTimeMessage& operator=( ModbusMessage& m );
		SetDateTimeMessage();

		bool checkFormat();

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		inline static int szData(){ return sizeof(ModbusByte)*7 + szCRC; }

	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, SetDateTimeMessage& m ); 
	std::ostream& operator<<(std::ostream& os, SetDateTimeMessage* m ); 
	// -----------------------------------------------------------------------

	/*! ����� (������ ��������� ������) */
	struct SetDateTimeRetMessage:
		public SetDateTimeMessage
	{

		// ------- from slave -------
		SetDateTimeRetMessage( ModbusMessage& m );
		SetDateTimeRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		// ------- to master -------
		SetDateTimeRetMessage( ModbusAddr _from );
		SetDateTimeRetMessage( const SetDateTimeMessage& query );
		static void cpy( SetDateTimeRetMessage& reply, SetDateTimeMessage& query );

		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
	};
	// -----------------------------------------------------------------------

	/*! ����� ���̣����� ������� */	
	struct RemoteServiceMessage:
		public ModbusHeader
	{
		ModbusByte bcnt;	/*!< ���������� ���� */

		/*! ������ */
		ModbusByte data[MAXLENPACKET-sizeof(ModbusByte)];
		ModbusCRC crc;		/*!< ����������� ����� */
	
		// -----------
		RemoteServiceMessage( ModbusMessage& m );
		RemoteServiceMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();

		/*! ������ ���������������� ��������� 
		 * (����� ��������� �� ����������� ������) 
		*/
		static inline int szHead()
		{ return sizeof(ModbusByte); } // bcnt
		
		/*! ������ ����� ������ ��������� �� ��������������� ���������� ( � ������ ) */
		static int getDataLen( ModbusMessage& m );

	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, RemoteServiceMessage& m ); 
	std::ostream& operator<<(std::ostream& os, RemoteServiceMessage* m ); 
	// -----------------------------------------------------------------------
	struct RemoteServiceRetMessage:
		public ModbusHeader
	{
		ModbusByte bcnt;	/*!< ���������� ���� */
		/*! ������ */
		ModbusByte data[MAXLENPACKET-sizeof(ModbusByte)];

		RemoteServiceRetMessage( ModbusAddr _from );

		/*! ���������� ������
			\warning ������ ������ ����� ��ԣ���
			\warning ������������ ��������� ModbusByte*
				�.�. ���������� ���� ������ ����� ����
				�� ��������� �� ������!
		*/
		bool setData( ModbusByte* b, int len );

		/*! ������� ������ */
		void clear();
		
		/*! �������� �� ������������ */	
		inline bool isFull() 		
			{ return ( count >= sizeof(data) ); }

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();
		
		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
		
		// ��� ���� �� ������ � �������� modbus
		// ��� ��������������� � ������������ ��� 
		// �������������� � ModbusMessage.
		unsigned int	count;	/*!< ����������� ���������� ������ � ��������� */
	};
	// -----------------------------------------------------------------------

	struct ReadFileRecordMessage:
		public ModbusHeader
	{
		struct SubRequest
		{
			ModbusByte reftype; /*!< referens type 06 */
			ModbusData numfile; /*!< file number 0x0000 to 0xFFFF */
			ModbusData numrec;  /*!< record number 0x0000 to 0x270F */
			ModbusData reglen;  /*!< registers length */
		}__attribute__((packed));	

		ModbusByte bcnt;	/*!< ���������� ���� 0x07 to 0xF5 */
 
		/*! ������ */
		SubRequest data[MAXLENPACKET/sizeof(SubRequest)-sizeof(ModbusByte)];
		ModbusCRC crc;		/*!< ����������� ����� */
	
		// -----------
		ReadFileRecordMessage( ModbusMessage& m );
		ReadFileRecordMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();

		/*! ������ ���������������� ��������� 
		 * (����� ��������� �� ����������� ������) 
		*/
		static inline int szHead()
		{ return sizeof(ModbusByte); } // bcnt
		
		/*! ������ ����� ������ ��������� �� ��������������� ���������� ( � ������ ) */
		static int getDataLen( ModbusMessage& m );

		/*! �������� ������������ ������ */
		bool checkFormat();
		
		// ��� ���� ��������� � �� ������������ � ��������� ������
		int count; /*!< ����������� ���������� ������ */
	};

	std::ostream& operator<<(std::ostream& os, ReadFileRecordMessage& m ); 
	std::ostream& operator<<(std::ostream& os, ReadFileRecordMessage* m ); 
	// -----------------------------------------------------------------------

	struct FileTransferMessage:
		public ModbusHeader
	{
		ModbusData numfile; 	/*!< file number 0x0000 to 0xFFFF */
		ModbusData numpacket;  	/*!< number of packet */
		ModbusCRC crc;			/*!< ����������� ����� */
	
		// ------- to slave -------
		FileTransferMessage( ModbusAddr addr, ModbusData numfile, ModbusData numpacket );
		ModbusMessage transport_msg(); 	/*!< �������������� ��� ������� � ���� */
	
		// ------- from master -------
		FileTransferMessage( ModbusMessage& m );
		FileTransferMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );

		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		static inline int szData()
		{	return sizeof(ModbusData)*2 + szCRC; }

	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, FileTransferMessage& m ); 
	std::ostream& operator<<(std::ostream& os, FileTransferMessage* m ); 
	// -----------------------------------------------------------------------

	struct FileTransferRetMessage:
		public ModbusHeader
	{
		// 255 - max of bcnt...(1 byte)		
//		static const int MaxDataLen = 255 - szCRC - szModbusHeader - sizeof(ModbusData)*3 - sizeof(ModbusByte)*2;
		static const int MaxDataLen = MAXLENPACKET - sizeof(ModbusData)*3 - sizeof(ModbusByte)*2;

		ModbusByte bcnt;		/*!< ����� ���������� ���� � ������ */
		ModbusData numfile; 	/*!< file number 0x0000 to 0xFFFF */
		ModbusData numpacks; 	/*!< all count packages (file size) */
		ModbusData packet;  	/*!< number of packet */
		ModbusByte dlen;		/*!< ���������� ���� ������ � ������ */
		ModbusByte data[MaxDataLen];

	
		// ------- from slave -------
		FileTransferRetMessage( ModbusMessage& m );
		FileTransferRetMessage& operator=( ModbusMessage& m );
		void init( ModbusMessage& m );
		ModbusCRC crc;
		static int szHead(){ return sizeof(ModbusByte); }
		static int getDataLen( ModbusMessage& m );
		
		// ------- to master -------
		FileTransferRetMessage( ModbusAddr _from );

		/*! ���������� ������
			\warning ������ ������ ����� ��ԣ���
		*/
		bool set( ModbusData numfile, ModbusData file_num_packets, ModbusData packet, ModbusByte* b, ModbusByte len );

		/*! ������� ������ */
		void clear();
		
		/*! ������ ������(����� ���������) � ������� ���� ��������� */
		int szData();
		
		/*! �������������� ��� ������� � ���� */	
		ModbusMessage transport_msg();
	};

	std::ostream& operator<<(std::ostream& os, FileTransferRetMessage& m ); 
	std::ostream& operator<<(std::ostream& os, FileTransferRetMessage* m );
	// -----------------------------------------------------------------------
} // end of ModbusRTU namespace
// ---------------------------------------------------------------------------
namespace ModbusTCP
{
	struct MBAPHeader
	{
		ModbusRTU::ModbusData	tID; /*!< transaction ID */
		ModbusRTU::ModbusData	pID; /*!< protocol ID */
		ModbusRTU::ModbusData	len; /*!< lenght */
/*		ModbusRTU::ModbusByte	uID; */ /*!< unit ID */ /* <------- see ModbusHeader */

		MBAPHeader():tID(0),pID(0) /*,uID(0) */{}
		
		void swapdata();

	}__attribute__((packed));

	std::ostream& operator<<(std::ostream& os, MBAPHeader& m ); 

  // -----------------------------------------------------------------------
} // end of namespace ModbusTCP
// ---------------------------------------------------------------------------
#endif // ModbusTypes_H_
// ---------------------------------------------------------------------------
