#pragma once

#include <cstddef>
#include <ref_type/num_fields.hpp>
#include <source_location>
#include <string_view>
#include <tuple>
#include <utility>
#include <version>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-compat-extra-semi"
#endif

#include <ref_type/get_ith_field_from_fake_object.hpp>

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

namespace ref_type {

// 基于 Statement Expressions
// 实现的计算类型成员数量，只要求类型可以被结构化绑定，无需满足聚合类型

// 1. 探测器基类
template <typename... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
  explicit overloads(Ts... ts) : Ts(ts)... {}
};

// 2. 探测结构化绑定
#define BIND_CASE(N, ...)                      \
  [](auto &&u, int) -> decltype(({             \
    [[maybe_unused]] auto &&[__VA_ARGS__] = u; \
    (char (*)[N])0;                            \
  })) { return {}; }

// 3. 探测器定义
template <typename T>
struct binder_detector {
  static constexpr auto get() {
    return overloads(
        // clang-format off
        BIND_CASE(16, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15),
        BIND_CASE(15, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14),
        BIND_CASE(14, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13),
        BIND_CASE(13, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12),
        BIND_CASE(12, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11),
        BIND_CASE(11, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10),
        BIND_CASE(10, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9),
        BIND_CASE(9, f0, f1, f2, f3, f4, f5, f6, f7, f8),
        BIND_CASE(8, f0, f1, f2, f3, f4, f5, f6, f7),
        BIND_CASE(7, f0, f1, f2, f3, f4, f5, f6),
        BIND_CASE(6, f0, f1, f2, f3, f4, f5),
        BIND_CASE(5, f0, f1, f2, f3, f4),
        BIND_CASE(4, f0, f1, f2, f3),
        BIND_CASE(3, f0, f1, f2),
        BIND_CASE(2, f0, f1),
        BIND_CASE(1, f0),
        // clang-format on
        [](auto &&u, unsigned) -> char (*)[0] { return {}; });
  }
};

// 4. 计算成员数量
template <typename T>
constexpr unsigned num_members = []() -> unsigned {
  if constexpr (__is_empty(T)) {
    return 0;
  } else if constexpr (std::is_aggregate_v<T>) {
    return rfl::internal::num_fields<T>;
  } else {
    using detector_t = decltype(binder_detector<T>::get());
    using result_ptr_t = decltype(((detector_t *)0)->operator()(*(T *)0, 0));
    return sizeof(*(result_ptr_t)0);
  }
}();

// 获取成员名称

template <class T>
struct Wrapper {
  using Type = T;
  T v;
};

template <class T>
Wrapper(T) -> Wrapper<T>;

// This workaround is necessary for clang.
template <class T>
constexpr auto wrap(const T &arg) noexcept {
  return Wrapper{arg};
}

template <class T, auto ptr>
consteval auto get_field_name_str_view() {
#if __cpp_lib_source_location >= 201907L
  const auto func_name =
      std::string_view{std::source_location::current().function_name()};
#elif defined(_MSC_VER)
  // Officially, we only support MSVC versions that are modern enough to contain
  // <source_location>, but inofficially, this might work.
  const auto func_name = std::string_view{__FUNCSIG__};
#else
  const auto func_name = std::string_view{__PRETTY_FUNCTION__};
#endif
#if defined(__clang__)
  const auto split = func_name.substr(0, func_name.size() - 2);
  return split.substr(split.find_last_of(":.") + 1);
#elif defined(__GNUC__)
  const auto split = func_name.substr(0, func_name.size() - 2);
  return split.substr(split.find_last_of(":") + 1);
#elif defined(_MSC_VER)
  const auto split = func_name.substr(0, func_name.size() - 7);
  return split.substr(split.rfind("->") + 2);
#else
  static_assert(false,
                "You are using an unsupported compiler. Please use GCC, Clang "
                "or MSVC or switch to the rfl::Field-syntax.");
#endif
}

template <typename T, size_t N, int _i>
consteval auto get_nth_field_name_str_view() {
  return get_field_name_str_view<
      T,
#if defined(__clang__)
      wrap(
#endif
          rfl::internal::fake_object_helper<T, N>::template get_field<_i>()
#if defined(__clang__)
              )
#endif
      >();
}

#define DESTRUCT_CASE(N, I, t, ...) \
  else if constexpr (N == I) {      \
    auto &[__VA_ARGS__] = t;        \
    return std::tie(__VA_ARGS__);   \
  }

// 将成员解构成 tuple
template <typename T, size_t N>
auto StructAsTuple(T &t) {
  if constexpr (N == 0) {
    return std::tie();
  }
  // clang-format off
  DESTRUCT_CASE(N, 1, t, f0)
  DESTRUCT_CASE(N, 2, t, f0, f1)
  DESTRUCT_CASE(N, 3, t, f0, f1, f2)
  DESTRUCT_CASE(N, 4, t, f0, f1, f2, f3)
  DESTRUCT_CASE(N, 5, t, f0, f1, f2, f3, f4)
  DESTRUCT_CASE(N, 6, t, f0, f1, f2, f3, f4, f5)
  DESTRUCT_CASE(N, 7, t, f0, f1, f2, f3, f4, f5, f6)
  DESTRUCT_CASE(N, 8, t, f0, f1, f2, f3, f4, f5, f6, f7)
  DESTRUCT_CASE(N, 9, t, f0, f1, f2, f3, f4, f5, f6, f7, f8)
  DESTRUCT_CASE(N, 10, t, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9)
  DESTRUCT_CASE(N, 11, t, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10)
  DESTRUCT_CASE(N, 12, t, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11)
  DESTRUCT_CASE(N, 13, t, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12)
  DESTRUCT_CASE(N, 14, t, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13)
  DESTRUCT_CASE(N, 15, t, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14)
  DESTRUCT_CASE(N, 16, t, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15)
  // clang-format on
}

template <typename T, size_t N, typename F>
void VisitN(T &t, F &&callback) {
  auto refs = StructAsTuple<T, N>(t);

  auto iterate = [&]<size_t... Is>(std::index_sequence<Is...>) {
    (callback(get_nth_field_name_str_view<T, N, Is>(), std::get<Is>(refs)),
     ...);
  };

  iterate(std::make_index_sequence<N>{});
}

template <typename T, typename F>
void Visit(T &t, F &&callback) {
  constexpr auto N = num_members<T>;
  VisitN<T, N, F>(t, std::forward<F>(callback));
}

}  // namespace ref_type
