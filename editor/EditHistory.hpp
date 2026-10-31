#pragma once
#include "SceneDocument.hpp"
#include <deque>

struct EditSnapshot { EditorFiles::Document document; Core::EntityId selected=0; };
class EditHistory {
    std::deque<EditSnapshot> states;
    std::size_t cursor=0;
    EditorFiles::Document saved;
    void trim();
public:
    static constexpr std::size_t maxStates=100;
    static constexpr std::size_t maxBytes=32*1024*1024;
    void reset(EditorFiles::Document document,Core::EntityId selected=0);
    bool commit(EditorFiles::Document document,Core::EntityId selected);
    bool canUndo() const { return !states.empty() && cursor>0; }
    bool canRedo() const { return !states.empty() && cursor+1<states.size(); }
    const EditSnapshot* undo();
    const EditSnapshot* redo();
    void markSaved(const EditorFiles::Document& document) { saved=document; }
    bool isDirty(const EditorFiles::Document& document) const { return document!=saved; }
    std::size_t size() const { return states.size(); }
};
