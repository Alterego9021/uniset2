// $Id: IOBase.h,v 1.3 2009/01/23 23:56:54 vpashka Exp $
// -----------------------------------------------------------------------------
#ifndef IOBase_H_
#define IOBase_H_
// -----------------------------------------------------------------------------
#include <string>
#include "PassiveTimer.h"
#include "Trigger.h"
#include "Mutex.h"
#include "DigitalFilter.h"
#include "Calibration.h"
#include "IOController.h"
#include "SMInterface.h"
// -----------------------------------------------------------------------------
static const int DefaultSubdev 	= -1;
static const int DefaultChannel = -1;
static const int NoSafety = -1;
// -----------------------------------------------------------------------------
		/*! ���������� � �����/������ */
		struct IOBase
		{
			IOBase():
				cdiagram(0),
				breaklim(0),
				value(0),
				craw(0),
				safety(0),
				defval(0),
				df(1),
				nofilter(false),
				f_median(false),
				f_ls(false),
				f_filter_iir(false),
				ignore(false),
				invert(false),
				noprecision(false),
				jar_state(false),
				ondelay_state(false),
				offdelay_state(false),
				t_ai(UniSetTypes::DefaultObjectId)
			{}


			bool check_channel_break( long val ); 	/*!< �������� ������ ������� */

			bool check_jar( bool val );			/*!< ���������� ������� ������ �������� */
			bool check_on_delay( bool val );	/*!< ���������� �������� �� ��������� */
			bool check_off_delay( bool val );	/*!< ���������� �������� �� ���������� */
			
			IOController_i::SensorInfo si;
			UniversalIO::IOTypes stype;			/*!< ��� ������ (DI,DO,AI,AO) */
			IOController_i::CalibrateInfo cal; 	/*!< ������������� ��������� */
			Calibration* cdiagram;				/*!< ����������� ������������� ��������� */

			long breaklim; 	/*!< �������� �������� ����� ������������ ����� (�������� '�����' ��������) */
			long value;		/*!< ������� �������� */
			long craw;		/*!< ������� '�����' �������� �� ���������� */
			long cprev;		/*!< ���������� �������� ����� ���������� */
			long safety;	/*!< ���������� ��������� ��� ���������� �������� */
			long defval;	/*!< ���������� ��������� ��� ���������� �������� */

			DigitalFilter df;	/*!< ���������� ������������ ������� */
			bool nofilter;		/*!< ���������� ������� */
			bool f_median;		/*!< ������� ������������� ���������� ������� */
			bool f_ls;			/*!< ������� ������������� ����������� ������� �� ������ ���������� ��������� */
			bool f_filter_iir;	/*!< ������� ������������� ������������ ������� */

			bool ignore;	/*!< ������������ ��� ������ */
			bool invert;	/*!< ��������������� ������ */
			bool noprecision;
			
			PassiveTimer ptJar; 		/*!< ������ �� ������� */
			PassiveTimer ptOnDelay; 	/*!< �������� �� ������������ */
			PassiveTimer ptOffDelay; 	/*!< �������� �� ���������� */
			
			bool jar_pause;
			Trigger trOnDelay;
			Trigger trOffDelay;
			Trigger trJar;
			
			bool jar_state;			/*!< �������� ��� ������� �������� */
			bool ondelay_state;		/*!< �������� ��� �������� ��������� */
			bool offdelay_state;	/*!< �������� ��� �������� ���������� */
			
			
			
			// �����
			UniSetTypes::ObjectId t_ai; /*!< ���� ������ ������ ����������,
												� �������� ���������, �� � ������ ����
												�������� ������������� ����������� �������
												� ������� �� ������ */
			IONotifyController_i::ThresholdInfo ti;
	
			
			IOController::AIOStateList::iterator ait;
			IOController::DIOStateList::iterator dit;
			UniSetTypes::uniset_spin_mutex val_lock; 	/*!< ���� ����������� ������ �� ��������� */
			
			friend std::ostream& operator<<(std::ostream& os, IOBase& inf );

			static void processingFasAI( IOBase* it, float new_val, SMInterface* shm, bool force );
			static void processingAsAI( IOBase* it, long new_val, SMInterface* shm, bool force );
			static void processingAsDI( IOBase* it, bool new_set, SMInterface* shm, bool force );
			static long processingAsAO( IOBase* it, SMInterface* shm, bool force );
			static float processingFasAO( IOBase* it, SMInterface* shm, bool force );
			static bool processingAsDO( IOBase* it, SMInterface* shm, bool force );
			static void processingThreshold( IOBase* it, SMInterface* shm, bool force );
			static bool initItem( IOBase* b, UniXML_iterator& it, SMInterface* shm,  
									DebugStream* dlog=0, std::string myname="",
									int def_filtersize=0, float def_filterT=0.0,
									float def_lsparam=0.2, float def_iir_coeff_prev=0.5,
									float def_iir_coeff_new=0.5 );
		};



// -----------------------------------------------------------------------------
#endif // IOBase_H_
// -----------------------------------------------------------------------------
