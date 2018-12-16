#include "helloworld.pb.h"
#include <cpptags.h>

template<typename T>
void testTags(const T& t, bool override) {
  std::string output;
  CppTagsMessageTypeMask<T>::MaskMessageToJson(t, &output);
  std::cout << output << std::endl;
}

int main() {
  helloworld::DoYouLoveMe abc;
  abc.set_me("fish");
  abc.mutable_user()->set_name("jj");
  abc.mutable_user()->set_sex("girl");
  //testTags(abc, false);
  helloworld::Response bcd;
  bcd.set_result("no");
  auto usr = bcd.add_user();
  usr->set_name("bb");
  usr->set_sex("boy");
  testTags(bcd, false);
  helloworld::Response2 cde;
  cde.set_result("no");
  cde.add_user()->set_name("cc");
  cde.add_user()->set_name("dd");
  //testTags(cde, false);

  return 0;

}
