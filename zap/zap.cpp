#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <zap.h>
#include <mutex>
#include <ctime>
#include <inttypes.h>
#include <memory>
#include <iostream>

#ifdef _WIN32
#ifndef localtime_r
#define localtime_r(t, r) localtime_s((r), (t))
#endif
#endif

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static const std::string hex_chars = "0123456789ABCDEF";

void escapeCopy(std::string& output, const char* input, bool skipDoubleQuota = false)
{
	while (*input)
	{
		auto ch = (unsigned char)*input;
		if (ch == '"')
		{
			if (skipDoubleQuota)
			{
				continue;
			}
			output.append("\\\"");
		}
		else if (ch == '\n')
		{
			output.append("\\n");
		}
		else if (ch == '\r')
		{
			output.append("\\r");
		}
		else if (ch == '\\')
		{
			output.append("\\\\");
		}
		else if (ch == '\b')
		{
			output.append("\\b");
		}
		else if (ch == '\f')
		{
			output.append("\\f");
		}
		else if (ch == '\t')
		{
			output.append("\\t");
		}
		else if (ch > 0x80) //Ä¿Ç°½ö´¦ÀíUCS-2·¶Î§
		{
			// 0080 ~ 07FF
			// 110x xxxx 10xx xxxx
			if ((ch >> 5) == 6) 
			{
				auto nch = (unsigned char)*(input + 1);
				if ((nch >> 6) == 2)
				{
					output.push_back((char)ch);
					output.push_back((char)nch);
					input++;
				}
				else
				{
					output.append("\\u");
					output.push_back(hex_chars[nch >> 4]);
					output.push_back(hex_chars[nch & 0xf]);
					output.push_back(hex_chars[ch >> 4]);
					output.push_back(hex_chars[ch & 0xf]);
					input++;
				}
			}
			// 0800 ~ FFFF
			// 1110 xxxx 10xx xxxx 10xx xxxx
			else if ((ch >> 4) == 0xe)
			{
				auto nch = (unsigned char)*(input + 1);
				auto nch2 = (unsigned char)*(input + 2);
				if ((nch >> 6) == 2 && (nch2 >> 6) == 2)
				{
					output.push_back((char)ch);
					output.push_back((char)nch);
					output.push_back((char)nch2);
					input+=2;
				}
				else
				{
					output.append("\\u");
					output.push_back(hex_chars[nch >> 4]);
					output.push_back(hex_chars[nch & 0xf]);
					output.push_back(hex_chars[ch >> 4]);
					output.push_back(hex_chars[ch & 0xf]);
					input++;
				}
			}
		}
		else
		{
			output.push_back((char)ch);
		}
		input++;
	}
}

void escapeCopy(std::string& output, const std::string& input, bool skipDoubleQuota = false)
{
	return escapeCopy(output, input.c_str(), skipDoubleQuota);
}

std::string Base64Encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i <4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}

void base64Encode(std::string& output, const std::string& input)
{
	output.append(Base64Encode((unsigned char*const)input.data(), input.length()));
}

void hexEncode(std::string& output, const std::string& input)
{
	for (auto ch_ : input)
	{
		unsigned char ch = ch_;
		output.push_back(hex_chars[ch >>  4]);
		output.push_back(hex_chars[ch & 0xf]);		
	}
}


// for micros < 10ms, print "XX us".
// for micros < 10sec, print "XX ms".
// for micros >= 10 sec, print "XX sec".
// for micros <= 1 hour, print Y:X M:S".
// for micros > 1 hour, print Z:Y:X H:M:S".
int AppendHumanMicros(uint64_t micros, char* output, int len, bool fixed_format) 
{
	if (micros < 10000 && !fixed_format) 
	{
		return snprintf(output, len, "%" PRIu64 " us", micros);
	}
	else if (micros < 10000000 && !fixed_format) 
	{
		return snprintf(output, len, "%.3lf ms",
			static_cast<double>(micros) / 1000);
	}
	else if (micros < 1000000l * 60 && !fixed_format) 
	{
		return snprintf(output, len, "%.3lf sec",
			static_cast<double>(micros) / 1000000);
	}
	else if (micros < 1000000ll * 60 * 60 && !fixed_format) 
	{
		return snprintf(output, len, "%02" PRIu64 ":%05.3f M:S",
			micros / 1000000 / 60,
			static_cast<double>(micros % 60000000) / 1000000);
	}
	else {
		return snprintf(output, len, "%02" PRIu64 ":%02" PRIu64 ":%05.3f H:M:S",
			micros / 1000000 / 3600, (micros / 1000000 / 60) % 60,
			static_cast<double>(micros % 60000000) / 1000000);
	}
}

std::string BytesToHumanString(uint64_t bytes) 
{
	const char* size_name[] = { "KB", "MB", "GB", "TB" };
	double final_size = static_cast<double>(bytes);
	size_t size_idx;

	// always start with KB
	final_size /= 1024;
	size_idx = 0;

	while (size_idx < 3 && final_size >= 1024) {
		final_size /= 1024;
		size_idx++;
	}

	char buf[20];
	snprintf(buf, sizeof(buf), "%.2f %s", final_size, size_name[size_idx]);
	return std::string(buf);
}

void AppendNumberTo(std::string* str, uint64_t num) {
	char buf[30];
	snprintf(buf, sizeof(buf), "%" PRIu64, num);
	str->append(buf);
}

void AppendNumberTo(std::string* str, int64_t num) {
	char buf[30];
	snprintf(buf, sizeof(buf), "%" PRId64, num);
	str->append(buf);
}

std::string NumberToString(uint64_t num) 
{
	std::string r;
	AppendNumberTo(&r, num);
	return r;
}

std::string NumberToString(int64_t num)
{
	std::string r;
	AppendNumberTo(&r, num);
	return r;
}

std::mutex g_loggerMtx;
std::map<std::string, std::shared_ptr<zap::Category>> g_loggers;

namespace zap
{
	Category::Category(const std::string& name) : m_log(log4cpp::Category::getInstance(name))
	{
	}

	std::string Category::build(const std::string& msg, std::initializer_list<Field> fields)
	{
		std::string m;
		m.reserve(4096);
		m.append(zap::String("msg", msg));
		for (auto it = fields.begin(); it != fields.end(); ++it)
		{
			if (it->length() == 0)
			{
				continue;
			}
			m.append(",");
			m.append(*it);
		}
		return std::move(m);
	}

	Category& Category::getInstance(const std::string& name)
	{
		std::lock_guard<std::mutex> lck(g_loggerMtx);
		auto it = g_loggers.find(name);
		if (it == g_loggers.end())
		{
			it = g_loggers.insert(std::make_pair(name, std::shared_ptr<Category>(new Category(name)))).first;
		}
		return *(it->second);
	}

	Field String(const std::string& key, const std::string& value)
	{
		Field m;
		m.append("\"");
		escapeCopy(m, key, true);
		m.append("\":\"");
		escapeCopy(m, value);
		m.append("\"");
		return m;
	}

	Field String(const std::string& key, const char* value)
	{
		Field m;
		m.append("\"");
		escapeCopy(m, key, true);
		m.append("\":\"");
		escapeCopy(m, value);
		m.append("\"");
		return m;
	}

	Field Binary(const std::string& key, const std::string& value)
	{
		Field m;
		m.append("\"");
		escapeCopy(m, key, true);
		m.append("\":\"");
		base64Encode(m, value);
		m.append("\"");
		return m;
	}

	Field Bytes(const std::string& key, const std::string& value)
	{
		Field m;
		m.append("\"");
		escapeCopy(m, key, true);
		m.append("\":\"");
		hexEncode(m, value);
		m.append("\"");
		return m;
	}
	Field Int(const std::string& key, int value)
	{
		return Int64(key, value);
	}

	Field UInt(const std::string& key, unsigned value)
	{
		return UInt64(key, value);
	}

	Field Int32(const std::string& key, int32_t value)
	{
		return Int64(key, value);
	}

	Field UInt32(const std::string& key, uint32_t value)
	{
		return UInt64(key, value);
	}

	Field Int64(const std::string& key, int64_t value)
	{
		Field m;
		m.append("\"");
		escapeCopy(m, key, true);
		m.append("\":");
		AppendNumberTo(&m, value);
		return m;
	}

	Field UInt64(const std::string& key, uint64_t value)
	{
		Field m;
		m.append("\"");
		escapeCopy(m, key, true);
		m.append("\":");
		AppendNumberTo(&m, value);
		return m;
	}

	Field Bytes(const std::string& key, uint64_t value)
	{
		return String(key, BytesToHumanString(value));
	}

	Field Bool(const std::string& key, bool value)
	{
		Field m;
		m.append("\"");
		escapeCopy(m, key, true);
		m.append("\":");
		if (value)
		{
			m.append("true");
		}
		else
		{
			m.append("false");
		}
		return m;
	}

	Field Float(const std::string& key, float value)
	{
		char buf[64] = {0};
		Field m;
		m.append("\"");
		escapeCopy(m, key, true);
		m.append("\":");
		snprintf(buf, sizeof(buf), "%f", value);
		m.append(buf);
		return m;
	}

	Field Double(const std::string& key, double value)
	{
		char buf[64] = { 0 };
		Field m;
		m.append("\"");
		escapeCopy(m, key, true);
		m.append("\":");
		snprintf(buf, sizeof(buf), "%lf", value);
		m.append(buf);
		return m;
	}
	
	Field JsonObject(const std::string& obj)
	{
		Field m;
		auto idx = 0;
		m.reserve(obj.size());
		for(auto ch : obj)
		{
			if (ch == '{')
			{
				if (idx != 0)
				{
					m.push_back(ch);
				}
				idx ++;
			}
			else if (ch == '}')
			{
				idx --;
				if (idx != 0)
				{
					m.push_back(ch);
				}
			}
			else
			{
				m.push_back(ch);
			}
		}
		return m;
	}


	Field Time(const std::string& key, time_t value)
	{
		struct tm tm;
		localtime_r(&value, &tm);
		char buf[64];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
		return String(key, buf);
	}

	Field DurationUs(const std::string& key, uint64_t value)
	{
		char buf[64] = { 0 };
		AppendHumanMicros(value, buf, sizeof(buf), false);
		return String(key, buf);
	}

	Field Caller(const char* fn, int line, const char* func)
	{
		char buf[64];
		Field m;
		m.append(R"("caller":")");
		m.append(func);
		m.push_back('@');
		m.append(fn);
		snprintf(buf, sizeof(buf), ":%d\"", line);
		m.append(buf);
		return m;
	}
}
