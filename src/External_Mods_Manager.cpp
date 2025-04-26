#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include "Hooker.hpp"
#if defined(GEODE_IS_ANDROID)
#include <cxxabi.h>
#endif
#include <typeinfo>
#include <memory>
#include <string>
#include <unordered_map>

using namespace geode::prelude;


void runhooks(CCNode* Send,const std::string& layer) {
        for (Betterhook::HookBetter* hook : Betterhook::HookBetter::Hooks()) {
            if (hook->PutLayer() == layer) {
                hook->init(Send);
            }
        }
    }

    std::string demangle(const char* mangledName) {
        #if defined(GEODE_IS_ANDROID)
        static std::unordered_map<std::string, std::string> cache;
        

        auto it = cache.find(mangledName);
        if (it != cache.end()) {
            return it->second;
        }

        int status = 0;
        char* demangled = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);
        std::string result;
        
        if (status == 0) {
            result = demangled;
        } else {
            result = mangledName;
        }

        free(demangled);


        cache[mangledName] = result;
        return result;
        #else
            return "Not android";
        #endif
    }

std::string getclass(auto object) {
#if defined(GEODE_IS_ANDROID)
    return demangle(typeid(*object).name());
#else
    std::string className = typeid(*object).name();
     size_t pos = className.find("class ");
            if (pos != std::string::npos) {
                return className.substr(pos + 6);
            } else {
                return className;
            }
#endif


}

class $modify(CCDirector) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("CCDirector::willSwitchToScene", -999); 
    }

    void willSwitchToScene(CCScene* scene) {
        CCDirector::willSwitchToScene(scene);
        if (CCLayer* child = scene->getChildByType<CCLayer>(0)) {
                std::string fc = getclass(child);
                #ifndef GITHUB_ACTIONS
                    log::debug("{}", fc);
                #endif
                runhooks(child,fc);
        }
    }
};

