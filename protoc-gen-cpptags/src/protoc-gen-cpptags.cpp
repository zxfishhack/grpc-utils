#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.pb.h>
#include "cpptags.pb.h"
#include <thread>
#include <memory>

using namespace google::protobuf;

inline bool HasSuffixString(const std::string& str,
                            const std::string& suffix) {
  return str.size() >= suffix.size() &&
         str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline std::string StripSuffixString(const std::string& str, const std::string& suffix) {
  if (HasSuffixString(str, suffix)) {
    return str.substr(0, str.size() - suffix.size());
  } else {
    return str;
  }
}

string StripProto(const std::string& filename) {
  if (HasSuffixString(filename, ".protodevel")) {
    return StripSuffixString(filename, ".protodevel");
  } else {
    return StripSuffixString(filename, ".proto");
  }
}

void StringReplace(const std::string& s, const std::string& oldsub,
                   const std::string& newsub, bool replace_all,
                   std::string* res) {
  if (oldsub.empty()) {
    res->append(s);  // if empty, append the given string.
    return;
  }

  std::string::size_type start_pos = 0;
  std::string::size_type pos;
  do {
    pos = s.find(oldsub, start_pos);
    if (pos == string::npos) {
      break;
    }
    res->append(s, start_pos, pos - start_pos);
    res->append(newsub);
    start_pos = pos + oldsub.size();  // start searching again after the "old"
  } while (replace_all);
  res->append(s, start_pos, s.length() - start_pos);
}

std::string StringReplace(const std::string& s, const std::string& oldsub,
                     const std::string& newsub, bool replace_all) {
  std::string ret;
  StringReplace(s, oldsub, newsub, replace_all, &ret);
  return ret;
}

std::string DotsToColons(const std::string& name) {
  return StringReplace(name, ".", "::", true);
}

inline string Namespace(const std::string& package) {
  if (package.empty()) return "";
  return "::" + DotsToColons(package);
}

inline string Namespace(const FileDescriptor* d) {
  return Namespace(d->package());
}
template <typename Desc>
string Namespace(const Desc* d) {
  return Namespace(d->file());
}

const auto CppTagsFileHeader =
R"(
#include <cpptags.h>
#include "$basename$.pb.h"
)";

const auto CppMaskEnumDefine =
R"(
template<>
void CppTagsMessageTypeMask<$ns$::$messageType$>::MaskMessageToJson(const $ns$::$messageType$& src, std::string* out) {
  out->push_back('"');
  out->append($ns$::$messageType$_Name(src));
  out->push_back('"');
}
)";

const auto CppMaskMessageDefinePrefix =
R"(
template<>
void CppTagsMessageTypeMask<$ns$::$messageType$>::MaskMessageToJson(const $ns$::$messageType$& src, std::string* out) {
)";

const auto CppMaskMessageDefinePostfix = 
R"(
}

)";

const char* kCppTypeToNamespace[] = {
	"",     // 0 is reserved for errors

	"::google::protobuf",     // CPPTYPE_INT32
	"::google::protobuf",     // CPPTYPE_INT64
	"::google::protobuf",    // CPPTYPE_UINT32
	"::google::protobuf",    // CPPTYPE_UINT64
	"",    // CPPTYPE_DOUBLE
	"",     // CPPTYPE_FLOAT
	"",      // CPPTYPE_BOOL
	"",      // CPPTYPE_ENUM
	"::google::protobuf",    // CPPTYPE_STRING
	"",   // CPPTYPE_MESSAGE
};

class CppTagsGenerator : public compiler::CodeGenerator
{
public:
	bool Generate(const FileDescriptor* file, const string& parameter,
		compiler::GeneratorContext* ctx, string* error) const override
	{
		string basename = StripProto(file->name());

		std::unique_ptr<io::ZeroCopyOutputStream> sourceOutput(ctx->Open(basename + ".cpptags.pb.cc"));

		io::Printer sourcePrinter(sourceOutput.get(), '$');

		std::map<string, string> variables;
		variables["basename"] = basename;

		sourcePrinter.Print(variables, CppTagsFileHeader);

		for (auto i = 0; i < file->message_type_count(); i++) {
			auto mt = file->message_type(i);
			variables["ns"] = Namespace(mt->file());
			variables["messageType"] = mt->name();
			sourcePrinter.Print(variables, CppMaskMessageDefinePrefix);
			sourcePrinter.Indent();
			sourcePrinter.Print("out->push_back('{');");
			bool first = true;
			for (auto j = 0; j < mt->field_count(); j++) {
				auto field = mt->field(j);
				addFieldCode(sourcePrinter, variables, field, first);
			}
			sourcePrinter.Print("out->push_back('}');");
			sourcePrinter.Outdent();
			sourcePrinter.Print(variables, CppMaskMessageDefinePostfix);
		}
		for(auto i=0; i<file->enum_type_count(); i++) {
			auto et = file->enum_type(i);
			variables["ns"] = Namespace(et->file());
			variables["messageType"] = et->name();
			sourcePrinter.Print(variables, CppMaskEnumDefine);
		}

		return true;
	}
private:
	void addFieldCode(io::Printer & sourcePrinter, std::map<string, string>& variables,
		const FieldDescriptor * field, bool& first) const {
		auto& opts = field->options();
		if (opts.HasExtension(cpptags::log_field) && opts.GetExtension(cpptags::log_field) != "") {
			if (field->type() == FieldDescriptor::TYPE_BYTES) {
				std::cerr << "cpptags not support bytes yet." << std::endl;
				return;
			}
			variables["fieldName"] = opts.GetExtension(cpptags::log_field);
			if (!first) {
				sourcePrinter.Print(variables, "out->append(\",\");\n");
			}
			else {
				first = false;
			}
			sourcePrinter.Print(variables, R"(out->append("\"$fieldName$\":");)");
			sourcePrinter.Print("\n");
			if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
				variables["ns"] = Namespace(field->message_type()->file());
				variables["fieldType"] = field->message_type()->name();
			} else if (field->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
				variables["ns"] = Namespace(field->enum_type()->file());
				variables["fieldType"] = field->enum_type()->name();
			} else {
				variables["ns"] = kCppTypeToNamespace[field->cpp_type()];
				variables["fieldType"] = field->cpp_type_name();
			}
			if (field->is_repeated()) {
				variables["fieldName"] = field->lowercase_name();
				sourcePrinter.Print("out->append(\"[\");");
				sourcePrinter.Print("\n");
				sourcePrinter.Print(variables, "for(int i=0; i<src.$fieldName$_size(); i++) {\n");
				variables["fieldName"] = field->lowercase_name() + "(i)";
				sourcePrinter.Indent();
				sourcePrinter.Print(variables, "if (i != 0) {\n");
				sourcePrinter.Indent();
				sourcePrinter.Print(variables, "out->append(\",\");\n");
				sourcePrinter.Outdent();
				sourcePrinter.Print(variables, "}\n");
			}
			else {
				variables["fieldName"] = field->lowercase_name() + "()";
			}

			if (field->cpp_type() <= FieldDescriptor::MAX_CPPTYPE) {
				sourcePrinter.Print(variables, "::CppTagsMessageTypeMask<$ns$::$fieldType$>::MaskMessageToJson(src.$fieldName$, out);");
				sourcePrinter.Print("\n");
			}
			else {
				sourcePrinter.Print(variables, "out->append(\"\\\"unkndown type\\\"\");");
				sourcePrinter.Print("\n");
			}

			if (field->is_repeated()) {
				sourcePrinter.Outdent();
				sourcePrinter.Print("}");
				sourcePrinter.Print("\n");
				sourcePrinter.Print("out->append(\"]\");");
				sourcePrinter.Print("\n");
			}
			variables.erase("fieldName");
		}
	}
};
int main(int argc, char* argv[])
{
	bool debugHook = true;
	while(!debugHook) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	CppTagsGenerator generator;
	return PluginMain(argc, argv, &generator);
}
