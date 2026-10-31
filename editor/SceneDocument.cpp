#include "SceneDocument.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <cmath>
#include <fstream>
#include <limits>
#include <set>
#include <stdexcept>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace EditorFiles {
namespace {
using Json = nlohmann::json;
constexpr std::size_t maxBytes = 4 * 1024 * 1024;
constexpr std::size_t maxObjects = 10000;
void validate(const Document& document) {
    if (document.objects.size() > maxObjects) throw std::runtime_error("Scene has too many objects (limit 10000)");
    std::set<Core::EntityId> ids;
    for (const auto& object : document.objects) {
        if (!object.id || object.id == std::numeric_limits<Core::EntityId>::max() || !ids.insert(object.id).second)
            throw std::runtime_error("Scene contains a reserved or duplicate object ID");
        if (object.name.size() > 1024) throw std::runtime_error("Object name exceeds 1024 bytes");
        if (object.mesh != "models/cube.obj" || object.material != "builtin/brick")
            throw std::runtime_error("Unsupported asset reference: this editor currently supports cube/brick objects");
        for (const auto& vector : {object.position, object.rotation, object.scale, object.velocity, object.acceleration})
            for (int axis=0; axis<3; ++axis)
                if (!std::isfinite(vector[axis])) throw std::runtime_error("Transforms and physics values must be finite");
        for (int axis=0; axis<3; ++axis)
            if (object.scale[axis] < 0.01f || object.scale[axis] > 1000)
                throw std::runtime_error("Scale must be between 0.01 and 1000");
        if (!std::isfinite(object.mass) || object.mass < 0) throw std::runtime_error("Mass must be finite and nonnegative");
    }
}
Json vectorJson(glm::vec3 v) { return Json::array({v.x,v.y,v.z}); }
float number(const Json& value) {
    if (!value.is_number()) throw std::runtime_error("Expected a number");
    auto result = value.get<float>();
    if (!std::isfinite(result)) throw std::runtime_error("Number exceeds supported finite range");
    return result;
}
glm::vec3 vectorFrom(const Json& value) {
    if (!value.is_array() || value.size()!=3) throw std::runtime_error("Expected a three-component vector");
    return {number(value[0]),number(value[1]),number(value[2])};
}
}
Document capture(const Core::Scene& scene) {
    Document result;
    for (const auto& entity : scene.getEntities()) {
        Object object;
        object.id=entity->id(); object.name=entity->name;
        object.position=entity->position; object.rotation=entity->rotation; object.scale=entity->scale;
        object.velocity=entity->velocity; object.acceleration=entity->acceleration;
        object.mass=entity->mass; object.gravity=entity->useGravity;
        result.objects.push_back(std::move(object));
    }
    return result;
}
std::string encode(const Document& document) {
    validate(document);
    Json root={{"format","fersa-scene"},{"version",1},{"objects",Json::array()}};
    for (const auto& o : document.objects) {
        root["objects"].push_back({{"id",o.id},{"name",o.name},{"mesh",o.mesh},{"material",o.material},
            {"position",vectorJson(o.position)},{"rotation",vectorJson(o.rotation)},{"scale",vectorJson(o.scale)},
            {"physics",{{"mass",o.mass},{"gravity",o.gravity},{"velocity",vectorJson(o.velocity)},{"acceleration",vectorJson(o.acceleration)}}}});
    }
    auto text = root.dump(2) + "\n";
    if (text.size()>maxBytes) throw std::runtime_error("Scene file exceeds 4 MiB");
    return text;
}
Document decode(const std::string& text) {
    if (text.size()>maxBytes) throw std::runtime_error("Scene file exceeds 4 MiB");
    // Stop deeply nested malformed input before it can exhaust the JSON parser's stack.
    auto root=Json::parse(text, [](int depth, Json::parse_event_t, Json&) {
        if (depth>32) throw std::runtime_error("Scene JSON is nested too deeply");
        return true;
    });
    if (!root.is_object() || root.at("format")!="fersa-scene" ||
        !root.at("version").is_number_integer() || root.at("version")!=1)
        throw std::runtime_error("Unsupported scene format or version");
    const auto& objects=root.at("objects");
    if (!objects.is_array() || objects.size()>maxObjects) throw std::runtime_error("Invalid scene object list");
    Document document;
    for (const auto& entry : objects) {
        Object o;
        const auto& id=entry.at("id");
        if (!id.is_number_unsigned()) throw std::runtime_error("Object ID must be a positive integer");
        o.id=id.get<Core::EntityId>(); o.name=entry.at("name").get<std::string>();
        o.mesh=entry.at("mesh").get<std::string>(); o.material=entry.at("material").get<std::string>();
        o.position=vectorFrom(entry.at("position")); o.rotation=vectorFrom(entry.at("rotation")); o.scale=vectorFrom(entry.at("scale"));
        const auto& physics=entry.at("physics");
        o.mass=number(physics.at("mass")); o.gravity=physics.at("gravity").get<bool>();
        o.velocity=vectorFrom(physics.at("velocity")); o.acceleration=vectorFrom(physics.at("acceleration"));
        document.objects.push_back(std::move(o));
    }
    validate(document);
    return document;
}
Document read(const std::filesystem::path& path) {
    if (std::filesystem::file_size(path)>maxBytes) throw std::runtime_error("Scene file exceeds 4 MiB");
    std::ifstream file(path,std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open scene file for reading");
    std::string text;
    char buffer[4096];
    while (file.read(buffer,sizeof(buffer)) || file.gcount()) {
        text.append(buffer,static_cast<std::size_t>(file.gcount()));
        if (text.size()>maxBytes) throw std::runtime_error("Scene file exceeds 4 MiB");
    }
    if (!file.eof()) throw std::runtime_error("Failed while reading scene file");
    return decode(text);
}
void write(const std::filesystem::path& path, const Document& document) {
    const auto text=encode(document); // Validate before touching the destination.
    if (path.empty() || path.filename().empty()) throw std::runtime_error("Choose a scene filename");
    auto temporary=path;
    temporary += ".tmp-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    try {
        std::ofstream file(temporary,std::ios::binary|std::ios::trunc);
        if (!file) throw std::runtime_error("Cannot create scene file; check the directory and permissions");
        file.write(text.data(),static_cast<std::streamsize>(text.size()));
        file.flush();
        if (!file) throw std::runtime_error("Failed to write scene file");
        file.close();
        if (!file) throw std::runtime_error("Failed to close scene file");
#ifdef _WIN32
        if (!MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
            throw std::runtime_error("Cannot replace scene file; the original was preserved");
#else
        std::filesystem::rename(temporary,path);
#endif
    } catch (...) {
        std::error_code ignored; std::filesystem::remove(temporary,ignored);
        throw;
    }
}
}
