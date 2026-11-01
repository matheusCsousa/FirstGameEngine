#pragma once
#include <filesystem>
#include <string>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
namespace AssetPaths {
inline std::string text(const std::filesystem::path& path){auto value=path.u8string();return {value.begin(),value.end()};}
inline std::filesystem::path path(const std::string& value){return std::filesystem::path(std::u8string(value.begin(),value.end()));}
inline std::string extension(const std::string& name){auto result=text(path(name).extension());std::transform(result.begin(),result.end(),result.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});return result;}
inline bool reference(const std::string& value){
    if(value.size()>512||!value.starts_with("assets/")||value.find('\\')!=std::string::npos||value.find(':')!=std::string::npos||value.find('\0')!=std::string::npos)return false;
    for(const auto& part:path(value))if(part==".."||part=="."||part.empty())return false;
    return value.back()!='/';
}
inline bool within(const std::filesystem::path& root,const std::filesystem::path& target){
    auto a=std::filesystem::weakly_canonical(root),b=std::filesystem::weakly_canonical(target);
    auto i=a.begin(),j=b.begin();for(;i!=a.end();++i,++j)if(j==b.end()||*i!=*j)return false;return j!=b.end();
}
inline std::filesystem::path resolve(const std::filesystem::path& root,const std::string& referenceValue){
    if(!reference(referenceValue))throw std::runtime_error("Invalid project asset reference");
    auto result=root/path(referenceValue);
    if(!within(root,result))throw std::runtime_error("Asset resolves outside the project folder");
    return result;
}
inline std::string readScript(const std::filesystem::path& file){
    auto size=std::filesystem::file_size(file);if(size>256*1024)throw std::runtime_error("Script exceeds 256 KiB");
    std::ifstream stream(file,std::ios::binary);std::string result(static_cast<std::size_t>(size),'\0');
    if(!stream.read(result.data(),static_cast<std::streamsize>(size)))throw std::runtime_error("Cannot read script");return result;
}
}
