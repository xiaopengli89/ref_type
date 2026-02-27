#include <ref_type/ref_type.h>

#include <iostream>
#include <string>

struct Foo {
  int bar;
  std::string baz;
};

int main() {
  Foo foo{10, "bazzzz"};
  ref_type::Visit(foo, [](std::string_view name, auto& value) {
    std::cout << name << ": " << value << "\n";
  });
  return 0;
}
