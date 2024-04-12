#include "Core/App.h"
#include "Core/DialogManager.h"
#include "rapidcsv.h"
#include <Utils/Log.h>
#include "Core/Render.h"

DialogManager::DialogManager() {
    name.Create("dialog_manager");
}

DialogManager::DialogManager(bool startEnabled) : Module(startEnabled)
{
	name.Create("dialog_manager");
}

DialogManager::~DialogManager() {
    // Destructor implementation
}

bool DialogManager::Awake(pugi::xml_node& config) {

    LOG("Loading Dialog Manager");
    bool ret = true;

    // Load dialog data from CSV file using rapidcsv
    LoadDialogs("dialogs.csv", dialogs);

    /*
    // TODO remove this debug code
    for (const auto& pair : dialogs) {
        const Dialog& dialog = pair.second;
        std::cout << "Dialog ID: " << dialog.id << std::endl;
        std::cout << "Character: " << dialog.character << std::endl;
        std::cout << "ES: " << dialog.ES << std::endl;
        std::cout << "EN: " << dialog.EN << std::endl;
        std::cout << "CAT: " << dialog.CAT << std::endl;
        std::cout << "Type: " << (dialog.type == DialogType::DIALOG ? "DIALOG" : "CHOICE") << std::endl;
        std::cout << "Choices: ";
        std::cout << std::endl << "-------------------" << std::endl;
    }
    */

    return ret;
}

bool DialogManager::Start() {
    // Initialization code
    StartDialog(1);

    indexText = 1;

    return true;
}

bool DialogManager::PreUpdate() {
    // Pre-update code
    return true;
}

bool DialogManager::Update(float dt) {
    // Update code
    ShowDialog();

    if (app->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN)
    {
        NextDialog();
        StartDialog(currentDialogId);
    }

    LOG("update");

    return true;
}

bool DialogManager::PostUpdate() {
    // Post-update code
    return true;
}

bool DialogManager::CleanUp() {
    // Cleanup code
    dialogs.clear();
    return true;
}

bool DialogManager::LoadState(pugi::xml_node&) {
    // Load state code
    return true;
}

bool DialogManager::SaveState(pugi::xml_node&) const {
    // Save state code
    return true;
}

void DialogManager::StartDialog(int dialogId) {
    currentDialogId = dialogId;
    // Set the current dialog text to the first line of the dialog
    currentDialogLine = dialogs.at(dialogId).ES;

    currentDialog = &dialogs.at(currentDialogId);
}

void DialogManager::NextDialog() {
    // Get the current dialog
    currentDialog = &dialogs.at(currentDialogId);
    // Advance to the next line or choice in the dialog
    // Set the current dialog text to the next line or choice
    if (currentDialog->type== DialogType::DIALOG) {
        // If the dialog is a dialog, advance to the next line
        // If there are no more lines, end the dialog
        // Otherwise, set the current dialog text to the next line
        currentDialogId++;
        indexText = 1;
        currentDialogType = DialogType::DIALOG;
    } else if (currentDialog->type == DialogType::CHOICE) {
        // If the dialog is a choice, advance to the next choice
        // If there are no more choices, end the dialog
        // Otherwise, set the current dialog text to the next choice
        currentDialogId++;
        indexText = 1;
        currentDialogType = DialogType::CHOICE;
    }
}

void DialogManager::EndDialog() {
    // Clear the current dialog
    currentDialogId = -1;
    currentDialogLine.clear();
    // TODO clean dialogs map
}

void DialogManager::ShowDialog() {
    if (currentDialogId != -1) {

        SDL_Texture* texture = nullptr;

        string actualText = currentDialogLine.substr(0, indexText);

        // Render the current dialog text on the screen

        if (currentDialog->type == DialogType::DIALOG) {
            texture = CreateTextTexture(font, actualText.c_str(), textColor, 200/*TODO text bound widht*/);
            app->render->DrawTexture(texture, 0/*TODO pos x*/, 0/*TODO pos y*/, 0, 0);
            SDL_DestroyTexture(texture);
        } else if (currentDialog->type == DialogType::CHOICE) {
            texture = CreateTextTexture(font, actualText.c_str(), textColor, 200/*TODO text bound widht*/);
            app->render->DrawTexture(texture, 0/*TODO pos x*/, 0/*TODO pos y*/, 0, 0);
            SDL_DestroyTexture(texture);

            ListItem<int>* choice = currentDialog->choices.start;

            while (currentDialog->choices.Count() > 0)
            {
                string currentChoice = dialogs.at(choice->data).ES;
                texture = CreateTextTexture(font, currentChoice.c_str(), textColor, 200/*TODO text bound widht*/);
                app->render->DrawTexture(texture, 0/*TODO pos x*/, 0/*TODO pos y*/, 0, 0);
                SDL_DestroyTexture(texture);

                choice = choice->next;
            }

        }

        // TODO draw character texture
        //if (dialog->character != nullptr) {
        //    app->render->DrawTexture(dialog->character, 100, 100, 0, 0);
        //}

        if (actualText.size() < currentDialogLine.size()) {

            if (charTimer.ReadMSec() >= charTimeMS) {
                indexText++;
                charTimer.Start();
            }

        }

    }
}

SDL_Texture* DialogManager::CreateTextTexture(TTF_Font* font, const char* text, SDL_Color color, int textBoundWidth)
{
    SDL_Surface* textSurface = nullptr;
    SDL_Texture* textTexture = nullptr;

    textSurface = TTF_RenderUTF8_Blended_Wrapped(app->render->font, text, color, textBoundWidth);
    textTexture = SDL_CreateTextureFromSurface(app->render->renderer, textSurface);

    SDL_FreeSurface(textSurface);

    return textTexture;
}

bool DialogManager::LoadDialogs(string path, map<int, Dialog>& dialogs)
{
    bool ret = true;

    try {
        rapidcsv::Document doc(path, rapidcsv::LabelParams(0, -1));

        vector<int> ids = doc.GetColumn<int>("id");
        vector<string> characters = doc.GetColumn<string>("character");
        vector<string> ESs = doc.GetColumn<string>("ES");
        vector<string> ENs = doc.GetColumn<string>("EN");
        vector<string> CATs = doc.GetColumn<string>("CAT");
        vector<string> types = doc.GetColumn<string>("type");
        vector<string> choices = doc.GetColumn<string>("choices");

        for (size_t i = 0; i < characters.size(); i++) {
            Dialog dialog;
            dialog.id = ids[i];
            dialog.character = characters[i];
            dialog.ES = ESs[i];
            dialog.EN = ENs[i];
            dialog.CAT = CATs[i];

            // Check the type and parse it to the DialogType enum
            if (types[i] == "D") {
                dialog.type = DialogType::DIALOG;
            } else if (types[i] == "C") {
                dialog.type = DialogType::CHOICE;
            } else {
                std::cerr << "Invalid dialog type: " << types[i] << std::endl;
                continue;
            }
            string token;
            stringstream ss(choices[i]);

            if (choices[i] != "") {
                while (getline(ss, token, ',')) {
                    dialog.choices.Add(stoi(token));
                }
            }

            dialogs[dialog.id] = dialog;
        }
    } catch (const std::ios_base::failure& e) {
        std::cerr << "Failed to open or read file at " << path << ": " << e.what() << std::endl;
        LOG_ERROR("Failed to open or read file at %s : %s", path.c_str(), e.what());
        ret = false;
    }

    return ret;
}