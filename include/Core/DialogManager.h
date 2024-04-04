#ifndef DIALOG_MANAGER_H
#define DIALOG_MANAGER_H

#include "Core/Module.h"
#include "Utils/List.h"
#include <SDL_ttf.h>
#include <map>
#include <string>

using namespace std;

enum DialogType {
    DIALOG,
    CHOICE
};

struct Dialog {
    int id;
    string character;
    string ES;
    string EN;
    string CAT;
    DialogType type;
    List<int> choices;
};

class DialogManager : public Module {
public:

    DialogManager();

    DialogManager(bool startEnabled);

    // Destructor
    virtual ~DialogManager();

    // Called before render is available
    bool Awake(pugi::xml_node config);

    // Called before the first frame
    bool Start();

    // Called each loop iteration
    bool PreUpdate();

    // Called each loop iteration
    bool Update(float dt);

    // Called each loop iteration
    bool PostUpdate();

    // Called before quitting
    bool CleanUp();

    // Load / Save
    bool LoadState(pugi::xml_node&);
    bool SaveState(pugi::xml_node&) const;

    // Dialog methods
    void StartDialog(int dialogId);
    void NextDialog();
    void EndDialog();

private:
    map<int, string> dialogs;   // Map of dialog ID to Dialog data
    int currentDialogId;        // Current dialog ID

};

#endif // DIALOG_MANAGER_H