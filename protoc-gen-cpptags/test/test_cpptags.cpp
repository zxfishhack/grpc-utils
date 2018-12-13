#include "helloworld.cpptags.h"
#include "helloworld.pb.h"
#include "helloworld.grpc.pb.h"
#include <google/protobuf/util/field_mask_util.h>
#include <google/protobuf/util/json_util.h>

template<typename T>
void testTags(const T& t, bool override) {
	T m;
	if (override) {
		m.CopyFrom(t);
	} else {
		CppTagsMessageTypeMask<T>::MaskMessage(t, &m);
	}

	std::string v;
	google::protobuf::util::MessageToJsonString(m, &v);
	std::cout << (override ? "unmask" : "mask") << v << std::endl;
}

int main() {
    helloworld::DoYouLoveMe abc;
    abc.set_me("fish");
    abc.mutable_user()->set_name("jj");
    abc.mutable_user()->set_sex("girl");
    testTags(abc, false);
    helloworld::Response bcd;
    bcd.set_result("no");
    bcd.mutable_user()->set_name("bb");
    bcd.mutable_user()->set_sex("boy");
    testTags(bcd, false);
    helloworld::Response2 cde;
    cde.set_result("no");
    cde.add_user()->set_name("cc");
    cde.add_user()->set_name("dd");
    testTags(cde, false);

    
    std::cout << CppTagsMethodReflect<helloworld::HelloWorld::Service>::Name(&helloworld::HelloWorld::Service::SayYouLoveMe) << std::endl;
    std::cout << CppTagsMethodReflect<helloworld::HelloWorld::Service>::FullName(&helloworld::HelloWorld::Service::SayYouLoveMe) << std::endl;
	std::cout << CppTagsMethodReflect<helloworld::HelloWorld::AsyncService>::Name(&helloworld::HelloWorld::AsyncService::RequestSayYouLoveMe) << std::endl;
	std::cout << CppTagsMethodReflect<helloworld::HelloWorld::AsyncService>::FullName(&helloworld::HelloWorld::AsyncService::RequestSayYouLoveMe) << std::endl;

	std::cout << CppTagsMethodReflect<helloworld::HelloWorld2::Service>::Name(&helloworld::HelloWorld2::Service::SayYouLoveMe) << std::endl;
	std::cout << CppTagsMethodReflect<helloworld::HelloWorld2::Service>::FullName(&helloworld::HelloWorld2::Service::SayYouLoveMe) << std::endl;
	std::cout << CppTagsMethodReflect<helloworld::HelloWorld2::AsyncService>::Name(&helloworld::HelloWorld2::AsyncService::RequestSayYouLoveMe) << std::endl;
	std::cout << CppTagsMethodReflect<helloworld::HelloWorld2::AsyncService>::FullName(&helloworld::HelloWorld2::AsyncService::RequestSayYouLoveMe) << std::endl;
    return 0;
}
