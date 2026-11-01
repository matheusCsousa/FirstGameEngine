#include "EditHistory.hpp"
namespace {
std::size_t bytes(const EditSnapshot& state) {
    std::size_t result=sizeof(state)+state.document.objects.size()*sizeof(EditorFiles::Object);
    for(const auto& object:state.document.objects) result+=object.name.size()+object.mesh.size()+object.material.size()+object.script.size();
    return result;
}
}
void EditHistory::reset(EditorFiles::Document document,Core::EntityId selected) {
    saved=document; states.clear(); states.push_back({std::move(document),selected}); cursor=0;
}
void EditHistory::trim() {
    std::size_t total=0; for(const auto& state:states) total+=bytes(state);
    while(states.size()>1 && (states.size()>maxStates || total>maxBytes)) {
        total-=bytes(states.front()); states.pop_front(); --cursor;
    }
}
bool EditHistory::commit(EditorFiles::Document document,Core::EntityId selected) {
    if(states.empty()) { reset(std::move(document),selected); return false; }
    if(states[cursor].document==document) { states[cursor].selected=selected; return false; }
    states.erase(states.begin()+static_cast<std::ptrdiff_t>(cursor+1),states.end());
    states.push_back({std::move(document),selected}); ++cursor; trim(); return true;
}
const EditSnapshot* EditHistory::undo() { if(!canUndo())return nullptr; return &states[--cursor]; }
const EditSnapshot* EditHistory::redo() { if(!canRedo())return nullptr; return &states[++cursor]; }
