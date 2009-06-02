// --------------------------------------------------------------------------
//!  \version $Id: RTUTypes.h,v 1.1 2008/12/14 21:57:50 vpashka Exp $
// --------------------------------------------------------------------------
#ifndef _RTUTypes_H_
#define _RTUTypes_H_
// -----------------------------------------------------------------------------
#include <string>
#include <cmath>
#include <cstring>
#include <ostream>
#include "modbus/ModbusTypes.h"
// -----------------------------------------------------------------------------
namespace VTypes
{
		enum VType
		{
			vtUnknown,
			vtF2,
			vtF4
		};

		std::ostream& operator<<( std::ostream& os, const VType& vt );

		// -------------------------------------------------------------------------
		std::string type2str( VType t );			/*!< ������������� ������ � ��� */
		VType str2type( const std::string s );	/*!< �������������� �������� � ������ */
		int wsize( VType t ); 					/*!< ����� ������ � ������ */
	// -------------------------------------------------------------------------
	class F2
	{
		public:
		
			// ------------------------------------------
			static const int f2Size=2;
			/*! ��� �������� � ������ */
			typedef union
			{
				unsigned short v[f2Size];
				float val; // 
			} F2mem;
			// ------------------------------------------
			// ������������ �� ������ ������...
			F2(){ memset(raw.v,0,sizeof(raw.v)); }
			
			F2( const ModbusRTU::ModbusData* data, int size )
			{
				for( int i=0; i<wsize() && i<size; i++ )
					raw.v[i] = data[i];
			}

			~F2(){}
			// ------------------------------------------
			/*! ������ � ������ */
			static int wsize(){ return f2Size; }
			/*! ��� �������� */
			static VType type(){ return vtF2; }
			// ------------------------------------------
			operator float(){ return raw.val; }
			operator long(){ return lroundf(raw.val); }
			
			F2mem raw;
	};
	// --------------------------------------------------------------------------
	class F4
	{
		public:
			// ------------------------------------------
			static const int f4Size=4;
			/*! ��� �������� � ������ */
			typedef union
			{
				unsigned short v[f4Size];
				float val; // 
			} F4mem;
			// ------------------------------------------
			// ������������ �� ������ ������...
			F4(){ memset(raw.v,0,sizeof(raw.v)); }
			
			F4( const ModbusRTU::ModbusData* data, int size )
			{
				for( int i=0; i<wsize() && i<size; i++ )
					raw.v[i] = data[i];
			}

			~F4(){}
			// ------------------------------------------
			/*! ������ � ������ */
			static int wsize(){ return f4Size; }
			/*! ��� �������� */
			static VType type(){ return vtF4; }
			// ------------------------------------------
			operator float(){ return raw.val; }
			operator long(){ return lroundf(raw.val); }
			
			F4mem raw;
	};
	// --------------------------------------------------------------------------

} // end of namespace VTypes
// --------------------------------------------------------------------------
#endif // _RTUTypes_H_
// -----------------------------------------------------------------------------
