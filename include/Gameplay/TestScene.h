#ifndef __TESTSCENE_H__
#define __TESTSCENE_H__

#include "Core/GuiControlLabel.h"
#include "Core/Module.h"
#include "Gameplay/Entities/Player.h"
#include "Gameplay/Entities/Enemies/EnemyBoss.h"
#include "Gameplay/Entities/Enemies/MiniSpider.h"
#include "Gameplay/Entities/Enemies/Wasp.h"
#include "Core/GuiControl.h"
#include "Core/GuiControlButton.h"
#include "Gameplay/Scene.h"

struct SDL_Texture;

class TestScene : public Scene
{
public:

	// Constructor
	TestScene(const SString& name) : Scene(name) {}

	// Destructor
	virtual ~TestScene();

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
	MiniSpider* minispider;
	Wasp* wasp;

private:
	SDL_Texture* img;
	float textPosX, textPosY = 0;
	uint texW, texH;
	uint windowW, windowH;
	GuiControlButton* gcButton;
	GuiControlLabel* gcScore;
	GuiControlLabel* gcLives;
	GuiControlLabel* gcTime;

	bool exitPressed = false;

	Timer* playingTime;
};

#endif // __TESTSCENE_H__