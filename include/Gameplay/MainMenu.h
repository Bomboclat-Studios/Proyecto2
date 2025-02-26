#ifndef __MainMenu_H__
#define __MainMenu_H__

#include "Gameplay/Scene.h"
#include "Core/GuiControl.h"
#include "Core/GuiControlButton.h"
#include "Core/GuiControlPopUp.h"
#include "Core/GuiControlSlider.h"
#include "Core/GuiControlCheckbox.h"
#include "Core/GuiControlDropDownBox.h"

struct SDL_Texture;

class MainMenu : public Scene
{
public:

	// Constructor
	MainMenu(const SString& name) : Scene(name) {}

	// Destructor
	virtual ~MainMenu();

	// Called before the first frame
	bool Enter() override;

	// Called before all Updates
	bool PreUpdate() override;

	// Called each loop iteration
	bool Update(float dt) override;

	// Called before all Updates
	bool PostUpdate() override;

	bool Exit() override;

	// Called before quitting
	bool CleanUp() override;

	// Handles multiple Gui Event methods
	bool OnGuiMouseClickEvent(GuiControl* control) override;

	void RenderGUI();

public:

private:
	SDL_Texture* img;
	float textPosX, textPosY = 0;
	uint texW, texH;
	uint windowW, windowH;
	GuiControlButton* playButton;
	GuiControlButton* optionsButton;
	GuiControlButton* creditsButton;
	GuiControlButton* loadButton;
	GuiControlButton* exitButton;
	GuiControlPopUp* popUpOptions = nullptr;
	GuiControlPopUp* popUpLoad = nullptr;
	GuiControlButton* crossOButton = nullptr;
	GuiControlSlider* music = nullptr;
	GuiControlSlider* fx = nullptr;
	GuiControlCheckbox* vsync = nullptr;
	GuiControlDropDownBox* screenMode = nullptr;
	GuiControlButton* fullScreen = nullptr;
	GuiControlButton* borderless = nullptr;
	GuiControlButton* fullScreenWindow = nullptr;

	GuiControlDropDownBox* resolution = nullptr;
	GuiControlButton* res7680x4320 = nullptr;
	GuiControlButton* res3840x2160 = nullptr;
	GuiControlButton* res2560x1440 = nullptr;
	GuiControlButton* res1920x1200 = nullptr;
	GuiControlButton* res1920x1080 = nullptr;
	GuiControlButton* res1600x900 = nullptr;
	GuiControlButton* res1280x720 = nullptr;
	GuiControlButton* res1024x768 = nullptr;
	GuiControlButton* res800x600 = nullptr;
	GuiControlButton* res640x480 = nullptr;

	GuiControlButton* saveSlot1 = nullptr;
	GuiControlButton* saveSlot2 = nullptr;
	GuiControlButton* saveSlot3 = nullptr;
	GuiControlButton* saveSlot4 = nullptr;
	GuiControlButton* saveSlot5 = nullptr;
	
	bool exitPressed = false;

	const char* texturePath;
	SDL_Texture* gameTitle = nullptr;
	SDL_Texture* settingsTexture = nullptr;

	//Audio fx
	int FxId;

};

#endif // __MainMenu_H__