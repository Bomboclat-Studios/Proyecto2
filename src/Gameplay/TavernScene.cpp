#include "Core/App.h"
#include "Core/Input.h"
#include "Utils/SString.h"
#include "Core/Render.h"
#include "Core/Textures.h"
#include "Utils/Timer.h"
#include "Core/Window.h"
#include "Gameplay/TavernScene.h"
#include "Gameplay/Entities/Npcs/Tabernero.h"
#include "Gameplay/Entities/Npcs/Npc.h"
#include "Gameplay/Entities/PlayerPointAndClick.h"
#include "Core/Map.h"
#include "Core/SceneManager.h"
#include "Utils/Log.h"
#include "Core/GuiControl.h"
#include "Core/GuiManager.h"
  
#include <box2d/b2_body.h>
#include <tracy/Tracy.hpp>

// Destructor
TavernScene::~TavernScene()
{}

// Called before the first frame
bool TavernScene::Enter()
{
	// iterate all objects in the testscene
	// Check https://pugixml.org/docs/quickstart.html#access

	if (parameters.child("player")) {
		player = (Player*)app->entityManager->CreateEntity(EntityType::PLAYER);
		player->parameters = parameters.child("player");
		player->Enable();
	}

	playerPointAndClick = new PlayerPointAndClick();
	playerPointAndClick->parameters = parameters.child("player");
	app->entityManager->AddEntity((Entity*)playerPointAndClick);
	playerPointAndClick->Enable();

	if (parameters.child("map")) {
		//Get the map name from the config file and assigns the value in the module
		app->map->name = parameters.child("map").attribute("name").as_string();
		app->map->path = parameters.child("map").attribute("path").as_string();
	}

	if (parameters.child("camera")) {
		//Get the map name from the config file and assigns the value in the module
		app->render->camera.x = parameters.child("camera").attribute("x").as_int();
		app->render->camera.y = parameters.child("camera").attribute("y").as_int();
	}

	if (parameters.child("npcs"))
	{
		pugi::xml_node npcs = parameters.child("npcs");

		for (pugi::xml_node npcsNode = npcs.child("npc"); npcsNode; npcsNode = npcsNode.next_sibling("npc"))
		{
			Npc* npcs = new Npc();
			app->entityManager->AddEntity(npcs);
			npcs->parameters = npcsNode;
			npcs->Start();
		}
	}

	if (parameters.child("tabernero")) {
		Tabernero* tabernero = new Tabernero();
		app->entityManager->AddEntity(tabernero);
		tabernero->parameters = parameters.child("tabernero");
		tabernero->Start();
	}

	//app->physics->Enable();
	app->map->Enable();
	app->entityManager->Enable();

	app->render->camera.target = playerPointAndClick;
	app->render->camera.useInterpolation = true;
	app->render->camera.lerpSpeed = 4.0f;
	app->render->camera.offset = { 0,0 };

	playingTime = new Timer();

	//Get the size of the window
	app->win->GetWindowSize(windowW, windowH);

	//Get the size of the texture
	//app->tex->GetSize(img, texW, texH);

	//Pause Menu UI
	gcResume = (GuiControlButton*)app->guiManager->CreateGuiControl(GuiControlType::BUTTON, 6, "Resume", { (int)windowW / 2 - 175, (int)windowH / 2 - 100, 300, 50 }, this);
	gcResume->SetObserver(this);
	gcResume->state = GuiControlState::DISABLED;

	gcSave = (GuiControlButton*)app->guiManager->CreateGuiControl(GuiControlType::BUTTON, 10, "Save", { (int)windowW / 2 - 175, (int)windowH / 2 - 50, 300, 50 }, this);
	gcSave->SetObserver(this);
	gcSave->state = GuiControlState::DISABLED;

	gcSettings = (GuiControlButton*)app->guiManager->CreateGuiControl(GuiControlType::BUTTON, 7, "Settings", { (int)windowW / 2 - 175, (int)windowH / 2, 300, 50 }, this);
	gcSettings->SetObserver(this);
	gcSettings->state = GuiControlState::DISABLED;

	gcBackToTitle = (GuiControlButton*)app->guiManager->CreateGuiControl(GuiControlType::BUTTON, 8, "Back to Title", { (int)windowW / 2 - 175, (int)windowH / 2 + 50, 300, 50 }, this);
	gcBackToTitle->SetObserver(this);
	gcBackToTitle->state = GuiControlState::DISABLED;

	gcExit = (GuiControlButton*)app->guiManager->CreateGuiControl(GuiControlType::BUTTON, 9, "Exit", { (int)windowW / 2 - 175, (int)windowH / 2 + 100, 300, 50 }, this);
	gcExit->SetObserver(this);
	gcExit->state = GuiControlState::DISABLED;

	cauldronOpen = (GuiControlButton*)app->guiManager->CreateGuiControl(GuiControlType::BUTTON, 16, "Open", { 500, 600, 150, 50 }, this);
	cauldronOpen->SetObserver(this);
	cauldronOpen->state = GuiControlState::DISABLED; 

	cauldronTex = app->tex->Load("Assets/Textures/Potions/Cauldron/Cauldron.png");
	cauldronSelectTex = app->tex->Load("Assets/Textures/Potions/Cauldron/CauldronSelect.png");

	CeleritaPotionTex = app->tex->Load("Assets/Textures/Potions/CreatePotion/CrafteableCeleritaPotion.png");
	NotCeleritaPotionTex = app->tex->Load("Assets/Textures/Potions/CreatePotion/NotCrafteableCeleritaPotion.png");

	EtherPotionTex = app->tex->Load("Assets/Textures/Potions/CreatePotion/CrafteableEtherPotion.png");
	NotEtherPotionTex = app->tex->Load("Assets/Textures/Potions/CreatePotion/NotCrafteableEtherPotion.png");

	VitaPotionTex = app->tex->Load("Assets/Textures/Potions/CreatePotion/CrafteableVitaPotion.png");
	NotVitaPotionTex = app->tex->Load("Assets/Textures/Potions/CreatePotion/NotCrafteableVitaPotion.png");

	OblitiusPotionTex = app->tex->Load("Assets/Textures/Potions/CreatePotion/CrafteableOblitiusPotion.png");
	NotOblitiusPotionTex = app->tex->Load("Assets/Textures/Potions/CreatePotion/NotCrafteableOblitiusPotion.png");

	currentPotion = CeleritaPotionTex;

	return true;
}

// Called each loop iteration
bool TavernScene::PreUpdate()
{
	// OPTICK PROFILIN
	ZoneScoped;

	return true;
}

// Called each loop iteration
bool TavernScene::Update(float dt)
{
	// OPTICK PROFILIN
	ZoneScoped;
	
	if(freeCam)
	{
		float camSpeed = 1;

		if(app->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
			app->render->camera.y -= (int)ceil(camSpeed * dt);

		if(app->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
			app->render->camera.y += (int)ceil(camSpeed * dt);

		if(app->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
			app->render->camera.x -= (int)ceil(camSpeed * dt);

		if(app->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
			app->render->camera.x += (int)ceil(camSpeed * dt);
	}

	if (cauldronIsOpened && cauldron == nullptr) {
		cauldron = (GuiControlPopUp*)app->guiManager->CreateGuiControl(GuiControlType::POPUP, 13, "test", { (int)windowW / 2 - 800, (int)windowH / 2 - 450 }, this, cauldronTex);
		cauldronExit = (GuiControlButton*)app->guiManager->CreateGuiControl(GuiControlType::BUTTON, 14, "Exit", { (int)windowW / 2 + 550, (int)windowH / 2 + 350, 200, 50 }, this);
		cauldronCreate = (GuiControlButton*)app->guiManager->CreateGuiControl(GuiControlType::BUTTON, 11, "Create", { (int)windowW / 2 + 320, (int)windowH / 2 + 350, 200, 50 }, this);
	}

	if (app->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN && cauldronSelect != nullptr) {
		if (type >= 1 && type < 4) {
			type++;
		}
	}
	if (app->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN && cauldronSelect != nullptr) {
		if (type > 1 && type <= 4) {
			type--;
		}
	}

	if (cauldron != nullptr) {
		if (cauldronExitPressed) {
			app->guiManager->RemoveGuiControl(cauldronCreate);
			app->guiManager->RemoveGuiControl(cauldronExit);
			app->guiManager->RemoveGuiControl(cauldron);
			cauldronExitPressed = false;
			cauldronIsOpened = false;
			cauldron = nullptr;
		}

		if (cauldronCreatePressed && cauldronSelect == nullptr) {
			cauldronSelect = (GuiControlPopUp*)app->guiManager->CreateGuiControl(GuiControlType::POPUP, 13, "test", { (int)windowW / 2 - 800, (int)windowH / 2 - 450 }, this, cauldronSelectTex);
			cauldronSelectExit = (GuiControlButton*)app->guiManager->CreateGuiControl(GuiControlType::BUTTON, 12, "Exit", { (int)windowW / 2 + 550, (int)windowH / 2 + 350, 200, 50 }, this);
			potionCreateButton = (GuiControlButton*)app->guiManager->CreateGuiControl(GuiControlType::BUTTON, 15, "Accept", { (int)windowW / 2 - 100, (int)windowH / 2 + 300, 200, 50 }, this);

			cauldronCreatePressed = false;
		}

		if (potionCreatePressed && cauldronSelect != nullptr) {

			potionCreatePressed = false;
		}
		Inventory* playerInventory = &app->entityManager->GetPlayerEntity()->inventory; 
		Item* potion; 
		if (cauldronSelect != nullptr) {
			switch (type) {
			case 1:	 
				for (int i = 0; i < playerInventory->items.size(); i++)
				{
					potion = playerInventory->items[i];
					if (potion->quantity >= 3)
					{
						if (CrafteableCeleritaPotion == nullptr)
							CrafteableCeleritaPotion = (GuiControlPopUp*)app->guiManager->CreateGuiControl(GuiControlType::POPUP, 13, "test", { (int)windowW / 2 - 800, (int)windowH / 2 - 450 }, this, CeleritaPotionTex);
					}
					else if (potion->quantity < 3) {
						if (NotCrafteableCeleritaPotion == nullptr)
							NotCrafteableCeleritaPotion = (GuiControlPopUp*)app->guiManager->CreateGuiControl(GuiControlType::POPUP, 13, "test", { (int)windowW / 2 - 800, (int)windowH / 2 - 450 }, this, NotCeleritaPotionTex);
					}
				}

				app->guiManager->RemoveGuiControl(CrafteableEtherPotion);
				CrafteableEtherPotion = nullptr;
				app->guiManager->RemoveGuiControl(CrafteableVitaPotion);
				CrafteableVitaPotion = nullptr;
				app->guiManager->RemoveGuiControl(CrafteableOblitiusPotion);
				CrafteableOblitiusPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableEtherPotion);
				NotCrafteableEtherPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableVitaPotion);
				NotCrafteableVitaPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableOblitiusPotion);
				NotCrafteableOblitiusPotion = nullptr;
				break;
			case 2:
				for (int i = 0; i < playerInventory->items.size(); i++)
				{
					potion = playerInventory->items[i];
					if (potion->quantity >= 3)
					{
						if (CrafteableEtherPotion == nullptr)
							CrafteableEtherPotion = (GuiControlPopUp*)app->guiManager->CreateGuiControl(GuiControlType::POPUP, 13, "test", { (int)windowW / 2 - 800, (int)windowH / 2 - 450 }, this, EtherPotionTex);
					}
					else if (potion->quantity < 3) {
						if (NotCrafteableEtherPotion == nullptr)
							NotCrafteableEtherPotion = (GuiControlPopUp*)app->guiManager->CreateGuiControl(GuiControlType::POPUP, 13, "test", { (int)windowW / 2 - 800, (int)windowH / 2 - 450 }, this, NotEtherPotionTex);
					}
				}

				app->guiManager->RemoveGuiControl(CrafteableCeleritaPotion);
				CrafteableCeleritaPotion = nullptr;
				app->guiManager->RemoveGuiControl(CrafteableVitaPotion);
				CrafteableVitaPotion = nullptr;
				app->guiManager->RemoveGuiControl(CrafteableOblitiusPotion);
				CrafteableOblitiusPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableCeleritaPotion);
				NotCrafteableCeleritaPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableVitaPotion);
				NotCrafteableVitaPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableOblitiusPotion);
				NotCrafteableOblitiusPotion = nullptr;
				break;
			case 3:
				for (int i = 0; i < playerInventory->items.size(); i++)
				{
					potion = playerInventory->items[i];
					if (potion->quantity >= 3)
					{
						if (CrafteableVitaPotion == nullptr)
							CrafteableVitaPotion = (GuiControlPopUp*)app->guiManager->CreateGuiControl(GuiControlType::POPUP, 13, "test", { (int)windowW / 2 - 800, (int)windowH / 2 - 450 }, this, VitaPotionTex);
					}
					else if (potion->quantity < 3) {
						if (NotCrafteableVitaPotion == nullptr)
							NotCrafteableVitaPotion = (GuiControlPopUp*)app->guiManager->CreateGuiControl(GuiControlType::POPUP, 13, "test", { (int)windowW / 2 - 800, (int)windowH / 2 - 450 }, this, NotVitaPotionTex);
					}
				}

				app->guiManager->RemoveGuiControl(CrafteableEtherPotion);
				CrafteableEtherPotion = nullptr;
				app->guiManager->RemoveGuiControl(CrafteableCeleritaPotion);
				CrafteableCeleritaPotion = nullptr;
				app->guiManager->RemoveGuiControl(CrafteableOblitiusPotion);
				CrafteableOblitiusPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableEtherPotion);
				NotCrafteableEtherPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableCeleritaPotion);
				NotCrafteableCeleritaPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableOblitiusPotion);
				NotCrafteableOblitiusPotion = nullptr;

				break;
			case 4:
				for (int i = 0; i < playerInventory->items.size(); i++)
				{
					potion = playerInventory->items[i];
					if (potion->quantity >= 3)
					{
						if (CrafteableOblitiusPotion == nullptr)
							CrafteableOblitiusPotion = (GuiControlPopUp*)app->guiManager->CreateGuiControl(GuiControlType::POPUP, 13, "test", { (int)windowW / 2 - 800, (int)windowH / 2 - 450 }, this, OblitiusPotionTex);
					}
					else if (potion->quantity < 3) {
						if (NotCrafteableOblitiusPotion == nullptr)
							NotCrafteableOblitiusPotion = (GuiControlPopUp*)app->guiManager->CreateGuiControl(GuiControlType::POPUP, 13, "test", { (int)windowW / 2 - 800, (int)windowH / 2 - 450 }, this, NotOblitiusPotionTex);
					}
				}

				app->guiManager->RemoveGuiControl(CrafteableEtherPotion);
				CrafteableEtherPotion = nullptr;
				app->guiManager->RemoveGuiControl(CrafteableVitaPotion);
				CrafteableVitaPotion = nullptr;
				app->guiManager->RemoveGuiControl(CrafteableCeleritaPotion);
				CrafteableCeleritaPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableEtherPotion);
				NotCrafteableEtherPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableVitaPotion);
				NotCrafteableVitaPotion = nullptr;
				app->guiManager->RemoveGuiControl(NotCrafteableCeleritaPotion);
				NotCrafteableCeleritaPotion = nullptr;
				break;
			}
		}

		if (selectExitPressed && cauldronSelect != nullptr) {
			app->guiManager->RemoveGuiControl(cauldronSelectExit);
			app->guiManager->RemoveGuiControl(cauldronSelect);
			app->guiManager->RemoveGuiControl(potionCreateButton);
			app->tex->UnLoad(currentPotion);

			app->guiManager->RemoveGuiControl(CrafteableEtherPotion);
			CrafteableEtherPotion = nullptr;
			app->guiManager->RemoveGuiControl(CrafteableVitaPotion);
			CrafteableVitaPotion = nullptr;
			app->guiManager->RemoveGuiControl(CrafteableOblitiusPotion);
			CrafteableOblitiusPotion = nullptr;
			app->guiManager->RemoveGuiControl(CrafteableCeleritaPotion);
			CrafteableCeleritaPotion = nullptr;
			app->guiManager->RemoveGuiControl(NotCrafteableEtherPotion);
			NotCrafteableEtherPotion = nullptr;
			app->guiManager->RemoveGuiControl(NotCrafteableVitaPotion);
			NotCrafteableVitaPotion = nullptr;
			app->guiManager->RemoveGuiControl(NotCrafteableOblitiusPotion);
			NotCrafteableOblitiusPotion = nullptr;
			app->guiManager->RemoveGuiControl(NotCrafteableCeleritaPotion);
			NotCrafteableCeleritaPotion = nullptr;
				
			cauldronIsOpened = false;
			cauldronSelect = nullptr; 
			selectExitPressed = false; 
			
		}

		
	}

	//Cambios de escena sin collider
	if (app->entityManager->GetPlayerEntity()->position.x <= 1390 && app->entityManager->GetPlayerEntity()->position.x >= 1150 && app->entityManager->GetPlayerEntity()->position.y <= 3835 && app->entityManager->GetPlayerEntity()->position.y >= 3670) {
		app->sceneManager->ChangeScene("townscene");
	}

	return true;
}

// Called each loop iteration
bool TavernScene::PostUpdate()
{
	// OPTICK PROFILIN
	ZoneScoped;

	bool ret = true;

	if(app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
	{
		if(paused)
		{
			paused = false;
			gcResume->state = GuiControlState::NORMAL;
			gcSettings->state = GuiControlState::NORMAL;
			gcBackToTitle->state = GuiControlState::NORMAL;
			gcExit->state = GuiControlState::NORMAL;
			gcSave->state = GuiControlState::NORMAL;
		}
		else
		{
			paused = true;
			gcResume->state = GuiControlState::DISABLED;
			gcSettings->state = GuiControlState::DISABLED;
			gcBackToTitle->state = GuiControlState::DISABLED;
			gcExit->state = GuiControlState::DISABLED;
			gcSave->state = GuiControlState::DISABLED;
		}
	}
	cauldronOpen->state = GuiControlState::NORMAL;  
	if(exitPressed)
		ret = false;

	return ret;
}

bool TavernScene::Exit()
{
	//IMPORTANTE: DESCARGAR EN ORDEN INVERSO AL CARGADO EN EL APP
	app->entityManager->Disable();
	app->map->Disable();

	app->guiManager->RemoveGuiControl(gcScore);
	app->guiManager->RemoveGuiControl(gcLives);
	app->guiManager->RemoveGuiControl(gcResume);
	app->guiManager->RemoveGuiControl(gcSettings);
	app->guiManager->RemoveGuiControl(gcBackToTitle);
	app->guiManager->RemoveGuiControl(gcExit);
	app->guiManager->RemoveGuiControl(gcSave);

	SDL_ShowCursor(SDL_ENABLE);

	return true;
}

// Called before quitting
bool TavernScene::CleanUp()
{
	LOG("Freeing testscene");

	app->guiManager->RemoveGuiControl(gcScore);
	app->guiManager->RemoveGuiControl(gcLives);
	app->guiManager->RemoveGuiControl(gcResume);
	app->guiManager->RemoveGuiControl(gcSettings);
	app->guiManager->RemoveGuiControl(gcBackToTitle);
	app->guiManager->RemoveGuiControl(gcExit);
	app->guiManager->RemoveGuiControl(gcSave);

	return true;
}

bool TavernScene::OnGuiMouseClickEvent(GuiControl* control)
{
	// L15: DONE 5: Implement the OnGuiMouseClickEvent method
	LOG("Press Gui Control: %d", control->id);

	switch (control->id)
	{
	case 6:
		paused = true;
		gcResume->state = GuiControlState::DISABLED;
		gcSettings->state = GuiControlState::DISABLED;
		gcBackToTitle->state = GuiControlState::DISABLED;
		gcExit->state = GuiControlState::DISABLED;
		gcSave->state = GuiControlState::DISABLED;
		break;
	case 7:
		break;
	case 8:
		app->sceneManager->ChangeScene("mainmenu");
		break;
	case 9:
		exitPressed = true;
	break;
	case 16:
		cauldronIsOpened = true;
		break;
	case 14:
		if(cauldronSelect == nullptr) cauldronExitPressed = true;
		break;
	case 11:
		cauldronCreatePressed = true;
		break;
	case 12:
		selectExitPressed = true;
		break;
	case 15:
		potionCreatePressed = true;
		break;
	case 10:
		app->SaveRequest();
		break;

	}

	return true;
}
