// C++ Header File(s)
#include <iostream>
#include <cstdlib>
#include <string>
#include <stdio.h>
#include <time.h>
// Code Specific Header Files(s)
#include "..\Header Files\Log.h"

using namespace std;
using namespace CPlusPlusLogging;

Logger* Logger::m_Instance = 0;
// Log file name. File name should be change from here only
const string logFileName = "MyLogFile.log";
const string openvino = "OpenVino_LogFile.log";

Logger::Logger()
{
    m_File.open(logFileName.c_str(), ios::out | ios::app);
    OpenVino_Logs.open(openvino.c_str(), ios::out | ios::app);

    m_LogLevel = LogLevel::LOG_LEVEL_TRACE;
    m_LogType = LogType::FILE_LOG;

    // Initialize mutex
#ifdef _WIN64
    InitializeCriticalSection(&m_Mutex);
#else
    int ret = 0;
    ret = pthread_mutexattr_settype(&m_Attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    if (ret != 0)
    {
        printf("Logger::Logger() -- Mutex attribute not initialize!!\n");
        exit(0);
    }
    ret = pthread_mutex_init(&m_Mutex, &m_Attr);
    if (ret != 0)
    {
        printf("Logger::Logger() -- Mutex not initialize!!\n");
        exit(0);
    }
#endif
}

Logger::~Logger()
{
    m_File.close();
    OpenVino_Logs.close();
#ifdef _WIN64
    DeleteCriticalSection(&m_Mutex);
#else
    pthread_mutexattr_destroy(&m_Attr);
    pthread_mutex_destroy(&m_Mutex);
#endif
}

Logger* Logger::getInstance() throw ()
{
    if (m_Instance == 0)
    {
        m_Instance = new Logger();
    }
    return m_Instance;
}

void Logger::lock()
{
#ifdef _WIN64
    EnterCriticalSection(&m_Mutex);
#else
    pthread_mutex_lock(&m_Mutex);
#endif
}

void Logger::unlock()
{
#ifdef _WIN64
    LeaveCriticalSection(&m_Mutex);
#else
    pthread_mutex_unlock(&m_Mutex);
#endif
}

void Logger::logIntoFile(std::string& data)
{
    lock();
    m_File << getCurrentTime() << "  " << data << endl;
    unlock();
}

void Logger::logOnConsole(std::string& data)
{
    cout << getCurrentTime() << "  " << data << endl;
}


string Logger::getCurrentTime()
{
    time_t      now = time(NULL);
    struct tm   tstruct;
    char        buf[80];
    localtime_s(&tstruct, &now);

    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}

// Interface for Error Log
void Logger::error(const char* text) throw()
{
    string data;
    data.append("[ERROR]: ");
    data.append(text);

    // ERROR must be capture
    if (m_LogType == LogType::FILE_LOG) //display on .txt file
    {
        logIntoFile(data);
    }
    else if (m_LogType == LogType::CONSOLE) //display log on console
    {
        logOnConsole(data);
    }
}

void Logger::error(std::string& text) throw()
{
    error(text.data());
}

void Logger::error(std::ostringstream& stream) throw()
{
    string text = stream.str();
    error(text.data());
}

// Interface for Alarm Log 
void Logger::alarm(const char* text) throw()
{
    string data;
    data.append("[ALARM]: ");
    data.append(text);

    // ALARM must be capture
    if (m_LogType == LogType::FILE_LOG)
    {
        logIntoFile(data);
    }
    else if (m_LogType == LogType::CONSOLE)
    {
        logOnConsole(data);
    }
}

void Logger::alarm(std::string& text) throw()
{
    alarm(text.data());
}

void Logger::alarm(std::ostringstream& stream) throw()
{
    string text = stream.str();
    alarm(text.data());
}

// Interface for Always Log 
void Logger::always(const char* text) throw()
{
    string data;
    data.append("[ALWAYS]: ");
    data.append(text);

    // No check for ALWAYS logs
    if (m_LogType == LogType::FILE_LOG)
    {
        logIntoFile(data);
    }
    else if (m_LogType == LogType::CONSOLE)
    {
        logOnConsole(data);
    }
}

void Logger::always(std::string& text) throw()
{
    always(text.data());
}

void Logger::always(std::ostringstream& stream) throw()
{
    string text = stream.str();
    always(text.data());
}

// Interface for Buffer Log 
void Logger::buffer(const char* text) throw()
{
    // Buffer is the special case. So don't add log level
    // and timestamp in the buffer message. Just log the raw bytes.
    if ((m_LogType == LogType::FILE_LOG) && (m_LogLevel >= LogLevel::LOG_LEVEL_BUFFER))
    {
        lock();
        m_File << text << endl;
        unlock();
    }
    else if ((m_LogType == LogType::CONSOLE) && (m_LogLevel >= LogLevel::LOG_LEVEL_BUFFER))
    {
        cout << text << endl;
    }
}

void Logger::buffer(std::string& text) throw()
{
    buffer(text.data());
}

void Logger::buffer(std::ostringstream& stream) throw()
{
    string text = stream.str();
    buffer(text.data());
}

// Interface for Info Log
void Logger::info(const char* text) throw()
{
    string data;
    data.append("[INFO]: ");
    data.append(text);

    if ((m_LogType == LogType::FILE_LOG) && (m_LogLevel >= LogLevel::LOG_LEVEL_INFO))
    {
        logIntoFile(data);
    }
    else if ((m_LogType == LogType::CONSOLE) && (m_LogLevel >= LogLevel::LOG_LEVEL_INFO))
    {
        logOnConsole(data);
    }
}

/*
inline void Logger::info(const std::shared_ptr<ov::Model>& model) noexcept
{
    ov::OutputVector inputs = model->inputs();
    ov::OutputVector outputs = model->outputs();

    OpenVino_Logs << model->get_friendly_name() << endl;
    if ((m_LogType == LogType::FILE_LOG) && (m_LogLevel >= LogLevel::LOG_LEVEL_INFO))
    {
        OpenVino_Logs << "INPUTS";
        // ov::Node is the backbone of graph value dataflow. Every node has zero or more nodes as arguments and one value, whuch is either a tensor or as (posibly aempty) tuple of values.
        for (const ov::Output<ov::Node>& input : inputs)
        {
            OpenVino_Logs << getCurrentTime() << " " << input.get_any_name() << " , " << input.get_element_type() << endl << "\t \t" << input.get_node() << endl;
            OpenVino_Logs << input.get_partial_shape() << endl << input.get_shape().data() << endl;
        }

        OpenVino_Logs << "OUTPUTS";

        for (const ov::Output<ov::Node>& output : outputs)
        {
            OpenVino_Logs << getCurrentTime() << " " << output.get_any_name() << " , " << output.get_element_type() << endl << "\t \t" << output.get_node() << endl;
            OpenVino_Logs << output.get_partial_shape() << endl << output.get_shape().data() << endl;
        }
    }
}
*/

void Logger::info(std::string& text) throw()
{
    info(text.data());
}

void Logger::info(std::ostringstream& stream) throw()
{
    string text = stream.str();
    info(text.data());
}

// Interface for Trace Log
void Logger::trace(const char* text) throw()
{
    string data;
    data.append("[TRACE]: ");
    data.append(text);

    if ((m_LogType == LogType::FILE_LOG) && (m_LogLevel >= LogLevel::LOG_LEVEL_TRACE))
    {
        logIntoFile(data);
    }
    else if ((m_LogType == LogType::CONSOLE) && (m_LogLevel >= LogLevel::LOG_LEVEL_TRACE))
    {
        logOnConsole(data);
    }
}

void Logger::trace(std::string& text) throw()
{
    trace(text.data());
}

void Logger::trace(std::ostringstream& stream) throw()
{
    string text = stream.str();
    trace(text.data());
}

// Interface for Debug Log
void Logger::debug(const char* text) throw()
{
    string data;
    data.append("[DEBUG]: ");
    data.append(text);

    if ((m_LogType == LogType::FILE_LOG) && (m_LogLevel >= LogLevel::LOG_LEVEL_DEBUG))
    {
        logIntoFile(data);
    }
    else if ((m_LogType == LogType::CONSOLE) && (m_LogLevel >= LogLevel::LOG_LEVEL_DEBUG))
    {
        logOnConsole(data);
    }
}

void Logger::debug(std::string& text) throw()
{
    debug(text.data());
}

void Logger::debug(std::ostringstream& stream) throw()
{
    string text = stream.str();
    debug(text.data());
}

// Interfaces to control log levels
void Logger::updateLogLevel(LogLevel logLevel)
{
    m_LogLevel = logLevel;
}

// Enable all log levels
void Logger::enaleLog()
{
    m_LogLevel = LogLevel::ENABLE_LOG;
}

// Disable all log levels, except error and alarm
void Logger::disableLog()
{
    m_LogLevel = LogLevel::DISABLE_LOG;
}

// Interfaces to control log Types
void Logger::updateLogType(LogType logType)
{
    m_LogType = logType;
}

void Logger::enableConsoleLogging()
{
    m_LogType = LogType::CONSOLE;
}

void Logger::enableFileLogging()
{
    m_LogType = LogType::FILE_LOG;
}

