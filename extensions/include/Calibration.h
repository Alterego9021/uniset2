#ifndef Calibration_H_
#define Calibration_H_
// -----------------------------------------------------------------------------
#include <cmath>
#include <string>
#include <list>
#include <ostream>
// -----------------------------------------------------------------------------
/*!
	����� ����������� ��������� ������������� 
�������������� �� ����������������� ����� 
� �������� �� ��� �����.
\code
C, ������������� ��������
  ^
  |
  |
  |
  |
   -------->
           R(raw value), ����� ��������
\endcode
*/
class Calibration
{
	public:
		Calibration();
		Calibration( const std::string name, const std::string confile="calibration.xml" );
		Calibration( xmlNode* node );
		~Calibration();

		/*! ����� �� ������� ��������� */
		static const int outOfRange=-1;

		/*!
			��������� �������������� ��������
			\param raw - ����� ��������
			\return ���������� �������������
		*/
		long getValue( long raw );


		/*!
			��������� ������ �������� �� ��������������
		*/
		long getRawValue( long cal );


		/*! ���������� ��������������� �� ����. ����� 
			\param name - �������� �������������� � �����
			\param confile - ���� ���������� ������
			\param node	- ���� node!=0, �� ������������ ���� ����...
		*/
		void build( const std::string name, const std::string confile, xmlNode* node=0  );


		/*! ��� ��� �������� �������� �������� */
		typedef float TypeOfValue;

		/*! �������������� ���� ��� �������� 
			� ��� ��� ���������� ��������
		 */
		inline long tRound( const TypeOfValue& val )
		{
			return lround(val);
		}

		friend std::ostream& operator<<(std::ostream& os, Calibration& c );
		friend std::ostream& operator<<(std::ostream& os, Calibration* c );

	protected:

		/*!	����� �������������� */
		struct Point
		{
			Point( TypeOfValue _x, TypeOfValue _y ):
				x(_x),y(_y){}

			TypeOfValue x;
			TypeOfValue y;

	   		inline bool operator < ( const Point& p ) const
			{
				return ( x < p.x );
			}
		};

		/*! ������� �������������� */
		class Part
		{
			public:
				Part( Point& pleft, Point& pright );
				~Part(){};

				/*!	��������� �� ����� �� ������ ������� */
				bool check( Point& p );

				/*!	��������� �� ����� �� ������ ������� �� X */
				bool checkX( TypeOfValue x );

				/*!	��������� �� ����� �� ������ ������� �� Y */
				bool checkY( TypeOfValue y );
				
				// ������� ����� ������� OutOfRange
				TypeOfValue getY( TypeOfValue x ); 		/*!< �������� �������� Y */
				TypeOfValue getX( TypeOfValue y );		/*!< �������� �������� X */

				TypeOfValue calcY( TypeOfValue x ); 	/*!< ��������� �������� ��� x */
				TypeOfValue calcX( TypeOfValue y ); 	/*!< ��������� �������� ��� y */
				
		   		inline bool operator < ( const Part& p ) const
				{
					return (p_right < p.p_right);
				}
			
				inline Point leftPoint(){ return p_left; }
				inline Point rightPoint(){ return p_right; }
				inline TypeOfValue getK(){ return k; } 	/*!< �������� ����������� ������� */
				inline TypeOfValue left_x(){ return p_left.x; }
				inline TypeOfValue left_y(){ return p_left.y; }
				inline TypeOfValue right_x(){ return p_right.x; }
				inline TypeOfValue right_y(){ return p_right.y; }

			protected:
				Point p_left; 	/*!< ����� ������ ������� */
				Point p_right; 	/*!< ������ ������ ������� */
				TypeOfValue k; 	/*!< ����������� ������� */
		};
		
		// ������ ���� ������������� �� x!
		typedef std::list<Part> PartsList;

	private:
		PartsList plist;
		std::string myname;
};
// -----------------------------------------------------------------------------
#endif // Calibration_H_
// -----------------------------------------------------------------------------
