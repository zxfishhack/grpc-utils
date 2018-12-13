#pragma once
#include <functional>
#include <map>

template <typename MessageType>
class CppTagsMessageTypeMask {
public:
	static const char* Mask();
	static void MaskMessage(const MessageType& src, MessageType* dst);
};

template<typename Service>
class CppTagsMethodReflect {
public:
	template<typename Rx, typename Ty>
	static const char* Name(Rx Ty::* _Pm) {
		return _name[typeid(std::mem_fn(_Pm)).name()].first.c_str();
	}
	template<typename Rx, typename Ty>
	static const char* FullName(Rx Ty::* _Pm) {
		return _name[typeid(std::mem_fn(_Pm)).name()].second.c_str();
	}
	template<typename Rx, typename Ty>
	static bool registerMethod(Rx Ty::* _Pm, const char* name, const char* fullName) {
		_name[typeid(std::mem_fn(_Pm)).name()] = std::make_pair(name, fullName);
		return true;
	}
	static std::map<std::string, std::pair<std::string, std::string>> _name;
};
