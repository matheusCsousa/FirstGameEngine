#include "ScriptRuntime.hpp"
#include "AssetPaths.hpp"
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <stdexcept>

struct ScriptRuntime::Instance {
    ScriptRuntime* owner=nullptr;
    lua_State* state=nullptr;
    Core::EntityId id=0;
    std::string label,source;
    std::size_t used=0;
    int budget=0,definition=LUA_NOREF,self=LUA_NOREF;
    bool disabled=false;
    Core::Entity* entity=nullptr;
    float dt=0;
    const char* callback=nullptr;
    static void* allocate(void* user,void* pointer,std::size_t oldSize,std::size_t size) {
        auto* instance=static_cast<Instance*>(user);if(!pointer)oldSize=0;
        if(!size){std::free(pointer);instance->used-=std::min(instance->used,oldSize);return nullptr;}
        const auto remaining=instance->used-std::min(instance->used,oldSize);
        if(size>4*1024*1024 || remaining>4*1024*1024-size)return nullptr;
        auto* result=std::realloc(pointer,size);if(result)instance->used=remaining+size;return result;
    }
    static void hook(lua_State* L,lua_Debug*) {
        void* user;lua_getallocf(L,&user);
        if(--static_cast<Instance*>(user)->budget<=0)luaL_error(L,"script instruction limit exceeded");
    }
    static int print(lua_State* L) {
        void* user;lua_getallocf(L,&user);auto* instance=static_cast<Instance*>(user);
        if(!instance->owner||instance->owner->messages.size()>=100)return 0;
        char buffer[1024]{};std::size_t length=0;
        for(int i=1;i<=lua_gettop(L)&&length<sizeof(buffer)-1;++i) {
            const char* value=lua_tostring(L,i);if(!value)value=lua_typename(L,lua_type(L,i));
            auto count=std::min(std::strlen(value),sizeof(buffer)-1-length);std::memcpy(buffer+length,value,count);length+=count;
            if(length<sizeof(buffer)-1)buffer[length++]=' ';
        }
        try{instance->owner->messages.push_back(instance->label+": "+std::string(buffer,length));}catch(...){}
        return 0;
    }
    static int setup(lua_State* L) {
        const luaL_Reg libraries[]={{"_G",luaopen_base},{LUA_MATHLIBNAME,luaopen_math},{LUA_STRLIBNAME,luaopen_string},{LUA_TABLIBNAME,luaopen_table},{LUA_UTF8LIBNAME,luaopen_utf8},{nullptr,nullptr}};
        for(auto* library=libraries;library->name;++library){luaL_requiref(L,library->name,library->func,1);lua_pop(L,1);}
        for(const char* name:{"dofile","loadfile","load","collectgarbage","setmetatable","getmetatable","pcall","xpcall"}){lua_pushnil(L);lua_setglobal(L,name);}
        lua_pushcfunction(L,print);lua_setglobal(L,"print");return 0;
    }
    static int load(lua_State* L) {
        auto* instance=static_cast<Instance*>(lua_touserdata(L,1));lua_settop(L,0);
        if(luaL_loadbufferx(L,instance->source.data(),instance->source.size(),instance->label.c_str(),"t")!=LUA_OK)return lua_error(L);
        lua_call(L,0,1);luaL_checktype(L,-1,LUA_TTABLE);
        for(const char* name:{"on_start","on_update"}) {
            lua_getfield(L,-1,name);if(!lua_isnil(L,-1)&&!lua_isfunction(L,-1))return luaL_error(L,"Callbacks must be functions");lua_pop(L,1);
        }
        instance->definition=luaL_ref(L,LUA_REGISTRYINDEX);
        lua_newtable(L);instance->self=luaL_ref(L,LUA_REGISTRYINDEX);return 0;
    }
    static void putVector(lua_State* L,const char* name,glm::vec3 value) {
        lua_newtable(L);int axis=0;for(const char* key:{"x","y","z"}){lua_pushnumber(L,value[axis++]);lua_setfield(L,-2,key);}lua_setfield(L,-2,name);
    }
    static glm::vec3 getVector(lua_State* L,const char* name,bool scale=false) {
        glm::vec3 result{};lua_getfield(L,-1,name);luaL_checktype(L,-1,LUA_TTABLE);int axis=0;
        for(const char* key:{"x","y","z"}) {
            lua_getfield(L,-1,key);const double value=luaL_checknumber(L,-1);lua_pop(L,1);
            if(!std::isfinite(value)||std::abs(value)>10000000||(scale&&(value<0.01||value>1000)))luaL_error(L,"Invalid transform/velocity value");
            result[axis++]=static_cast<float>(value);
        }
        lua_pop(L,1);return result;
    }
    static int invoke(lua_State* L) {
        auto* instance=static_cast<Instance*>(lua_touserdata(L,1));lua_settop(L,0);auto* entity=instance->entity;
        lua_rawgeti(L,LUA_REGISTRYINDEX,instance->self);
        putVector(L,"position",entity->position);putVector(L,"rotation",entity->rotation);putVector(L,"scale",entity->scale);putVector(L,"velocity",entity->velocity);
        lua_pushlstring(L,entity->name.data(),entity->name.size());lua_setfield(L,-2,"name");
        lua_rawgeti(L,LUA_REGISTRYINDEX,instance->definition);lua_getfield(L,-1,instance->callback);
        if(!lua_isnil(L,-1)) {lua_pushvalue(L,1);lua_pushnumber(L,instance->dt);lua_call(L,2,0);}else lua_pop(L,1);
        lua_pop(L,1);
        const auto position=getVector(L,"position"),rotation=getVector(L,"rotation"),scale=getVector(L,"scale",true),velocity=getVector(L,"velocity");
        entity->position=position;entity->rotation=rotation;entity->scale=scale;entity->velocity=velocity;return 0;
    }
    Instance() {state=lua_newstate(allocate,this);if(!state)throw std::runtime_error("Cannot create Lua state");lua_sethook(state,hook,LUA_MASKCOUNT,1000);}
    ~Instance(){if(state)lua_close(state);}
    bool call(lua_CFunction function,bool argument=true) {
        budget=200;lua_settop(state,0);lua_pushcfunction(state,function);if(argument)lua_pushlightuserdata(state,this);
        const auto result=lua_pcall(state,argument?1:0,0,0);
        if(result!=LUA_OK){disabled=true;if(owner&&owner->messages.size()<100){auto* message=lua_tostring(state,-1);owner->messages.push_back(label+": "+(message?std::string(message,std::min<std::size_t>(std::strlen(message),2048)):std::string("Lua error")));}lua_settop(state,0);return false;}return true;
    }
};
ScriptRuntime::ScriptRuntime()=default;
ScriptRuntime::~ScriptRuntime()=default;
std::string ScriptRuntime::checkSyntax(const std::string& source) {
    if(source.size()>256*1024)return "Script exceeds 256 KiB";
    Instance instance;auto* L=instance.state;
    if(luaL_loadbufferx(L,source.data(),source.size(),"script","t")!=LUA_OK){auto* error=lua_tostring(L,-1);return error?error:"Invalid Lua source";}return {};
}
void ScriptRuntime::start(Core::Scene& scene,const std::filesystem::path& projectRoot) {
    stop();messages.clear();std::size_t count=0;
    for(const auto& entity:scene.getEntities())if(!entity->scriptAsset.empty()) {
        try {
            if(++count>64)throw std::runtime_error("Preview supports up to 64 scripted objects");
            auto instance=std::make_unique<Instance>();instance->owner=this;instance->id=entity->id();instance->label=entity->name+" / "+entity->scriptAsset;
            instance->source=AssetPaths::readScript(AssetPaths::resolve(projectRoot,entity->scriptAsset));
            if(instance->call(Instance::setup,false)&&instance->call(Instance::load)) {
                instance->entity=entity.get();instance->callback="on_start";instance->call(Instance::invoke);
            }
            instances.push_back(std::move(instance));
        }catch(const std::exception& error){if(messages.size()<100)messages.push_back(entity->name+": "+error.what());}
    }
}
void ScriptRuntime::update(Core::Scene& scene,float dt) {
    for(auto& instance:instances)if(!instance->disabled)if(auto* entity=scene.findEntity(instance->id)) {
        instance->entity=entity;instance->dt=dt;instance->callback="on_update";instance->call(Instance::invoke);
    }
}
void ScriptRuntime::stop(){instances.clear();}
