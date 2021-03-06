
// --------------------------------------------------------------------------
/*
 DO NOT EDIT THIS FILE. IT IS AUTOGENERATED FILE.
 ALL YOUR CHANGES WILL BE LOST.
 
 НЕ РЕДАКТИРУЙТЕ ЭТОТ ФАЙЛ. ЭТОТ ФАЙЛ СОЗДАН АВТОМАТИЧЕСКИ.
 ВСЕ ВАШИ ИЗМЕНЕНИЯ БУДУТ ПОТЕРЯНЫ.
*/ 
// --------------------------------------------------------------------------
// generate timestamp: 2015-12-19+03:00
// -----------------------------------------------------------------------------
#ifndef UObject_SK_H_
#define UObject_SK_H_
// -----------------------------------------------------------------------------
#include <memory>
#include <string>
#include <unordered_map>
#include <sstream>
#include "UniSetObject.h"
#include "UniXML.h"
#include "Trigger.h"
#include "DebugStream.h"
#include "LogServer.h"
#include "LogAgregator.h"
#include "VMonitor.h"
// -----------------------------------------------------------------------------
class UObject_SK:
	public UniSetObject
{
	public:
		UObject_SK( UniSetTypes::ObjectId id, xmlNode* node=UniSetTypes::uniset_conf()->getNode("UObject"), const std::string& argprefix="" );
		UObject_SK();
		virtual ~UObject_SK();

		
		long getValue( UniSetTypes::ObjectId sid );
		void setValue( UniSetTypes::ObjectId sid, long value );
		void askSensor( UniSetTypes::ObjectId sid, UniversalIO::UIOCommand, UniSetTypes::ObjectId node = UniSetTypes::uniset_conf()->getLocalNode() );
		void updateValues();

		virtual UniSetTypes::SimpleInfo* getInfo( CORBA::Long userparam = 0 ) override;

		virtual bool setMsg( UniSetTypes::ObjectId code, bool state = true );

		inline std::shared_ptr<DebugStream> log(){ return mylog; }
		inline std::shared_ptr<LogAgregator> logAgregator(){ return loga; }

		void init_dlog( std::shared_ptr<DebugStream> d );

        // "синтаксический сахар"..для логов
        #ifndef myinfo 
        	#define myinfo if( log()->debugging(Debug::INFO) ) log()->info() 
        #endif
        #ifndef mywarn
	        #define mywarn if( log()->debugging(Debug::WARN) ) log()->warn()
        #endif
        #ifndef mycrit
    	    #define mycrit if( log()->debugging(Debug::CRIT) ) log()->crit()
        #endif
        #ifndef mylog1
        	#define mylog1 if( log()->debugging(Debug::LEVEL1) ) log()->level1()
        #endif
        #ifndef mylog2
	        #define mylog2 if( log()->debugging(Debug::LEVEL2) ) log()->level2()
        #endif
        #ifndef mylog3
    	    #define mylog3 if( log()->debugging(Debug::LEVEL3) ) log()->level3()
        #endif
        #ifndef mylog4
        	#define mylog4 if( log()->debugging(Debug::LEVEL4) ) log()->level4()
        #endif
        #ifndef mylog5
	        #define mylog5 if( log()->debugging(Debug::LEVEL5) ) log()->level5()
        #endif
        #ifndef mylog6
    	    #define mylog6 if( log()->debugging(Debug::LEVEL6) ) log()->level6()
        #endif
        #ifndef mylog7
        	#define mylog7 if( log()->debugging(Debug::LEVEL7) ) log()->level7()
        #endif
        #ifndef mylog8
	        #define mylog8 if( log()->debugging(Debug::LEVEL8) ) log()->level8()
        #endif
        #ifndef mylog9
    	    #define mylog9 if( log()->debugging(Debug::LEVEL9) ) log()->level9()
        #endif
        #ifndef mylogany
        	#define mylogany log()->any()
        #endif
        #ifndef vmonit
            #define vmonit( var ) vmon.add( #var, var )
        #endif
        
        // Вспомогательные функции для удобства логирования
        // ------------------------------------------------------------
        /*! вывод в строку значение всех входов и выходов в формате
           ObjectName: 
              in_xxx  = val
              in_xxx2 = val
              out_zzz = val
              ...
        */
        std::string dumpIO();
        
        /*! Вывод в строку названия входа/выхода в формате: in_xxx(SensorName) 
           \param id           - идентификатор датчика
           \param showLinkName - TRUE - выводить SensorName, FALSE - не выводить
        */
        std::string str( UniSetTypes::ObjectId id, bool showLinkName=true );
        
        /*! Вывод значения входа/выхода в формате: in_xxx(SensorName)=val 
           \param id           - идентификатор датчика
           \param showLinkName - TRUE - выводить SensorName, FALSE - не выводить
        */
        std::string strval( UniSetTypes::ObjectId id, bool showLinkName=true );        
        
        /*! Вывод состояния внутренних переменных */
        inline std::string dumpVars(){ return std::move(vmon.pretty_str()); }
        // ------------------------------------------------------------
        std::string help();
        


		// Используемые идентификаторы
		

		// Используемые идентификаторы сообщений
		

		// Текущее значение
		

		// --- public variables ---
		
		
		// --- end of public variables ---

	protected:
		// --- protected variables ---
		
		
		// ---- end of protected variables ----

		
		virtual void callback() override;
		virtual void processingMessage( UniSetTypes::VoidMessage* msg ) override;
		virtual void sysCommand( const UniSetTypes::SystemMessage* sm ){};
		virtual void askSensors( UniversalIO::UIOCommand cmd ){}
		virtual void sensorInfo( const UniSetTypes::SensorMessage* sm ) override{}
		virtual void timerInfo( const UniSetTypes::TimerMessage* tm ) override{}
		virtual void sigterm( int signo ) override;
		virtual bool activateObject() override;
		virtual std::string getMonitInfo(){ return ""; } /*!< пользовательская информация выводимая в getInfo() */
		
		virtual void testMode( bool state );
		void updatePreviousValues();
		void checkSensors();
		void updateOutputs( bool force );

		void preAskSensors( UniversalIO::UIOCommand cmd );
		void preSensorInfo( const UniSetTypes::SensorMessage* sm );
		void preTimerInfo( const UniSetTypes::TimerMessage* tm );
		void preSysCommand( const UniSetTypes::SystemMessage* sm );
		void waitSM( int wait_msec, UniSetTypes::ObjectId testID = UniSetTypes::DefaultObjectId );
		void initFromSM();

		void resetMsg();
		Trigger trResetMsg;
		PassiveTimer ptResetMsg;
		int resetMsgTime;

		// Выполнение очередного шага программы
		virtual void step(){}

		int sleep_msec; /*!< пауза между итерациями */
		bool active;

		const std::string argprefix;
		UniSetTypes::ObjectId smTestID; /*!< идентификатор датчика для тестирования готовности SM */

		// управление датчиком "сердцебиения"
		PassiveTimer ptHeartBeat;				/*! < период "сердцебиения" */
		UniSetTypes::ObjectId idHeartBeat;		/*! < идентификатор датчика (AI) "сердцебиения" */
		int maxHeartBeat;						/*! < сохраняемое значение */
		
		xmlNode* confnode;
		/*! получить числовое свойство из конф. файла по привязанной confnode */
		int getIntProp(const std::string& name) { return UniSetTypes::uniset_conf()->getIntProp(confnode, name); }
		/*! получить текстовое свойство из конф. файла по привязанной confnode */
		inline const std::string getProp(const std::string& name) { return UniSetTypes::uniset_conf()->getProp(confnode, name); }

		timeout_t smReadyTimeout; 	/*!< время ожидания готовности SM */
		std::atomic_bool activated;
		timeout_t activateTimeout;	/*!< время ожидания готовности UniSetObject к работе */
		PassiveTimer ptStartUpTimeout;	/*!< время на блокировку обработки WatchDog, если недавно был StartUp */
		int askPause; /*!< пауза между неудачными попытками заказать датчики */
		
		IOController_i::SensorInfo si;
		bool forceOut; /*!< флаг принудительного обноления "выходов" */
		
		std::shared_ptr<LogAgregator> loga;
		std::shared_ptr<DebugStream> mylog;
		std::shared_ptr<LogServer> logserv;
		std::string logserv_host = {""};
		int logserv_port = {0};

		// snap
		bool no_snap = {false};
		
		VMonitor vmon;

		

	private:
		
		// --- private variables ---
		// --- end of private variables ---

		// предыдущее значение (для работы UpdateValue())
		

		// Используемые идентификаторы сообщений
		

		bool end_private; // вспомогательное поле (для внутреннего использования при генерировании кода)
};

// -----------------------------------------------------------------------------
#endif // UObject_SK_H_
