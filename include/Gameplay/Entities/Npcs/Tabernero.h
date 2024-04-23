#ifndef __TABERNERO_H__
#define __TABERNERO_H__

#include "Core/Physics.h"
#include "Utils/Timer.h"
#include "Utils/StateMachine.h"
#include "Gameplay/Entities/Npcs/Npc.h"

#include <box2d/b2_fixture.h>
#include <SDL.h>
#include <box2d/b2_math.h>

struct SDL_Texture;

class Tabernero : public Npc
{
public:

	bool startTimer = true;
	Timer timer;

	Tabernero();

	virtual ~Tabernero();

	bool Awake() override;

	bool Start() override;

	bool Update(float dt) override;

	bool CleanUp() override;

public:

	const char* texturePath;
	SDL_Texture* texture = NULL;

	PhysBody* pbody;
};

#endif // __TABERNERO_H__