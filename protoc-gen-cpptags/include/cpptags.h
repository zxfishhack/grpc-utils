#pragma once
#include <string>
#include <google/protobuf/message.h>

template <typename MessageType>
class CppTagsMessageTypeMask {
public:
  static void MaskMessageToJson(const MessageType& src, std::string* out);
};

template<>
inline void CppTagsMessageTypeMask<google::protobuf::int32>::MaskMessageToJson(const google::protobuf::int32& src, std::string* out)
 {
	char buf[64] = {0};
	snprintf(buf, sizeof(buf), "%d", int(src));
	out->append(buf);
}

template<>
inline void CppTagsMessageTypeMask<google::protobuf::int64>::MaskMessageToJson(const google::protobuf::int64& src, std::string* out) {
	char buf[64] = { 0 };
	snprintf(buf, sizeof(buf), "%lld", (long long)(src));
	out->append(buf);
}

template<>
inline void CppTagsMessageTypeMask<google::protobuf::uint32>::MaskMessageToJson(const google::protobuf::uint32& src, std::string* out) {
	char buf[64] = { 0 };
	snprintf(buf, sizeof(buf), "%u", (unsigned int)(src));
	out->append(buf);
}

template<>
inline void CppTagsMessageTypeMask<google::protobuf::uint64>::MaskMessageToJson(const google::protobuf::uint64& src, std::string* out) {
	char buf[64] = { 0 };
	snprintf(buf, sizeof(buf), "%llu", (unsigned long long)(src));
	out->append(buf);
}

template<>
inline void CppTagsMessageTypeMask<double>::MaskMessageToJson(const double& src, std::string* out) {
	char buf[64] = { 0 };
	snprintf(buf, sizeof(buf), "%lf", src);
	out->append(buf);
}

template<>
inline void CppTagsMessageTypeMask<float>::MaskMessageToJson(const float& src, std::string* out) {
	char buf[64] = { 0 };
	snprintf(buf, sizeof(buf), "%f", src);
	out->append(buf);
}

template<>
inline void CppTagsMessageTypeMask<google::protobuf::string>::MaskMessageToJson(const google::protobuf::string& src, std::string* out) {
	out->append("\"");
	out->append(src);
	out->append("\"");
}

template<>
inline void CppTagsMessageTypeMask<bool>::MaskMessageToJson(const bool& src, std::string* out) {
	out->append(src ? "true" : "false");
}

