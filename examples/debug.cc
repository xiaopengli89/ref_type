#include <ref_type/ref_type.h>

#include <iostream>
#include <string>

struct Foo {
  int bar;
  std::string baz;
};

int main() {
  Foo foo{10, "bazzzz"};
  ref_type::Visit(foo, []<auto field_info>(auto& value) {
    std::cout << field_info.name() << ": " << value << "\n";
    if constexpr (field_info.name() == "bar") {
      value = 99;
    } else if constexpr (field_info.name() == "baz") {
      value = "baiiii";
    }
  });
  std::cout << "bar: " << foo.bar << "\n";
  std::cout << "baz: " << foo.baz << "\n";
  return 0;
}
