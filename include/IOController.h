/* This file is part of the UniSet project
 * Copyright (c) 2002 Free Software Foundation, Inc.
 * Copyright (c) 2002 Pavel Vainerman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
// --------------------------------------------------------------------------
/*! \file
 * \brief Реализация IOController_i
 * \author Pavel Vainerman
*/
// --------------------------------------------------------------------------
#ifndef IOController_H_
#define IOController_H_
//---------------------------------------------------------------------------
#include <unordered_map>
#include <list>
#include <sigc++/sigc++.h>
#include "IOController_i.hh"
#include "UniSetTypes.h"
#include "UniSetManager.h"
#include "Configuration.h"
#include "Mutex.h"
//---------------------------------------------------------------------------
/*! Реализация интерфейса IOController-а */
class IOController:
	public UniSetManager,
	public POA_IOController_i
{
	public:

		IOController( const std::string& name, const std::string& section );
		IOController( const UniSetTypes::ObjectId id );
		virtual ~IOController();

		virtual UniSetTypes::ObjectType getType() override
		{
			return UniSetTypes::ObjectType("IOController");
		}

		virtual UniSetTypes::SimpleInfo* getInfo( ::CORBA::Long userparam = 0 ) override;

		virtual CORBA::Long getValue( UniSetTypes::ObjectId sid ) override;

		//     -------------------- !!!!!!!!! ---------------------------------
		//        Реализуются конкретным i/o контроллером
		//        Не забывайте писать реализацию этих функций
		virtual void setValue( UniSetTypes::ObjectId sid, CORBA::Long value,
							   UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId ) override;

		virtual void fastSetValue( UniSetTypes::ObjectId sid, CORBA::Long value,
								   UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId ) override;

		//     ----------------------------------------------------------------
		virtual void setUndefinedState( UniSetTypes::ObjectId sid,
										CORBA::Boolean undefined,
										UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId ) override;


		virtual IOController_i::SensorInfoSeq* getSensorSeq( const UniSetTypes::IDSeq& lst ) override;
		virtual UniSetTypes::IDSeq* setOutputSeq( const IOController_i::OutSeq& lst, UniSetTypes::ObjectId sup_id ) override;

		//     ----------------------------------------------------------------
		virtual UniversalIO::IOType getIOType( UniSetTypes::ObjectId sid ) override;

		virtual IOController_i::SensorInfoSeq* getSensorsMap() override;
		virtual IOController_i::SensorIOInfo getSensorIOInfo( UniSetTypes::ObjectId sid ) override;

		virtual CORBA::Long getRawValue( UniSetTypes::ObjectId sid ) override;
		virtual void calibrate( UniSetTypes::ObjectId sid,
								const IOController_i::CalibrateInfo& ci,
								UniSetTypes::ObjectId adminId ) override;

		IOController_i::CalibrateInfo getCalibrateInfo( UniSetTypes::ObjectId sid ) override;

		inline IOController_i::SensorInfo SensorInfo( const UniSetTypes::ObjectId sid,
				const UniSetTypes::ObjectId node = UniSetTypes::uniset_conf()->getLocalNode())
		{
			IOController_i::SensorInfo si;
			si.id = sid;
			si.node = node;
			return si;
		};

		UniSetTypes::Message::Priority getPriority( const UniSetTypes::ObjectId id );

		virtual IOController_i::ShortIOInfo getChangedTime( const UniSetTypes::ObjectId id ) override;

		virtual IOController_i::ShortMapSeq* getSensors() override;

	public:

		// предварительное объявление..
		struct USensorInfo;
		typedef std::unordered_map<UniSetTypes::ObjectId, std::shared_ptr<USensorInfo>> IOStateList;

		// ================== Достпуные сигналы =================
		/*!
		// \warning  В сигнале напрямую передаётся указатель на внутреннюю структуру!
		// Это не очень хорошо, с точки зрения "архитектуры", но оптимальнее по быстродействию!
		// необходимо в обработчике не забывать использовать uniset_rwmutex_wrlock(val_lock) или uniset_rwmutex_rlock(val_lock)
		*/
		typedef sigc::signal<void, std::shared_ptr<USensorInfo>&, IOController*> ChangeSignal;
		typedef sigc::signal<void, std::shared_ptr<USensorInfo>&, IOController*> ChangeUndefinedStateSignal;

		// signal по изменению определённого датчика
		ChangeSignal signal_change_value( UniSetTypes::ObjectId sid );

		// signal по изменению любого датчика
		ChangeSignal signal_change_value();

		// сигналы по изменению флага "неопределённое состояние" (обрыв датчика например)
		ChangeUndefinedStateSignal signal_change_undefined_state( UniSetTypes::ObjectId sid );
		ChangeUndefinedStateSignal signal_change_undefined_state();
		// -----------------------------------------------------------------------------------------
		inline IOStateList::iterator ioBegin()
		{
			return ioList.begin();
		}
		inline IOStateList::iterator ioEnd()
		{
			return ioList.end();
		}
		inline IOStateList::iterator find(UniSetTypes::KeyType k)
		{
			return ioList.find(k);
		}
		inline int ioCount()
		{
			return ioList.size();
		}

		// доступ к элементам через итератор
		virtual void localSetValue( IOStateList::iterator& it, const UniSetTypes::ObjectId sid,
									CORBA::Long value, UniSetTypes::ObjectId sup_id );

		virtual long localGetValue( IOStateList::iterator& it, const UniSetTypes::ObjectId sid );

		// вариант с указателем
		void localSetValue( std::shared_ptr<USensorInfo>& usi, UniSetTypes::ObjectId sid,
							CORBA::Long value, UniSetTypes::ObjectId sup_id );

		long localGetValue( std::shared_ptr<USensorInfo>& it, const UniSetTypes::ObjectId sid );


		/*! функция выставления признака неопределённого состояния для аналоговых датчиков
		    // для дискретных датчиков необходимости для подобной функции нет.
		    // см. логику выставления в функции localSaveState
		*/
		virtual void localSetUndefinedState( IOStateList::iterator& it, bool undefined,
											 const UniSetTypes::ObjectId sid );

	protected:
		// переопределяем для добавления вызова регистрации датчиков
		virtual bool deactivateObject() override;
		virtual bool activateObject() override;

		/*! Начальная инициализация (выставление значений) */
		virtual void activateInit();

		/*! регистрация датчиков, за информацию о которых отвечает данный IOController */
		virtual void sensorsRegistration() {};
		/*! удаление из репозитория датчиков за информацию о которых отвечает данный IOController */
		virtual void sensorsUnregistration();

		typedef sigc::signal<void, IOStateList::iterator&, IOController*> InitSignal;
		// signal по изменению определённого датчика
		inline InitSignal signal_init()
		{
			return sigInit;
		}

		/*! регистрация датчика
		    force=true - не проверять на дублирование (оптимизация)
		*/
		void ioRegistration( std::shared_ptr<USensorInfo>&, bool force = false );

		/*! разрегистрация датчика */
		void ioUnRegistration( const UniSetTypes::ObjectId sid );

		// ------------------------------
		inline IOController_i::SensorIOInfo
		SensorIOInfo(long v, UniversalIO::IOType t, const IOController_i::SensorInfo& si,
					 UniSetTypes::Message::Priority p = UniSetTypes::Message::Medium,
					 long defval = 0, IOController_i::CalibrateInfo* ci = 0,
					 UniSetTypes::ObjectId sup_id = UniSetTypes::DefaultObjectId )
		{
			IOController_i::SensorIOInfo ai;
			ai.si = si;
			ai.type = t;
			ai.value = v;
			ai.priority = p;
			ai.default_val = defval;
			ai.real_value = v;
			ai.blocked = false;
			ai.supplier = sup_id;

			if( ci != 0 )
				ai.ci = *ci;
			else
			{
				ai.ci.minRaw = 0;
				ai.ci.maxRaw = 0;
				ai.ci.minCal = 0;
				ai.ci.maxCal = 0;
				ai.ci.precision = 0;
			}

			return ai;
		};

		//! сохранение информации об изменении состояния датчика
		virtual void logging( UniSetTypes::SensorMessage& sm );

		//! сохранение состояния всех датчиков в БД
		virtual void dumpToDB();

		IOController();

		// доступ к списку c изменением только для своих
		IOStateList::iterator myioBegin();
		IOStateList::iterator myioEnd();
		IOStateList::iterator myiofind( UniSetTypes::ObjectId id );
		// --------------------------
		// ФИЛЬТРОВАНИЕ
		//
		typedef sigc::slot<bool, std::shared_ptr<USensorInfo>&, CORBA::Long, UniSetTypes::ObjectId> IOFilterSlot;
		typedef std::list<IOFilterSlot> IOFilterSlotList;

		/*
		    Фильтрующая функция должна возвращать:
		    TRUE - если значение 'нормальное'
		    FALSE - если значение не подходит (отбрасывается)

		    Пример использования:
		        addIOFilter( sigc::mem_fun(my,&MyClass::my_filter) );
		*/
		IOFilterSlotList::iterator addIOFilter( IOFilterSlot sl, bool push_front = false );
		void eraseIOFilter(IOFilterSlotList::iterator& it);

		// функии проверки текущего значения
		bool checkIOFilters( std::shared_ptr<USensorInfo>& ai, CORBA::Long& newvalue, UniSetTypes::ObjectId sup_id );

		inline bool iofiltersEmpty()
		{
			return iofilters.empty();
		}
		inline int iodiltersSize()
		{
			return iofilters.size();
		}

	private:
		friend class NCRestorer;

		UniSetTypes::uniset_mutex siganyMutex;
		ChangeSignal sigAnyChange;

		UniSetTypes::uniset_mutex siganyundefMutex;
		ChangeSignal sigAnyUndefChange;
		InitSignal sigInit;

		IOStateList ioList;    /*!< список с текущим состоянием аналоговых входов/выходов */
		UniSetTypes::uniset_rwmutex ioMutex; /*!< замок для блокирования совместного доступа к ioList */

		bool isPingDBServer;    // флаг связи с DBServer-ом

		IOFilterSlotList iofilters; /*!< список фильтров для аналоговых значений */

		UniSetTypes::uniset_rwmutex loggingMutex; /*!< logging info mutex */

	public:
		struct USensorInfo:
			public IOController_i::SensorIOInfo
		{
			USensorInfo( const USensorInfo& ) = delete;
			const USensorInfo& operator=(const USensorInfo& ) = delete;
			USensorInfo( USensorInfo&& ) = default;
			USensorInfo& operator=(USensorInfo&& ) = default;

			USensorInfo(): any(0), d_value(0), d_off_value(0)
			{
				d_si.id = UniSetTypes::DefaultObjectId;
				d_si.node = UniSetTypes::DefaultObjectId;
				default_val = 0;
				value = default_val;
				real_value = default_val;
				dbignore = false;
				undefined = false;
				blocked = false;
				supplier = UniSetTypes::DefaultObjectId;
			}

			virtual ~USensorInfo() {}

			USensorInfo(IOController_i::SensorIOInfo& r);
			USensorInfo(IOController_i::SensorIOInfo* r);
			USensorInfo(const IOController_i::SensorIOInfo& r);

			USensorInfo& operator=(IOController_i::SensorIOInfo& r);
			const USensorInfo& operator=(const IOController_i::SensorIOInfo& r);
			USensorInfo& operator=(IOController_i::SensorIOInfo* r);

			// Дополнительные (вспомогательные поля)
			UniSetTypes::uniset_rwmutex val_lock; /*!< флаг блокирующий работу со значением */

			// IOStateList::iterator it;
			std::shared_ptr<USensorInfo> it;

			void* any; /*!< расширение для возможности хранения своей информации */

			// сигнал для реализации механизма зависимостией..
			// (все зависимые датчики подключаются к нему (см. NCRestorer::init_depends_signals)
			UniSetTypes::uniset_rwmutex changeMutex;
			ChangeSignal sigChange;

			UniSetTypes::uniset_rwmutex undefMutex;
			ChangeUndefinedStateSignal sigUndefChange;

			IOController_i::SensorInfo d_si;  /*!< идентификатор датчика, от которого зависит данный */
			long d_value; /*!< разрешающее работу значение датчика от которого зависит данный */
			long d_off_value; /*!< блокирующее значение */

			// функция обработки информации об изменении состояния датчика, от которого зависит данный
			void checkDepend( std::shared_ptr<USensorInfo>& it, IOController* );

			void init( const IOController_i::SensorIOInfo& s );

			inline IOController_i::SensorIOInfo makeSensorIOInfo()
			{
				UniSetTypes::uniset_rwmutex_rlock lock(val_lock);
				IOController_i::SensorIOInfo s(*this);
				return std::move(s);
			}

			inline UniSetTypes::SensorMessage makeSensorMessage()
			{
				UniSetTypes::SensorMessage sm;

				UniSetTypes::uniset_rwmutex_rlock lock(val_lock);
				sm.id           = si.id;
				sm.node         = si.node; // uniset_conf()->getLocalNode()?
				sm.sensor_type  = type;
				sm.value        = value;
				sm.undefined    = undefined;
				sm.priority     = (UniSetTypes::Message::Priority)priority;
				sm.sm_tv_sec    = tv_sec;
				sm.sm_tv_usec   = tv_usec;
				sm.ci           = ci;
				sm.supplier     = supplier;
				return std::move(sm);
			}
		};
};
// --------------------------------------------------------------------------
#endif
// --------------------------------------------------------------------------
