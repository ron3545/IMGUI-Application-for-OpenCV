#include "DataBase.h"

using namespace CPlusPlusLogging;
using namespace std;
using namespace sql;

namespace jdbc
{
	void Account::CleanAll()
	{
		m_ptchHost.clear();
		m_ptchPassword.clear();
		m_ptchSchema.clear();
		m_ptchUser.clear();
	}

	void Account::ClearEntry()
	{
		m_ptchPassword.clear();
		m_ptchUser.clear();
	}

	bool Account::IsEmpty()
	{
		if (m_ptchHost.empty() || m_ptchPassword.empty() || m_ptchSchema.empty() || m_ptchUser.empty())
			return true;
		return false;
	}

	SQLDataBase::SQLDataBase(void) :	m_driver(NULL), m_con(NULL),
										m_pstmt(NULL), m_res(NULL),
										m_Host(NULL), m_UserID(NULL),
										m_Password(NULL), m_schema(NULL),
										m_pstmt2(NULL)
	{}

	SQLDataBase::SQLDataBase(const SQLDataBase& db) :	m_driver(NULL), m_con(NULL),
														m_pstmt(NULL), m_res(NULL), m_pstmt2(NULL),
														m_Host(new TCHAR[_tcslen(db.m_Host) + 1]),
														m_UserID(new TCHAR[_tcslen(db.m_UserID) + 1]),
														m_Password(new TCHAR[_tcslen(db.m_Password) + 1]),
														m_schema(new TCHAR[_tcslen(db.m_schema) + 1])
	{
		_tcscpy_s(m_Host, _tcslen(db.m_Host) + 1, db.m_Host);
		_tcscpy_s(m_UserID, _tcslen(db.m_UserID) + 1, db.m_UserID);
		_tcscpy_s(m_schema, _tcslen(db.m_schema) + 1, db.m_schema);
		_tcscpy_s(m_Password, _tcslen(db.m_Password) + 1, db.m_Password);
	}

	SQLDataBase::SQLDataBase(TCHAR* ptchHost, TCHAR* ptchUser,
		TCHAR* ptchDB, TCHAR* ptchPassword) : m_driver(NULL), m_con(NULL),
		m_pstmt(NULL), m_res(NULL), m_pstmt2(NULL),
		m_Host(new TCHAR[_tcslen(ptchHost) + 1]),
		m_UserID(new TCHAR[_tcslen(ptchUser) + 1]),
		m_Password(new TCHAR[_tcslen(ptchPassword) + 1]),
		m_schema(new TCHAR[_tcslen(ptchDB) + 1])
	{
		_tcscpy_s(m_Host, _tcslen(ptchHost) + 1, ptchHost);
		_tcscpy_s(m_UserID, _tcslen(ptchUser) + 1, ptchUser);
		_tcscpy_s(m_schema, _tcslen(ptchDB) + 1, ptchDB);
		_tcscpy_s(m_Password, _tcslen(ptchPassword) + 1, ptchPassword);
	}

	void SQLDataBase::SetAccount(const Account& acc)
	{
		const char* Host = acc.m_ptchHost.c_str();
		const char* User = acc.m_ptchUser.c_str();
		const char* Pass = acc.m_ptchPassword.c_str();
		const char* Schema = acc.m_ptchSchema.c_str();

		SetHost(Host);
		SetUser(User);
		SetPassword(Pass);
		SetSchema(Schema);
	}

	void SQLDataBase::SetHost(const TCHAR* host)
	{
		if (m_Host != NULL) {
			delete[] m_Host;
			m_Host = NULL;

			m_Host = new TCHAR[_tcslen(host) + 1];
			_tcscpy_s(m_Host, _tcslen(host) + 1, host);
		}
		else {
			m_Host = new TCHAR[_tcslen(host) + 1];
			_tcscpy_s(m_Host, _tcslen(host) + 1, host);
		}
	}


	void SQLDataBase::SetUser(const TCHAR* user)
	{
		if (m_UserID != NULL) {
			delete[] m_UserID;
			m_UserID = NULL;

			m_UserID = new TCHAR[_tcslen(user) + 1];
			_tcscpy_s(m_UserID, _tcslen(user) + 1, user);
		}
		else {
			m_UserID = new TCHAR[_tcslen(user) + 1];
			_tcscpy_s(m_UserID, _tcslen(user) + 1, user);
		}
	}

	void SQLDataBase::SetPassword(const TCHAR* pass)
	{
		if (m_Password != NULL) {
			delete[] m_Password;
			m_Password = NULL;

			m_Password = new TCHAR[_tcslen(pass) + 1];
			_tcscpy_s(m_Password, _tcslen(pass) + 1, pass);
		}
		else {
			m_Password = new TCHAR[_tcslen(pass) + 1];
			_tcscpy_s(m_Password, _tcslen(pass) + 1, pass);
		}
	}

	void SQLDataBase::SetSchema(const TCHAR* schema)
	{
		if (m_schema != NULL) {
			delete[] m_schema;
			m_schema = NULL;

			m_schema = new TCHAR[_tcslen(schema) + 1];
			_tcscpy_s(m_schema, _tcslen(schema) + 1, schema);
		}
		else {
			m_schema = new TCHAR[_tcslen(schema) + 1];
			_tcscpy_s(m_schema, _tcslen(schema) + 1, schema);
		}
	}

	SQLDataBase::~SQLDataBase()
	{
		delete this->m_pstmt;
		delete this->m_pstmt2;
		delete this->m_res;
		delete this->m_con;

		delete[] m_Host;
		m_Host = NULL;

		delete[] m_UserID;
		m_UserID = NULL;

		delete[] m_Password;
		m_Password = NULL;

		delete[] m_schema;
		m_schema = NULL;

	}

	bool SQLDataBase::Connect(void) throw()
	{
		static unsigned int m_count = 0; //counts the number, connected variable was tested to see if it reconects
		bool connected = m_con != NULL && (m_con->isValid() || m_con->reconnect());
		try {
			m_driver = get_driver_instance();
			if (!connected) {
				m_con = m_driver->connect(m_Host, m_UserID, m_Password);
				connected = m_con->isValid();
				m_con->setSchema(m_schema);
				m_count++;

				if (m_count > 1 && m_con->isValid())
					Logger::getInstance()->info("Connection has been restored");
				else 
					Logger::getInstance()->info("\nConnection is alive\n");
			}
			else connected = false;
		}
		catch (SQLException& e)
		{
	
			ostringstream msg;
			msg << " ERROR: " << e.what() << ";" << "(" << __FUNCTION__ << ") on line "
				<< __LINE__ << endl;
			msg << " (MySQL error code: " << e.getErrorCode();
			msg << ", SQLState: " << e.getSQLState() << ")" << endl;
			Logger::getInstance()->error(msg);

			delete this->m_pstmt;
			delete this->m_res;

			this->m_con->close();
			delete this->m_con;
			this->m_driver->threadEnd();

			connected = false;
		}
		return connected;
	}

	bool SQLDataBase::Connect(const TCHAR* ptchHost, const TCHAR* ptchUser, const TCHAR* ptchPassword, const TCHAR* ptchSchema) throw()
	{
		static unsigned int m_count = 0; //counts the number, connected variable was tested to see if it reconects
		bool connected = m_con != NULL && (m_con->isValid() || m_con->reconnect());
		try {
			m_driver = get_driver_instance();
			if (!connected) {
				m_con = m_driver->connect(ptchHost, ptchUser, ptchPassword);
				connected = m_con->isValid();
				m_con->setSchema(ptchSchema);
				m_count++;
				Logger::getInstance()->info("\nConnection is alive\n");

				if (m_count > 1 && m_con->isValid())
					Logger::getInstance()->info("Connection has been restored");
			}
			else connected = false;
		}
		catch (SQLException& e)
		{
			ostringstream msg;
			msg << " ERROR: " << e.what() << ";" << "(" << __FUNCTION__ << ") on line "
				<< __LINE__ << endl;
			msg << " (MySQL error code: " << e.getErrorCode();
			msg << ", SQLState: " << e.getSQLState() << ")" << endl;
			Logger::getInstance()->error(msg);

			delete this->m_pstmt;
			delete this->m_res;

			this->m_con->close();
			delete this->m_con;
			this->m_driver->threadEnd();

			connected = false;
		}
		return connected;
	}

	bool SQLDataBase::Disconnect() throw()
	{
		try {
			delete this->m_pstmt;
			delete this->m_pstmt2;
			delete this->m_res;

			this->m_con->close();
			delete this->m_con;
			this->m_driver->threadEnd();
			return true;
		}
		catch (SQLException& e) {
			ostringstream msg;
			msg << "Failed to close connection. \n";
			msg << " \tERROR: " << e.what();
			msg << " \t(MySQL error code: " << e.getErrorCode() << ",";
			msg << " \tSQLState: " << e.getSQLState() << ")" << endl;
			Logger::getInstance()->error(msg);
			return false;
		}
	}


#pragma region tbl_face_info
	bool SQLDataBase::InsertFamily(std::vector<std::stringstream>& data,  const Family& query) noexcept
	{
		sql::Statement* m_stmt = NULL;
		try 
		{
			m_stmt = m_con->createStatement();
			m_pstmt = m_con->prepareStatement("INSERT INTO tbl_family_info(FirstName, LastName, MiddleName, Sex, Position, Birthday) VALUES (?, ?, ?, ?, ?, ?)");
			m_pstmt->setString(1, query.FirstName);
			m_pstmt->setString(2, query.LastName);
			m_pstmt->setString(3, query.MiddleName);
			m_pstmt->setString(4, query.Sex);
			m_pstmt->setString(5, query.Relation);
			m_pstmt->setDateTime(6, query.Birthday);
			
			m_pstmt->execute();

			ostringstream info;
			info << "Number of row(s) affected:" << m_pstmt->getUpdateCount() << endl;
			Logger::getInstance()->always("Affected Rows");
			Logger::getInstance()->info(info);

			delete m_stmt;
			delete this->m_pstmt;
			delete this->m_pstmt2;
			return true;
		}
		catch (SQLException& e) {
			std::ostringstream err;
			err << "# ERR: SQLException in " << __FILE__;
			err << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			err << "# ERR: " << e.what();
			err << " (MySQL error code: " << e.getErrorCode();
			err << ", SQLState: " << e.getSQLState() << " )" << endl;
			Logger::getInstance()->error(err);

			delete m_stmt;
			delete this->m_pstmt;
			delete this->m_pstmt2;
			return false;
		}
	}

	bool SQLDataBase::InsertFamily(std::vector<std::stringstream>& data, const std::string& FirstName, const std::string& MiddleName, const std::string& LastName, const std::string& Sex, const std::string& Relation, const std::string& Birthday)
	{
		sql::Statement* m_stmt;
		try 
		{
			m_stmt = m_con->createStatement();

			m_pstmt = m_con->prepareStatement("INSERT INTO tbl_family_info(FirstName, LastName, MiddleName, Sex, Position, Birthday) VALUES (?, ?, ?, ?, ?, ?)");
			m_pstmt->setString(1, FirstName);
			m_pstmt->setString(2, LastName);
			m_pstmt->setString(3, MiddleName);
			m_pstmt->setString(4, Sex);
			m_pstmt->setString(5, Relation);
			m_pstmt->setDateTime(6, Birthday);
			m_pstmt->execute();

			ostringstream info;
			info << "\nNumber of row(s) affected:" << m_pstmt->getUpdateCount() << endl;
			Logger::getInstance()->always("Affected Rows");
			Logger::getInstance()->info(info);

			delete m_stmt;
			delete this->m_pstmt;
			delete this->m_pstmt2;
			return true;
		}
		catch (SQLException& e) 
		{
			std::ostringstream err;
			err << "# ERR: SQLException in " << __FILE__;
			err << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			err << "# ERR: " << e.what();
			err << " (MySQL error code: " << e.getErrorCode();
			err << ", SQLState: " << e.getSQLState() << " )" << endl;
			Logger::getInstance()->error(err);

			delete m_stmt;
			delete this->m_pstmt;
			delete this->m_pstmt2;
			return false;
		}
	}

	Family SQLDataBase::GetInfo() noexcept
	{
	
		m_pstmt = m_con->prepareStatement("SELECT * FROM tbl_family_info");
		m_res = m_pstmt->executeQuery();

		//create a dictionary(your very own implementation to practice data structure and algorithms) for this part
	}
	bool SQLDataBase::MatSerialize(std::ostream& out, const cv::Mat& mat)
	{
		MatInfo info(mat.cols, mat.rows, mat.type());
		ostream::sentry sen(out);
		if (!(sen && out.good()))
			return false;
		out.write((char*)(&info), sizeof(MatInfo));
		if (mat.isContinuous())
			out.write((char*)mat.data, info.Size());
		else
		{
			size_t rowsz = info.RowSize();
			for (int r = 0; r < info.rows; ++r)
				out.write((char*)mat.ptr<char>(r), rowsz);
		}
		out.flush();
		return out.good();
	}
	bool SQLDataBase::MatDeserialize(std::istream& in, cv::Mat& out)
	{
		std::istream::sentry s(in);
		if (!(s && in.good()))
		{
			out = cv::Mat();
			return false;
		}
		MatInfo mi;
		in.read((char*)(&mi), sizeof(MatInfo));
		if (mi.Size() < 0)
		{
			out = cv::Mat();
			return false;
		}
		out = cv::Mat(mi.rows, mi.colmns, mi.type);
		in.read((char*)out.data, mi.Size());
		return in.good();
	}
#pragma endregion
}//end namespace jdbc