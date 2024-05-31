#ifndef __UI_H__
#define __UI_H__

#include "Core/Physics.h"

#include <box2d/b2_fixture.h>
#include <SDL.h>
#include <box2d/b2_math.h>

struct SDL_Texture;

class UI : public Module
{
public:

	UI();

	UI(bool startEnabled);

	virtual ~UI();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

public:

	SDL_Texture* texture_inventory = NULL;

	SDL_Texture* MenuPequeño;
	SDL_Texture* VidaBestiario;
	SDL_Texture* Espada;
	SDL_Texture* Cetro;

	uint windowW, windowH;
};

#endif // __UI_H__