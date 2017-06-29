#include <Windows.h>
#include <atomic>
#include <string>
#include <unordered_map>

struct ModuleAPIL0 {
    HANDLE CALLBACK (*load)(const char*);
    void CALLBACK (*unload)(HANDLE);
    void* CALLBACK (*getAPI)(HANDLE, const char*);
};

namespace {
    struct Module {
        HMODULE hdc = nullptr;
        void* (*getty)(const char*) = nullptr;
        int reference;
        std::string name;
    };

    using MMapT = std::unordered_map<std::string, Module>;
    using MMapIterT = MMapT::iterator;

    static std::unordered_map<std::string, Module> moduleMap;

    static std::string catModuleName(const std::string name) {
        return std::string("./Module/") + std::string(name) + ".dll";
    }

    static HANDLE CALLBACK moduleLoad(const char* nameI) {
        std::string name(nameI);
        MMapIterT find = moduleMap.find(name);
        if (find != moduleMap.end()) {
            find->second.reference++;
            return &find->second;
        }
        else{
            Module newRec;
            newRec.hdc = LoadLibraryA(catModuleName(name).c_str());
            if (newRec.hdc != INVALID_HANDLE_VALUE){
                newRec.getty = reinterpret_cast<void*(*)(const char*)>(GetProcAddress(newRec.hdc, "getty"));
                if (newRec.getty) {
                    newRec.name = name;
                    newRec.reference = 1;
                    moduleMap[name] = newRec;
                    return &moduleMap.find(name)->second;
                }
            }
        }
        return nullptr;
    }

    static void CALLBACK moduleUnload (HANDLE entry) {
        auto ent = reinterpret_cast<Module*>(entry);
        ent->reference--;
        if (ent.reference == 0) {

        }
    }

    static void* CALLBACK moduleGetAPI (HANDLE entry, const char* in) {
        return reinterpret_cast<Module*>(entry)->getty(in);
    }

    static ModuleAPIL0 moduleAPIL0 = {
        &moduleLoad, &moduleUnload, &moduleGetAPI
    };
}
