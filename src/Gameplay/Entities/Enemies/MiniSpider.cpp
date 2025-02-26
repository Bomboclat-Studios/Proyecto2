#include "Gameplay/Entities/Enemies/MiniSpider.h"
#include "Core/App.h"
#include "Gameplay/Entities/Entity.h"
#include "Gameplay/Entities/Player.h"
#include "Core/Textures.h"
#include "Core/Input.h"
#include "Core/Render.h"
#include "Gameplay/Scene.h"
#include "Utils/Point.h"
#include "Core/Physics.h"
#include "Core/Audio.h"
#include "Utils/StateMachine.h"
#include "Core/SceneManager.h"
#include "Core/Map.h"
#include "Core/AnimationManager.h"
#include "Core/ParticleManager.h"


#include "Gameplay/States/MiniSpider/MiniSpiderIdleState.hpp"
#include "Gameplay/States/MiniSpider/MiniSpiderAttackState.hpp"
#include "Gameplay/States/MiniSpider/MiniSpiderMoveState.hpp"
#include "Gameplay/States/MiniSpider/MiniSpiderHurtState.hpp"
#include "Gameplay/States/MiniSpider/MiniSpiderDeadState.hpp"
#include "Gameplay/States/MiniSpider/MiniSpiderParalyzedState.hpp"

#include <cmath>
#include <iostream>

#include <box2d/b2_math.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>

#include <imgui.h>

MiniSpider::MiniSpider() : Entity(EntityType::MINISPIDER)
{
	name.Create("MiniSpider");
}

MiniSpider::~MiniSpider() {

}

bool MiniSpider::Awake() {

	return true;
}

bool MiniSpider::Start() {

	pathfinding = new PathFinding();

	pathfinding->SetNavigationMap((uint)app->map->mapData.width, (uint)app->map->mapData.height, app->map->navigationMap);

	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();
	texturePath = parameters.attribute("texturepath").as_string();
	newPosition = spawnPosition = position;

	timer = Timer();

	pbody = app->physics->CreateRectangle(position.x, position.y, 90, 90, bodyType::DYNAMIC);
	pbody->listener = this;
	pbody->ctype = ColliderType::ENEMYSPIDER;

	//si quieres dar vueltos como la helice de un helicoptero Boeing AH-64 Apache pon en false la siguiente funcion
	pbody->body->SetFixedRotation(true);
	//pbody->body->GetFixtureList()->SetFriction(25.0f);
	pbody->body->SetLinearDamping(10.0f);

	player = app->entityManager->GetPlayerEntity();

	movementFSM = new StateMachine<MiniSpider>(this);
	movementFSM->AddState(new MiniSpiderIdleState("idle"));
	movementFSM->AddState(new MiniSpiderMoveState("move"));
	movementFSM->AddState(new MiniSpiderHurtState("hurt"));
	movementFSM->AddState(new MiniSpiderAttackState("attack"));
	movementFSM->AddState(new MiniSpiderDeadState("die"));
	movementFSM->AddState(new MiniSpiderParalyzedState("paralyzed"));

	//Animations
	spiderIdle = *app->animationManager->GetAnimByName("Mini_Spider_Idle");
	spiderIdle.speed = 2.0f;

	spiderAttack = *app->animationManager->GetAnimByName("Mini_Spider_Ataque");
	spiderAttack.speed = 2.0f;

	spiderMove = *app->animationManager->GetAnimByName("Mini_Spider_Caminar");
	spiderMove.speed = 2.0f;

	spiderDamage = *app->animationManager->GetAnimByName("Mini_Spider_Damage");
	spiderDamage.speed = 2.0f;

	spiderDeath = *app->animationManager->GetAnimByName("Mini_Spider_Muerte");
	spiderDeath.speed = 1.0f;

	damageFx = app->audio->LoadFx(parameters.attribute("damageFxPath").as_string());
	attackFx = app->audio->LoadFx(parameters.attribute("attackFxPath").as_string());
	dieFx = app->audio->LoadFx(parameters.attribute("dieFxPath").as_string());

	damage = new ParticleGenerator();
	damage->emiting = false;
	damage->oneShoot = true;
	damage->lifetime = 0.25f;
	damage->explosiveness = 1.0f;
	damage->spawnRadius = 50;
	damage->size = 30;
	damage->initialVelocity = 0;
	damage->Damping = 0.0f;
	damage->spread = 180;
	damage->sizeFade = -1.0f;
	damage->opacityFade = 0.5f;
	damage->color = { 50, 128, 128, 255 };
	app->particleManager->AddGenerator(damage);

	paralyzedParticles = new ParticleGenerator();
	paralyzedParticles->emiting = false;
	paralyzedParticles->oneShoot = true;
	paralyzedParticles->lifetime = 0.25f;
	paralyzedParticles->explosiveness = 1.0f;
	paralyzedParticles->spawnRadius = 50;
	paralyzedParticles->size = 30;
	paralyzedParticles->initialVelocity = 0;
	paralyzedParticles->Damping = 0.0f;
	paralyzedParticles->spread = 180;
	paralyzedParticles->sizeFade = -1.0f;
	paralyzedParticles->opacityFade = 0.5f;
	paralyzedParticles->color = { 50, 128, 128, 255 };
	app->particleManager->AddGenerator(paralyzedParticles);

	return true;
}

bool MiniSpider::Update(float dt)
{

	movementFSM->Update(dt);
	pbody->body->SetTransform(pbody->body->GetPosition(), 0);

	app->render->DrawLine(METERS_TO_PIXELS(pbody->body->GetPosition().x), METERS_TO_PIXELS(pbody->body->GetPosition().y), METERS_TO_PIXELS(pbody->body->GetPosition().x) + pbody->body->GetLinearVelocity().x * 10, METERS_TO_PIXELS(pbody->body->GetPosition().y) + +pbody->body->GetLinearVelocity().y * 10, 255, 255, 0);

	//Update player position in pixels
	position.x = METERS_TO_PIXELS(pbody->body->GetTransform().p.x) - 16;
	position.y = METERS_TO_PIXELS(pbody->body->GetTransform().p.y) - 16;

	damage->position = { position.x + 16, position.y + 16 };

	if (debug) {
		if (app->input->GetKey(SDL_SCANCODE_F4) == KEY_DOWN) {
			freeCam = !freeCam;
		}
	}

	//app->render->DrawRectangle({ position.x - 1, position.y - 2, 36, 36 }, 0, 50, 255);

	app->render->DrawCircle(position.x, position.y + 5, 1, 0, 0, 0, 0);
	/* 	app->render->DrawTexture(currentAnimation->texture, position.x - 9, position.y - 9, &currentAnimation->GetCurrentFrame(), 1.0f, pbody->body->GetAngle()*RADTODEG, flip);

		currentAnimation->Update(dt); */

		// Calculate the angle between the enemy and the player
	float angleToPlayer = atan2(player->position.y - position.y, player->position.x - position.x);

	// Determine if the player is to the left or right of the enemy
	if (player->position.x < position.x) {
		flip = SDL_FLIP_HORIZONTAL;
	}
	else {
		flip = SDL_FLIP_NONE;
	}

	return true;
}

void MiniSpider::DrawImGui()
{
	ImGui::Begin("MiniSpider");
	ImGui::Text("MiniSpider Position: %d, %d", position.x, position.y);
	ImGui::Text("MiniSpider Speed: %f", pbody->body->GetLinearVelocity().Length());
	ImGui::SliderFloat("max speed", &maxSpeed, 1.0f, 10.0f);
	ImGui::SliderFloat("move force", &moveForce, 1.0f, 10.0f);
	ImGui::End();
}

void MiniSpider::Idle(float dt) {

}

void MiniSpider::Move(float dt) {
	// TODO move logic
}

void MiniSpider::Attack(float dt)
{

}

bool MiniSpider::SaveState(pugi::xml_node& node) {

	pugi::xml_node minispiderAttributes = node.append_child("enemies").append_child("MiniSpider");
	minispiderAttributes.append_attribute("x").set_value(this->position.x);
	minispiderAttributes.append_attribute("y").set_value(this->position.y);
	minispiderAttributes.append_attribute("lives").set_value(this->vida);

	return true;
}

bool MiniSpider::LoadState(pugi::xml_node& node)
{
	pbody->body->SetTransform({ PIXEL_TO_METERS(node.child("enemies").child("MiniSpider").attribute("x").as_int()), PIXEL_TO_METERS(node.child("enemies").child("MiniSpider").attribute("y").as_int()) }, node.child("enemies").child("MiniSpider").attribute("angle").as_int());
	// reset player physics
	pbody->body->SetAwake(false);
	pbody->body->SetAwake(true);

	this->vida = node.child("enemies").child("MiniSpider").attribute("lives").as_float(); 

	return true;
}

void MiniSpider::StopMoving()
{
	if (PIXEL_TO_METERS(player->position.DistanceTo(position)) > 5.0f) { updateSpeed == noSpeed; }
	else { updateSpeed == moveSpeed; }

	pbody->body->SetTransform({ PIXEL_TO_METERS(position.x), PIXEL_TO_METERS(position.y) }, 0);

	pbody->body->SetAwake(false);
	pbody->body->SetAwake(true);
}

void MiniSpider::moveToSpawnPoint()
{
	position = spawnPosition;

	pbody->body->SetTransform({ PIXEL_TO_METERS(position.x), PIXEL_TO_METERS(position.y) }, 0);

	pbody->body->SetAwake(false);
	pbody->body->SetAwake(true);
}

bool MiniSpider::CleanUp() {
	app->tex->UnLoad(texture);
	app->physics->DestroyBody(pbody);

	return true;
}

void MiniSpider::OnCollision(PhysBody* physA, PhysBody* physB) {

	switch (physB->ctype) {
	
		case ColliderType::SWORD:
			LOG("Collision ARMAPLAYER");
		 	//if (state != EntityState::DEAD and !invencible){
			vida -= player->strength - (defense / 2);
			damage->emiting = true;

			if (player->stealLife) {
				player->vida += player->stealLifeRatio;
				if (player->vida >= 15.0f or player->vida == 15.0f)
				{
					player->vida = 15.0f;
				}
				LOG("Player steal life %f", player->vida);
			}

			if (vida <= 0.0f)
			{
				// AUDIO DONE boss death
				app->audio->PlayFx(dieFx);
				player->bestiary->enemy2Killed = true;
				player->currentExperience += 20;
				movementFSM->ChangeState("die");
			}
			else if (vida > 0.0f) {
				app->audio->PlayFx(damageFx);
				spiderDamage.Reset();

				if (app->entityManager->GetPlayerEntity()->bleed) {
					// 15% change to bleed
					if (rand() % 100 < player->bleedChance) {
						// TODO add bleed effect
						vida -= 1.0f;
						LOG("MiniSpider Bleed! %f", vida);
					}
				}

				movementFSM->ChangeState("hurt");
			}
				//else {
				//	// AUDIO DONE boss hit
				//	app->audio->PlayFx(bossHit);
				//	movementStateMachine->ChangeState("hurt");
				//	lives--;
				//}a
			break;

	case ColliderType::PLAYER:
		isTouchingPlayer = true;
		break;

	case ColliderType::MAGIC:
		vida -= player->intelligence - (defense /2);
		damage->emiting = true;
		if (vida <= 0.0f && !dead)
		{
			dead = true;
			app->audio->PlayFx(dieFx);
			player->bestiary->enemy2Killed = true;
			player->currentExperience += 20;
			movementFSM->ChangeState("die");
		}
		else if (vida > 0.0f) {
			app->audio->PlayFx(damageFx);
			spiderDamage.Reset();
			movementFSM->ChangeState("hurt");
		}
		break;

	case ColliderType::UNKNOWN:
		LOG("Colision UNKNOWN");
		break;
	}
}

void MiniSpider::EndCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype) {
	case ColliderType::PLAYER:
		isTouchingPlayer = false;
		break;
	}
}

void MiniSpider::pathfindingMovement(float dt) {

	iPoint origin = app->map->WorldToMap(newPosition.x, newPosition.y); //a�adir el tile size / 2 hace que el owl se acerque mas 

	if (timer.ReadMSec() > 250) {
		iPoint destination = app->map->WorldToMap(player->position.x, player->position.y);  //a�adir el tile size / 2 hace que el owl se acerque mas
		pathfinding->CreatePath(origin, destination);
		timer.Start();
		currentPathPos = 0;
	}

	const DynArray<iPoint>* path = pathfinding->GetLastPath();

	if (movementDelay.ReadMSec() > 100) {
		if (currentPathPos < path->Count())
		{
			newPosition = app->map->MapToWorld(path->At(currentPathPos)->x, path->At(currentPathPos)->y);
			currentPathPos++;
			//LOG("%d", currentPathPos);
			movementDelay.Start();
		}
	}

	pbody->body->SetTransform(
		{
			std::lerp(pbody->body->GetPosition().x, PIXEL_TO_METERS(newPosition.x), dt * moveSpeed / 1000),
			std::lerp(pbody->body->GetPosition().y, PIXEL_TO_METERS(newPosition.y), dt * moveSpeed / 1000)

		},

		angle * DEGTORAD
	);

	if (debug)
	{
		//Render Path
		if (path->Count() > 0)
		{
			for (uint i = 0; i < path->Count() - 1; ++i)
			{
				iPoint pos1 = app->map->MapToWorld(path->At(i)->x, path->At(i)->y);
				iPoint pos2 = app->map->MapToWorld(path->At(i + 1)->x, path->At(i + 1)->y);
				app->render->DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, 0, 0, 255);
			}

		}
	}
}

void MiniSpider::OnRaycastHit(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) {

}
