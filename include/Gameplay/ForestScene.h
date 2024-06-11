#ifndef __FORESTSCENE_H__
#define __FORESTSCENE_H__

#include "Core/GuiControlLabel.h"
#include "Core/Module.h"
#include "Core/GuiControl.h"
#include "Core/GuiControlButton.h"
#include "Gameplay/Entities/Player.h"
#include "Gameplay/Entities/Enemies/EnemyBoss.h"
#include "Gameplay/Scene.h"

struct SDL_Texture;

class ForestScene : public Scene
{
public:

	// Constructor
	ForestScene(const SString& name) : Scene(name) {}

	// Destructor
	virtual ~ForestScene();

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
	bool winCondition = false;
	Player* player;
	EnemyBoss* enemyboss;

private:
	SDL_Texture* img;
	SDL_Texture* loseScreenTex;
	float textPosX, textPosY = 0;
	uint texW, texH;
	uint windowW, windowH;
	GuiControlButton* gcButton;
	GuiControlLabel* gcScore;
	GuiControlLabel* gcLives;
	GuiControlLabel* gcTime;
	GuiControlPopUp* loseScreen = nullptr;

	bool paused = false;
	bool exitPressed = false;

	Timer* playingTime;
};

#endif // __FORESTSCENE_H__