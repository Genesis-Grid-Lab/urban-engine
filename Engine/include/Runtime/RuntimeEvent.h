#pragma once
#include "Core/Config.h"
#include "UE_Assert.h"
#include <type_traits>
#include <typeindex>

namespace UE {

struct RuntimeEvent {
  RuntimeEvent() = default;
  virtual ~RuntimeEvent() = default;
};

class RuntimeEventBus {
public:
  template <typename T, typename... Args> static void Emit(Args &&...args) {
    // UE_CORE_ASSERT(std::is_base_of_v<RuntimeEvent, T>::value "");
    static_assert(std::is_base_of_v<RuntimeEvent, T>);

    s_Events[typeid(T)].emplace_back(
        CreateScope<T>(std::forward<Args>(args)...));
  }

  template <typename T> static bool Poll(T &outEvent) {
    static_assert(std::is_base_of_v<RuntimeEvent, T>);

    auto it = s_Events.find(typeid(T));
    if (it == s_Events.end() || it->second.empty())
      return false;

    auto &vec = it->second;
    outEvent = std::move(*static_cast<T *>(vec.back().get()));
    vec.pop_back();

    return true;
  }

  static void Clear() { s_Events.clear(); }

private:
  static inline std::unordered_map<std::type_index,
                                   std::vector<Scope<RuntimeEvent>>>
      s_Events;
};

struct RuntimeStart : public RuntimeEvent {
  RuntimeStart() = default;
};

struct RuntimeEnd : public RuntimeEvent {
  RuntimeEnd() = default;
};

struct RuntimeSwitch : public RuntimeEvent {
  RuntimeSwitch() = default;
};

} // namespace UE