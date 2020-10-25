#include "UndoSystem.h"

#include "OpenFunscripter.h"
#include <array>

std::array<std::string, (int32_t)StateType::TOTAL_UNDOSTATE_TYPES> stateStrings{
	"Add/Edit actions",
	"Add/Edit action",
	"Add action",

	"Remove actions",
	"Remove action",

	"Mouse moved actions",
	"Actions moved",

	"Cut selection",
	"Remove selection",
	"Paste selection",

	"Equalize",
	"Invert",
	"Isolate",

	"Top points",
	"Mid points",
	"Bottom points",

	"Generate actions",
	"Frame align",
	"Range extend",
};

void UndoSystem::SnapshotRedo(StateType type) noexcept
{
	RedoStack.emplace_back(type, OpenFunscripter::script().Data());
}

void UndoSystem::ShowUndoRedoHistory(bool* open)
{
	if (*open) {
		ImGui::SetNextWindowSizeConstraints(ImVec2(200, 100), ImVec2(200, 200));
		ImGui::Begin(UndoHistoryId, open, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
		//for(auto it = undoRedoSystem.RedoStack.rbegin(); it != undoRedoSystem.RedoStack.rend(); it++) {
		ImGui::TextDisabled("Redo stack");
		// TODO: get rid of the string comparison but keep the counting
		for (auto it = RedoStack.begin(); it != RedoStack.end(); it++) {
			int count = 1;
			auto copy_it = it;
			while (++copy_it != RedoStack.end() && copy_it->type == it->type) {
				count++;
			}
			it = copy_it - 1;

			ImGui::BulletText("%s (%d)", (*it).Message().c_str(), count);
		}
		ImGui::Separator();
		ImGui::TextDisabled("Undo stack");
		for (auto it = UndoStack.rbegin(); it != UndoStack.rend(); it++) {
			int count = 1;
			auto copy_it = it;
			while (++copy_it != UndoStack.rend() && copy_it->type == it->type) {
				count++;
			}
			it = copy_it - 1;

			ImGui::BulletText("%s (%d)", (*it).Message().c_str(), count);
		}
		ImGui::End();
	}
}

void UndoSystem::Snapshot(StateType type, bool clearRedo) noexcept
{
	UndoStack.emplace_back(type, OpenFunscripter::script().Data());

	if (UndoStack.size() > MaxScriptStateInMemory) {
		UndoStack.erase(UndoStack.begin()); // erase first action
	}
	
	// redo gets cleared after every snapshot
	if (clearRedo && !RedoStack.empty())
		ClearRedo();
}

void UndoSystem::Undo() noexcept
{
	if (UndoStack.empty()) return;
	SnapshotRedo(UndoStack.back().type);
	OpenFunscripter::script().rollback(UndoStack.back().Data()); // copy data
	UndoStack.pop_back(); // pop of the stack
}

void UndoSystem::Redo() noexcept
{
	if (RedoStack.empty()) return;
	Snapshot(RedoStack.back().type, false);
	OpenFunscripter::script().rollback(RedoStack.back().Data()); // copy data
	RedoStack.pop_back(); // pop of the stack
}

void UndoSystem::ClearHistory() noexcept
{
	UndoStack.clear();
	RedoStack.clear();
}

void UndoSystem::ClearRedo() noexcept
{
	RedoStack.clear();
}

const std::string& ScriptState::Message() const
{
	return stateStrings[(int32_t)type];
}
