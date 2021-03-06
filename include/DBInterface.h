#ifndef DBInterface_H_
#define DBInterface_H_
// --------------------------------------------------------------------------
#include <string>
#include <deque>
#include <vector>
#include "UniSetTypes.h"
// --------------------------------------------------------------------------
/*! Асбстрактный класс для доступа к БД
*/
class DBResult;

class DBInterface
{
	public:

		DBInterface() {};
		virtual ~DBInterface() {};

		// Функция подключения к БД, параметры подключения зависят от типа БД
		virtual bool connect( const std::string& param ) = 0;
		virtual bool close() = 0;
		virtual bool isConnection() = 0;
		virtual bool ping() = 0; // проверка доступности БД

		virtual DBResult query( const std::string& q ) = 0;
		virtual const std::string lastQuery() = 0;
		virtual bool insert( const std::string& q ) = 0;
		virtual double insert_id() = 0;
		virtual const std::string error() = 0;
};
// ----------------------------------------------------------------------------------
class DBNetInterface : public DBInterface
{
	public:

		DBNetInterface() {};
		virtual ~DBNetInterface() {};

		// Для сетевых БД параметры должны быть в формате user@host:pswd:dbname
		virtual bool connect( const std::string& param );
		virtual bool nconnect( const std::string& host, const std::string& user, const std::string& pswd, const std::string& dbname ) = 0;
};
// ----------------------------------------------------------------------------------
class DBResult
{
	public:

		DBResult() {}
		virtual ~DBResult() {};

		typedef std::vector<std::string> COL;
		typedef std::deque<COL> ROW;
		typedef ROW::iterator iterator;

		ROW& row();
		iterator begin();
		iterator end();
		operator bool();
		size_t size();
		bool empty();

		// ----------------------------------------------------------------------------
		// ROW
		static int as_int( const DBResult::iterator& it, int col );
		static double as_double( const DBResult::iterator& it, int col );
		static std::string as_string( const DBResult::iterator& it, int col );
		// ----------------------------------------------------------------------------
		// COL
		static int as_int( const DBResult::COL::iterator& it );
		static double as_double(const  DBResult::COL::iterator& it );
		static std::string as_string( const DBResult::COL::iterator& it );
		static int num_cols( const DBResult::iterator& it );
		// ----------------------------------------------------------------------------

	protected:

		ROW row_;
};
// the types of the class factories
typedef DBInterface* create_dbinterface_t();
typedef void destroy_dbinterface_t(DBInterface*);
// --------------------------------------------------------------------------
#endif // DBInterface_H_
// --------------------------------------------------------------------------
