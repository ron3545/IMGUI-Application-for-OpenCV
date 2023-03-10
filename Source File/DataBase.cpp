#include "../Header Files/DataBase.h"

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

	SQLDataBase::SQLDataBase(void)  :	m_driver(NULL), m_con(NULL),
										m_Host(NULL), m_UserID(NULL),
										m_Password(NULL), m_schema(NULL)
	{}

	SQLDataBase::SQLDataBase(TCHAR* ptchHost, TCHAR* ptchUser,
		TCHAR* ptchDB, TCHAR* ptchPassword) :	m_driver(NULL), m_con(NULL),
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
					Logger::getInstance()->info("Connection is alive\n");
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
				Logger::getInstance()->info("Connection is alive\n");

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

			connected = false;
		}
		return connected;
	}

	bool SQLDataBase::Disconnect() throw()
	{
		try {
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
	
	bool SQLDataBase::InsertFamily(const std::vector<cv::Mat>& images, const std::string& FirstName, const std::string& MiddleName, const std::string& LastName, const std::string& Sex, const std::string& Relation, const std::string& Birthday)
	{

		time_t      now = time(NULL);
		struct tm   tstruct;
		char        buf[80];
		localtime_s(&tstruct, &now);

		strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
		sql::PreparedStatement* m_pstmt, * m_pstmt2;
		sql::Statement* m_stmt;
		try 
		{
			m_stmt = m_con->createStatement();
			std::ostringstream Fam_Info;

			m_pstmt = m_con->prepareStatement("INSERT INTO tbl_family_info(FirstName, LastName, MiddleName, Sex, Relationship, Birthday, time_of_entry) VALUES (?, ?, ?, ?, ?, ?, ?)");
			m_pstmt->setString(1, FirstName);
			m_pstmt->setString(2, LastName);
			m_pstmt->setString(3, MiddleName);
			m_pstmt->setString(4, Sex);
			m_pstmt->setString(5, Relation);
			m_pstmt->setString(6, Birthday);
			m_pstmt->setDateTime(7, buf);
			m_pstmt->execute();

			m_stmt->execute("SET @lastInsertId = LAST_INSERT_ID()");
			std::string insert_to_TBLFace = "INSERT INTO tbl_face(Picture, family_info_id) VALUES (?, @lastInsertId)";
			m_pstmt2 = m_con->prepareStatement(insert_to_TBLFace);
			
			//prevent thisloop for creating copy
			for (const cv::Mat& image : images)
			{
				std::stringstream ss;
				MatSerialize(ss, image);
				std::istream data(ss.rdbuf()); //SERIALIZED MAT
				m_pstmt2->setBlob(1, &data);
				m_pstmt2->execute();
			}

			delete m_stmt;
			delete m_pstmt;
			delete m_pstmt2;

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
			return false;
		}
	}

	// this will be displayed as a table form in menu
	void SQLDataBase::GetInfo(std::map<string, RegistryInfo>& person_info, std::map<std::string, RegistryImages>& person_images) noexcept
	{
		try
		{
			sql::ResultSet* m_res;

			RegistryInfo person_data;
			RegistryImages img_data;

			std::ostringstream one_to_many;

			one_to_many << "SELECT info.ID AS fam_id, "
						<< "info.FirstName AS first_name, "
						<< "info.LastName AS last_name, "
						<< "info.MiddleName AS middle_name, "
						<< "info.Sex AS sex, "
						<< "info.Relationship AS relation, "
						<< "info.Birthday AS birthday, "
						<< "info.time_of_entry AS time_stamp, "
						<< "face.Img_ID AS img_id,   face.Picture AS pic "
						<< " FROM tbl_family_info info LEFT JOIN tbl_face face ON info.ID = face.family_info_id";

			sql::Statement* stmt;
			stmt = m_con->createStatement();
			m_res = stmt->executeQuery(one_to_many.str());

			Logger::getInstance()->info("retrieving the data from the database");
			static std::string previous_name, current_name;
			
			while (m_res->next())
			{
				int person_id = 0;
				int img_id = 0;

				if (!current_name.empty() && !previous_name.empty() && current_name != previous_name && person_info.find(current_name) == person_info.end() && person_images.find(current_name) == person_images.end())
				{
					person_info.emplace(current_name, person_data);
					person_images.emplace(current_name, img_data);
					img_data.Clear();
					previous_name = current_name;
				}

				person_id	= m_res->getInt("fam_id");
				img_id		= m_res->getInt("img_id");
				
				//every iteration this part will be countinousely over written
				person_data.SetData(person_id, m_res->getString("first_name"), m_res->getString("middle_name"), m_res->getString("last_name"), m_res->getString("relation"), m_res->getString("sex"), m_res->getString("birthday"));
				current_name = person_data.Get_FullName();
				
				cv::Mat img;
				std::istream& is(*m_res->getBlob("pic"));
				MatDeserialize(is, img);
				vector<cv::Mat> images;
				images.push_back(img);

				//every iteration data will be added to vector. So, we need to clear this part everytime current_name changes 
				img_data.Set(img_id, img_id, images);
			}
			Logger::getInstance()->info("All data has been retrieved and loaded to RAM");

			delete m_res;
			delete stmt;
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

			return;
		}
	}

	bool SQLDataBase::Is_TableEmpty(const char* tbl_name)
	{
		sql::ResultSet* m_res;
		sql::Statement* stmt;

		bool is_empty = false;
		try 
		{
			std::ostringstream promt;
			promt << "SELECT EXISTS(SELECT 1 FROM " << tbl_name << ") AS output";

			stmt = m_con->createStatement();
			m_res = stmt->executeQuery(promt.str());

			while (m_res->next()) {
				//ony one row is expected to be captured
				if (m_res->getString("output") == "0")
					is_empty = true;
				else is_empty = false;
			}
			delete m_res;
			delete stmt;
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

			return false;
		}
		return is_empty;
	}

	bool SQLDataBase::InsertunknownPerson()
	{
		
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

}