// $Id: Extensions.h,v 1.1 2008/12/14 21:57:50 vpashka Exp $
// -------------------------------------------------------------------------
#ifndef Extensions_H_
#define Extensions_H_
// -------------------------------------------------------------------------
#include <string>
#include "UniXML.h"
#include "Debug.h"
#include "UniSetTypes.h"
#include "Calibration.h"
// -------------------------------------------------------------------------
namespace UniSetExtensions
{
	/*! ��������� �������������� �������(��������) ����������� ������ */
	UniSetTypes::ObjectId getSharedMemoryID();

	/*! ��������� ������� ��� ������������� "�������" */
	int getHeartBeatTime();

	xmlNode* findNode( xmlNode* node, const std::string snode, const std::string field );
	
	xmlNode* getCalibrationsSection();
	
	/*! ������ ��������� �������� � ������ 
	 * '\\' -> '\n'
	*/
	void escape_string( std::string& s );

	/*! �������� ������������� ��������� */
	Calibration* buildCalibrationDiagram( const std::string dname );

	extern DebugStream dlog;
}
// -------------------------------------------------------------------------
#endif // Extensions_H_
