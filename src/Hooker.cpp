#include "Hooker.hpp"
#include <Geode/Geode.hpp>

static std::vector<Betterhook::HookBetter*> ListHooks;
  
const std::vector<Betterhook::HookBetter*>& Betterhook::HookBetter::Hooks() {
    return ListHooks;
}

void Betterhook::HookBetter::registerHook(Betterhook::HookBetter* hook) {
     ListHooks.push_back(hook);
 }

