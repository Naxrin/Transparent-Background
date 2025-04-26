
#pragma once
#include <Geode/Geode.hpp>


#define REGISTER_HookBetter(Class) $execute { Betterhook::HookBetter::registerHook<Class>(); }
namespace Betterhook {
 class HookBetter {
    public:
     static void registerHook(HookBetter* hook);
     template<typename T, typename = std::enable_if_t<std::is_base_of_v<HookBetter, T>>>
     static void registerHook() { registerHook(new T()); }
     static const std::vector<HookBetter*>& Hooks();
     virtual void init(cocos2d::CCNode* Layer) = 0;
     virtual const char* PutLayer() const = 0;
 };
}
