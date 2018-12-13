#pragma once
#include <string>
#include <log4cpp/Category.hh>
#include <stdint.h>
#include <chrono>

namespace zap
{
	class Field : public std::string{};
	//此Category关联的Appender的Layout必须为PtternLayout，且格式形如：{"ts":"%d{%FT%T.%l%z}","level":"%p","thr":"%t",%m}%n
	class Category
	{
	public:
		static Category& getInstance(const std::string& name);
		void debug(const std::string& msg, std::initializer_list<Field> fields)
		{
			m_log.debug(build(msg, fields));
		}
		void info(const std::string& msg, std::initializer_list<Field> fields)
		{
			m_log.info(build(msg, fields));
		}
		void notice(const std::string& msg, std::initializer_list<Field> fields)
		{
			m_log.notice(build(msg, fields));
		}
		void warn(const std::string& msg, std::initializer_list<Field> fields)
		{
			m_log.warn(build(msg, fields));
		}
		void error(const std::string& msg, std::initializer_list<Field> fields)
		{
			m_log.error(build(msg, fields));
		}
		void crit(const std::string& msg, std::initializer_list<Field> fields)
		{
			m_log.crit(build(msg, fields));
		}
		void alert(const std::string& msg, std::initializer_list<Field> fields)
		{
			m_log.alert(build(msg, fields));
		}
		void fatal(const std::string& msg, std::initializer_list<Field> fields)
		{
			m_log.fatal(build(msg, fields));
		}
		void emerg(const std::string& msg, std::initializer_list<Field> fields)
		{
			m_log.emerg(build(msg, fields));
		}
		void log(log4cpp::Priority::Value priority, const std::string& msg, std::initializer_list<Field> fields)
		{
			m_log.log(priority, build(msg, fields));
		}
	private:
		Category(const std::string& name);
		std::string build(const std::string& msg, std::initializer_list<Field> fields);

		log4cpp::Category& m_log;
	};

	//按STRING输出，UTF8-ONLY
	Field String(const std::string& key, const std::string& value);
	Field String(const std::string& key, const char* value);
	//按BASE64输出
	Field Binary(const std::string& key, const std::string& value);
	//按十六进制输出
	Field Hex(const std::string& key, const std::string& value);

	Field Int(const std::string& key, int value);
	Field UInt(const std::string& key, unsigned int value);
	Field Int32(const std::string& key, int32_t value);
	Field UInt32(const std::string& key, uint32_t value);
	Field Int64(const std::string& key, int64_t value);
	Field UInt64(const std::string& key, uint64_t value);
	//输出可读的大小
	//如 XXTB XXGB
	Field Bytes(const std::string& key, uint64_t value);

	Field Bool(const std::string& key, bool value);

	Field Float(const std::string& key, float value);
	Field Double(const std::string& key, double value);
	
	Field JsonObject(const std::string& obj);

	//格式化成localtime输出
	Field Time(const std::string& key, time_t value);
	Field DurationUs(const std::string& key, uint64_t value);
	template<class _Rep, class _Period>
	Field Duration(const std::string& key, const std::chrono::duration<_Rep, _Period>& value)
	{
		return DurationUs(key, std::chrono::duration_cast<std::chrono::duration<uint64_t, std::micro>>(value).count());
	}
	inline Field DurationMs(const std::string& key, uint64_t value)
	{
		return DurationUs(key, value * 1000);
	}
	

	Field Caller(const char* fn, int line, const char* func);
}

#define ZAP_CALLER() zap::Caller(__FILE__, __LINE__, __FUNCTION__)
