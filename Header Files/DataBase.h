/*
reffer to this:
https://dev.mysql.com/doc/connector-cpp/1.1/en/connector-cpp-examples-complete-example-1.html

https://stackoverflow.com/questions/46800465/mysql-c-connector-how-to-retrieve-auto-increment-keys-from-insertion-query
*/

//C:\Program Files\MySQL\Connector C++ 8.0\include

#pragma once
#pragma warning (disable : 26451)
#pragma warning(disable : 4996)

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE  
#define _CRT_NONSTDC_NO_DEPRECATE

#include "mysql/jdbc.h"
#include <tchar.h>
#include "..\Header Files\Log.h"
#include <ostream>
#include "opencv2/opencv.hpp"
#include <array>
#include <time.h>
#include <map>
/*
	get the auto increment value of mysql:
		https://stackoverflow.com/questions/46800465/mysql-c-connector-how-to-retrieve-auto-increment-keys-from-insertion-query
*/

namespace jdbc
{
	struct RegistryInfo
	{
		//data
		int person_id;
		std::string FirstName, MiddleName, LastName, Relation, Sex, Birthday;

		RegistryInfo() : person_id(0){}
		std::string Get_FullName() const 
		{ 
			return FirstName + " " + MiddleName + " " + LastName;
		}
		
		void SetData(int person_id, std::string FirstName, std::string MiddleName, std::string LastName, 
			std::string Relation, std::string Sex, std::string Birthday)
		{
			this->person_id		= person_id;
			this->FirstName		= FirstName;
			this->MiddleName	= MiddleName;
			this->LastName		= LastName;
			this->Relation		= Relation;
			this->Sex			= Sex;
			this->Birthday		= Birthday;
		}

		int GetAge(int index) const
		{
			//date format passed into BirthDay sample: tuesday, 16 January 2022
			int		Year = 0, Day = 0;
			char*	month = nullptr;
			char*	day_of_the_month = nullptr;

			if (sscanf(Birthday.c_str(), "%s %d %s %d", day_of_the_month, &Day, month, &Year) == NULL)
				return 0;

			time_t      now = time(NULL);
			struct tm   tstruct;
			localtime_s(&tstruct, &now);
			
			return tstruct.tm_year - Year;
		}
	};

	struct RegistryImages
	{
		int image_id;
		int person_id;
		std::vector<cv::Mat> images;

		void Set(int image_id, int person_id, const std::vector<cv::Mat>& images)
		{
			this->image_id	= image_id;
			this->person_id = person_id;
			this->images	= images;
		}
		void Clear()
		{
			this->images.clear();
			this->image_id = 0;
			this->person_id = 0;
		}
	};

	struct Account 
	{
		std::string m_ptchHost, m_ptchUser, m_ptchPassword, m_ptchSchema;

		Account() {}
		Account(std::string pthcHost, std::string pthcUser, std::string ptchPassword, std::string ptchSchema)
			:m_ptchHost(pthcHost), m_ptchUser(pthcUser), m_ptchPassword(ptchPassword), m_ptchSchema(ptchSchema)
		{}

		void CleanAll();
		void ClearEntry();
		bool IsEmpty();
	};

	class SQLDataBase
	{
	private:
		sql::Driver*			m_driver;
		sql::Connection*		m_con;
		
		TCHAR* m_Host = nullptr, * m_UserID = nullptr, * m_Password = nullptr, *m_schema = nullptr;
		
		struct MatInfo
		{
			MatInfo(int m_colmn = -1, int m_rows = -1, int m_type = CV_8U)
				: colmns(m_colmn), rows(m_rows), type(m_type) {}
			int colmns, rows, type;

			size_t Size()		{ return CV_ELEM_SIZE(type) * rows * colmns;	}
			size_t RowSize()	{ return CV_ELEM_SIZE(type) * colmns;			}
		};

	public:
		SQLDataBase(void);
		SQLDataBase(TCHAR* ptchHost, TCHAR* ptchUser, TCHAR* ptchDB, TCHAR* ptchPassword);
		~SQLDataBase();

		sql::Driver* GetDriver()	const	{ return m_driver;	}
		sql::Connection* GetConn()	const	{ return m_con;		}

		TCHAR* GetHost()	const { return m_Host;		}
		TCHAR* GetUserID()	const { return m_UserID;	}
		TCHAR* GetPass()	const { return m_Password;	}
		TCHAR* GetSchema()	const { return m_schema;	}

		void SetAccount		(const Account& acc);
		void SetHost		(const TCHAR* host);
		void SetUser		(const TCHAR* user);
		void SetPassword	(const TCHAR* pass);
		void SetSchema		(const TCHAR* schema);

		bool Connect() throw();
		bool Connect(const TCHAR* ptchHost, const TCHAR* ptchUser, const TCHAR* ptchPassword, const TCHAR* ptchSchema) throw();
		bool Disconnect() throw();

		bool InsertFamily (const std::vector<cv::Mat>& images,  const std::string& FirstName, const std::string& MiddleName, const std::string& LastName, const std::string& Sex, const std::string& Relation, const std::string& Birthday);
		void GetInfo (std::map<std::string, RegistryInfo>& person_info, std::map <std::string, RegistryImages>& person_images) noexcept;

		bool Is_TableEmpty(const char* tbl_name);
		bool InsertunknownPerson();
	private:
		SQLDataBase(const SQLDataBase&);				//prevent copyingin other object of this class
		SQLDataBase& operator=(const SQLDataBase&);		//Same din sa part na toh. para pag ginawa ng user ito ay mag produce ng error during link-time

		bool MatSerialize(std::ostream& out, const cv::Mat& mat);
		bool MatDeserialize(std::istream& in, cv::Mat& out);
	};
}

