#include "Core/App.h"
#include "Gameplay/Entities/Bullet.h"
#include "Core/Physics.h"
#include "Core/Render.h"
#include "Core/Textures.h"
#include "Utils/Log.h"

#include <imgui.h>

Bullet::Bullet() {
	texture = app->tex->Load("Assets/Textures/Bullet.png");
	pbody = app->physics->CreateCircle(0, 0, 10, bodyType::DYNAMIC);
	pbody->ctype = ColliderType::BULLET;
	timer = new Timer();
}

void Bullet::Shoot(b2Vec2 force, SDL_RendererFlip flip) {
	active = true;
	timer->Start();

	//resets the inertia of the body
	pbody->body->SetAwake(false);
	pbody->body->SetAwake(true);
	pbody->body->GetFixtureList()->SetSensor(true);
	if (flip == SDL_FLIP_NONE) {
		pbody->body->SetTransform(b2Vec2(PIXEL_TO_METERS(position.x+5) , PIXEL_TO_METERS(position.y+2) ), 0);
	}
	else {
		pbody->body->SetTransform(b2Vec2(PIXEL_TO_METERS(position.x), PIXEL_TO_METERS(position.y+2)), 0);
	}

	pbody->body->ApplyForceToCenter(b2Vec2(force.x * 100, force.y * 100), true);
}

void Bullet::Draw(float angleToPlayer) {
	if (active) {
		pbody->GetPosition(position.x, position.y);
		app->render->DrawTexture(texture, position.x-40, position.y+10, 0, 1.0f, angleToPlayer,0.06f);
	}
}

void Bullet::Update(float angleToPlayer) {
	Draw(angleToPlayer);
	if (timer->ReadSec() > 1) {
		active = false;
	}
}

void Bullet::CleanUp() {
	app->physics->DestroyBody(pbody);
	app->tex->UnLoad(texture);
}