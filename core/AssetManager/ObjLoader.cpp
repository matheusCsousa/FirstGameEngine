#include "ObjLoader.hpp"
#include <fstream>
#include <sstream>
#include <charconv>
#include <cmath>
#include <stdexcept>
#include <limits>

namespace Core {
namespace {
int index(const std::string& text,std::size_t count,bool optional) {
    if(text.empty()&&optional)return -1;
    int value=0;auto parsed=std::from_chars(text.data(),text.data()+text.size(),value);
    if(parsed.ec!=std::errc{}||parsed.ptr!=text.data()+text.size()||value==0)throw std::runtime_error("Invalid OBJ index");
    const auto resolved=value>0 ? static_cast<long long>(value)-1 : static_cast<long long>(count)+value;
    if(resolved<0||resolved>=static_cast<long long>(count))throw std::runtime_error("OBJ index is out of range");
    return static_cast<int>(resolved);
}
bool finite(glm::vec3 v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
}
ObjData readOBJ(const std::filesystem::path& path) {
    if(std::filesystem::file_size(path)>64*1024*1024)throw std::runtime_error("OBJ exceeds 64 MiB");
    std::ifstream file(path);if(!file)throw std::runtime_error("Cannot open OBJ");
    std::vector<glm::vec3> positions,normals;std::vector<glm::vec2> uvs;ObjData result;
    result.bounds.min=glm::vec3(std::numeric_limits<float>::max());result.bounds.max=-result.bounds.min;
    std::string line;std::size_t lineNumber=0;
    while(std::getline(file,line)) {
        ++lineNumber;
        try {
            if(line.size()>256*1024)throw std::runtime_error("OBJ line is too long");
            std::istringstream stream(line);std::string type;stream>>type;
            if(type.empty()||type[0]=='#')continue;
            if(type=="v"||type=="vn") {
                glm::vec3 value{};if(!(stream>>value.x>>value.y>>value.z)||!finite(value))throw std::runtime_error("Invalid OBJ vector");
                if(type=="v"&&glm::any(glm::greaterThan(glm::abs(value),glm::vec3(1000000))))throw std::runtime_error("OBJ positions exceed supported range");
                auto& destination=type=="v"?positions:normals;
                if(destination.size()>=1000000)throw std::runtime_error("Too many OBJ vectors");
                destination.push_back(value);
            } else if(type=="vt") {
                glm::vec2 value{};if(!(stream>>value.x>>value.y)||!std::isfinite(value.x)||!std::isfinite(value.y))throw std::runtime_error("Invalid OBJ UV");
                if(uvs.size()>=1000000)throw std::runtime_error("Too many OBJ UVs");uvs.push_back(value);
            } else if(type=="f") {
                struct Corner {int p,t,n;};std::vector<Corner> face;std::string token;
                while(stream>>token) {
                    if(token[0]=='#')break;
                    if(face.size()>=4096)throw std::runtime_error("OBJ face exceeds 4096 corners");
                    auto a=token.find('/'),b=a==std::string::npos?a:token.find('/',a+1);
                    if(b!=std::string::npos&&token.find('/',b+1)!=std::string::npos)throw std::runtime_error("Malformed OBJ face");
                    face.push_back({index(token.substr(0,a),positions.size(),false),
                        a==std::string::npos?-1:index(token.substr(a+1,b==std::string::npos?b:b-a-1),uvs.size(),true),
                        b==std::string::npos?-1:index(token.substr(b+1),normals.size(),true)});
                }
                if(face.size()<3)throw std::runtime_error("OBJ face needs at least three corners");
                for(std::size_t i=1;i+1<face.size();++i) {
                    const auto normal=glm::cross(positions[face[i].p]-positions[face[0].p],positions[face[i+1].p]-positions[face[0].p]);
                    auto flat=glm::length(normal)>1e-8f?glm::normalize(normal):glm::vec3(0,1,0);
                    for(auto corner:{face[0],face[i],face[i+1]}) {
                        if(result.indices.size()>=1000000)throw std::runtime_error("OBJ exceeds one million emitted vertices");
                        const auto p=positions[corner.p];auto n=corner.n<0?flat:normals[corner.n];
                        n=glm::length(n)>1e-8f?glm::normalize(n):flat;
                        const auto uv=corner.t<0?glm::vec2(0):uvs[corner.t];
                        result.bounds.min=glm::min(result.bounds.min,p);result.bounds.max=glm::max(result.bounds.max,p);
                        result.indices.push_back(static_cast<unsigned int>(result.indices.size()));
                        result.vertices.insert(result.vertices.end(),{p.x,p.y,p.z,n.x,n.y,n.z,1,1,1,uv.x,uv.y});
                    }
                }
            }
        } catch(const std::exception& error){throw std::runtime_error("OBJ line "+std::to_string(lineNumber)+": "+error.what());}
    }
    if(!file.eof()||result.indices.empty())throw std::runtime_error("OBJ contains no renderable faces or could not be read");
    return result;
}
}
