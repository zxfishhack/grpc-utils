#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/compiler/cpp/cpp_helpers.h>
#include "cpptags.pb.h"

const auto CppMaskHeader =
R"(
#pragma once
#include <cpptags.h>
#include "$basename$.$postfix$"
#include <google/protobuf/util/field_mask_util.h>
)";

const auto CppMaskTemplateClass =
R"(template<>
class CppTagsMessageTypeMask<$messageType$> {
public:
  static const char* Mask() {
    return "$mask$";
  }
  static void MaskMessage(const $messageType$& src, $messageType$* dst) {
    google::protobuf::util::FieldMaskUtil::MergeOptions opts;
    opts.set_replace_repeated_fields(true);
    opts.set_replace_message_fields(true);
    google::protobuf::FieldMask fm;
    google::protobuf::util::FieldMaskUtil::FromString(Mask(), &fm);
    google::protobuf::util::FieldMaskUtil::MergeMessageTo(src, fm, opts, dst);
  }
};

)";

const auto CppTagsMethodReflectInclude =
R"(
#include "$basename$.cpptags.h"
)";

const auto CppTagsMethodReflectClassDefine =
R"(
std::map<std::string, std::pair<std::string, std::string>> CppTagsMethodReflect<$ns$::$serviceName$::Service>::_name;
std::map<std::string, std::pair<std::string, std::string>> CppTagsMethodReflect<$ns$::$serviceName$::AsyncService>::_name;
)";

const auto CppTagsMethodReflectMethodDefine =
R"(
static bool $ns$_$serviceName$_$methodName$ = CppTagsMethodReflect<$ns$::$serviceName$::Service>::registerMethod(&$ns$::$serviceName$::Service::$methodName$, "$methodName$", "$serviceName$.$methodName$");
static bool async_$ns$_$serviceName$_$methodName$ = CppTagsMethodReflect<$ns$::$serviceName$::AsyncService>::registerMethod(&$ns$::$serviceName$::AsyncService::Request$methodName$, "$methodName$", "$serviceName$.$methodName$");
)";

class CppTagsGenerator : public google::protobuf::compiler::CodeGenerator
{
public:
    bool Generate(const google::protobuf::FileDescriptor* file, const std::string& parameter,
        google::protobuf::compiler::GeneratorContext* ctx, std::string* error) const override
    {
        std::string basename = google::protobuf::compiler::cpp::StripProto(file->name());
        
    	google::protobuf::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream> headerOutput(
            ctx->Open(basename + ".cpptags.h"));

		google::protobuf::scoped_ptr<google::protobuf::io::ZeroCopyOutputStream> sourceOutput(
			ctx->Open(basename + ".cpptags.cpp"));

        google::protobuf::io::Printer headerPrinter(headerOutput.get(), '$');
		google::protobuf::io::Printer sourcePrinter(sourceOutput.get(), '$');

        std::map<std::string, std::string> variables;
        variables["basename"] = basename;

        if (file->service_count() > 0) {
            variables["postfix"] = "grpc.pb.h";
        }
        else {
            variables["postfix"] = "pb.h";
        }

        headerPrinter.Print(variables, CppMaskHeader);

        google::protobuf::compiler::cpp::NamespaceOpener ns(google::protobuf::compiler::cpp::Namespace(file), &headerPrinter);

        for (auto i = 0; i < file->message_type_count(); i++) {
            auto mt = file->message_type(i);
            variables["messageType"] = mt->name();
            variables["mask"] = "";
            for (auto j = 0; j < mt->field_count(); j++) {
                auto field = mt->field(j);
                variables["mask"] = variables["mask"] + recurecursiveGetTags("", field);
            }
            headerPrinter.Print(variables, CppMaskTemplateClass);
        }
		std::string nsPrefix = google::protobuf::compiler::cpp::Namespace(file);
		if (nsPrefix.find("::") == 0) {
			nsPrefix = nsPrefix.substr(2);
		}
		variables["ns"] = nsPrefix;
		sourcePrinter.Print(variables, CppTagsMethodReflectInclude);
        //服务名称反射
        for (auto i = 0; i < file->service_count(); i++) {
            auto svc = file->service(i);
			variables["serviceName"] = svc->name();
			sourcePrinter.Print(variables, CppTagsMethodReflectClassDefine);
            for(auto j=0; j<svc->method_count(); j++) {
                auto method = svc->method(j);
				variables["methodName"] = method->name();
				sourcePrinter.Print(variables, CppTagsMethodReflectMethodDefine);
            }
        }

        return true;
    }
private:
    std::string recurecursiveGetTags(const std::string& base, const google::protobuf::FieldDescriptor* field) const {
        auto& opts = field->options();
        std::string tags;
        if (opts.HasExtension(cpptags::log) && opts.GetExtension(cpptags::log)) {
            tags = base + field->name() + ",";
        }
        else if (field->is_repeated()) {
            // a repeated field is only allowed in the last position of a field mask.
            // @ref https://developers.google.com/protocol-buffers/docs/reference/java/com/google/protobuf/FieldMask
        }
        else if (field->type() == google::protobuf::FieldDescriptorProto::TYPE_MESSAGE
            || field->type() == google::protobuf::FieldDescriptorProto::TYPE_GROUP) {
            for (auto i = 0; i < field->message_type()->field_count(); i++) {
                tags += recurecursiveGetTags(base + field->name() + ".", field->message_type()->field(i));
            }
        }
        return tags;
    }
};

int main(int argc, char* argv[])
{
    CppTagsGenerator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}