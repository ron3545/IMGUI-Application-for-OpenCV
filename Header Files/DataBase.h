/*
reffer to this:
https://dev.mysql.com/doc/connector-cpp/1.1/en/connector-cpp-examples-complete-example-1.html

https://stackoverflow.com/questions/46800465/mysql-c-connector-how-to-retrieve-auto-increment-keys-from-insertion-query
*/

#pragma once
#include "mysql/jdbc.h"
#include <tchar.h>
#include "..\Header Files\Log.h"
#include <ostream>
#include "opencv2/opencv.hpp"
#include <array>

/*
	get the auto increment value of mysql:
		https://stackoverflow.com/questions/46800465/mysql-c-connector-how-to-retrieve-auto-increment-keys-from-insertion-query
*/
namespace jdbc
{
	typedef struct tbl_family_info {
		std::string	FirstName, 
					MiddleName, 
					LastName, 
					Relation, 
					Sex,
					Birthday; //YYYY-MM-DD

		tbl_family_info() {}
		tbl_family_info(std::string first_name, std::string middle_name,
						std::string last_name, std::string relation, 
						std::string sex, std::string bday) 
			:	FirstName(first_name), MiddleName(middle_name), LastName(last_name),
				Relation(relation), Sex(sex), Birthday(bday) 
		{}

	}Family;

	typedef struct Account 
	{
		std::string m_ptchHost, m_ptchUser, m_ptchPassword, m_ptchSchema;

		Account() {}
		Account(std::string pthcHost, std::string pthcUser, std::string ptchPassword, std::string ptchSchema)
			:m_ptchHost(pthcHost), m_ptchUser(pthcUser), m_ptchPassword(ptchPassword), m_ptchSchema(ptchSchema)
		{}

		void CleanAll();
		void ClearEntry();
		bool IsEmpty();
	}Account;

	class SQLDataBase
	{
	private:
		sql::Driver*			m_driver;
		sql::Connection*		m_con;
		sql::ResultSet*			m_res;
		sql::PreparedStatement* m_pstmt, *m_pstmt2;
		
		TCHAR* m_Host, * m_UserID, * m_Password, *m_schema;
		
		struct MatInfo
		{
			MatInfo(int m_colmn = -1, int m_rows = -1, int m_type = CV_8U)
				: colmns(m_colmn), rows(m_rows), type(m_type) {}
			int colmns, rows, type;

			size_t Size() { return static_cast<size_t>(CV_ELEM_SIZE(type)) * rows * colmns; }
			size_t RowSize() { return static_cast<size_t>(CV_ELEM_SIZE(type)) * colmns; }
		};

	public:
		SQLDataBase(void);
		SQLDataBase(const SQLDataBase& db);
		SQLDataBase(TCHAR* ptchHost, TCHAR* ptchUser, TCHAR* ptchDB, TCHAR* ptchPassword);
		~SQLDataBase();

		sql::Driver* GetDriver()	const	{ return m_driver; }
		sql::Connection* GetConn()	const	{ return m_con; }
		sql::Statement* GetStmt()	const	{ return m_pstmt; }
		sql::ResultSet* GetResult() const	{ return m_res; }

		TCHAR* GetHost()	const { return m_Host; }
		TCHAR* GetUserID()	const { return m_UserID; }
		TCHAR* GetPass()	const { return m_Password; }
		TCHAR* GetSchema()	const { return m_schema; }

		void SetAccount(const Account& acc);
		void SetHost(const TCHAR* host);
		void SetUser(const TCHAR* user);
		void SetPassword(const TCHAR* pass);
		void SetSchema(const TCHAR* schema);

		bool Connect() throw();
		bool Connect(const TCHAR* ptchHost, const TCHAR* ptchUser, const TCHAR* ptchPassword, const TCHAR* ptchSchema) throw();
		bool Disconnect() throw();

		// tbl_family_info
		bool InsertFamily(std::vector<std::stringstream>& data, const Family& query) noexcept;
		bool InsertFamily(std::vector<std::stringstream>& data,  const std::string& FirstName, const std::string& MiddleName, const std::string& LastName, const std::string& Sex, const std::string& Relation, const std::string& Birthday);
		Family GetInfo() noexcept; 

	private:
		bool MatSerialize(std::ostream& out, const cv::Mat& mat);
		bool MatDeserialize(std::istream& in, cv::Mat& out);
	};
}

/*
Sample Query:
	https://dev.mysql.com/doc/connector-cpp/1.1/en/connector-cpp-examples-query.html

Lesson:
	
	https://stackoverflow.com/questions/27480741/which-execute-function-should-i-use-in-mysql-connector-c
	execute
		This function is the most generic one. It returns a boolean value, which value is true if the query returns multiple results, or false if the query returns either nothing or an update count.

		This is the function you'll want to use if you only want to use one to be as generic as possible.

		If it returns true, you'll want to use ResultSet * getResultSet() to get the results.
		If it returns false, you'll want to use uint64_t getUpdateCount() to get the number of updated rows.

	executeQuery
		This function directly returns a ResultSet which is useful for SELECT statements, and assumes there is indeed a result set to be returned.

		It is equivalent to call execute() followed by getResultSet().

		You'll want to use this function when you know you are using SQL code that returns results such as rows.

	executeUpdate
		This function returns an integer value which is useful for UPDATE statements and assumes there is an update count to be returned.

		It is equivalent to call execute() followed by getUpdateCount(), even though, for some reason, the return types are different (int vs uint64_t).

		This is the function to use when executing SQL statements that modify data and you need to know whether some data was modified.
*/