#ifndef __INPUT_H__
#define __INPUT_H__

#include "Core/Module.h"
#include <SDL_scancode.h>
#include <SDL_haptic.h>

#define MAX_KEYS 352
#define NUM_MOUSE_BUTTONS 5
//#define LAST_KEYS_PRESSED_BUFFER 50
#define MAX_PADS 4

struct SDL_Rect;
struct _SDL_GameController;

enum EventWindow
{
	WE_QUIT = 0,
	WE_HIDE = 1,
	WE_SHOW = 2,
	WE_COUNT
};

enum Key_State
{
	KEY_IDLE ,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

struct GamePad
{
	//Input data
	bool start, back, guide;
	bool x, y, a, b, l1, r1, l3, r3;
	bool up, down, left, right;
	float l2, r2;
	float l_x, l_y, r_x, r_y, l_dz, r_dz;
	//Controller data
	bool enabled;
	int index;
	_SDL_GameController* controller;
	_SDL_Haptic* haptic;
	//Rumble controller
	int rumble_countdown;
	float rumble_strength;
};

class Input : public Module
{

public:

	Input();

	Input(bool startEnabled);

	// Destructor
	virtual ~Input();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();

	// Called before quitting
	bool CleanUp();

	// Check key states (includes mouse and joy buttons)
	Key_State GetKey(int id) const
	{
		return keyboard[id];
	}

	Key_State GetMouseButtonDown(int id) const
	{
		return mouseButtons[id - 1];
	}

	int GetMouseX() const
	{
		return mouseX;
	}

	int GetMouseY() const
	{
		return mouseY;
	}

	// Check if a certain window event happened
	bool GetWindowEvent(EventWindow ev);

	// Get mouse / axis position
	void GetMousePosition(int &x, int &y);
	void GetMouseMotion(int& x, int& y);

	// Activates SDL device funcionallity when a gamepad has been connected
	void HandleDeviceConnection(int index);

	// Deactivates SDL device funcionallity when a gamepad has been disconnected
	void HandleDeviceRemoval(int index);

	// Called at PreUpdate
	// Iterates through all active gamepads and update all input data
	void UpdateGamepadsInput();

	bool ShakeController(int id, int duration, float strength = 0.5f);
	const char* GetControllerName(int id) const;

	GamePad pads[MAX_PADS];
	Key_State keys[MAX_KEYS] = { KEY_IDLE };


private:
	bool windowEvents[WE_COUNT];
	Key_State*	keyboard;
	Key_State mouseButtons[NUM_MOUSE_BUTTONS];
	int	mouseMotionX;
	int mouseMotionY;
	int mouseX;
	int mouseY;
};

#endif // __INPUT_H__