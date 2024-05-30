#ifndef __PLAYERMOVESTATE_H__
#define __PLAYERMOVESTATE_H__

#include "Gameplay/Entities/Player.h"
#include "Utils/State.h"
#include "Utils/SString.h"
#include "Utils/Defs.h"

class PlayerMoveState : public State<Player> {
private:
    Player* player;

public:
    PlayerMoveState(SString name) : State(name) {}
    inline void Enter() override
    {


        player = StateMachineReference->owner;
    }
    inline void Update(float dt) override
    {


        PhysBody* pbody = player->pbody;

        b2Vec2 impulse = { 0, 0 };

        if (pbody->body->GetLinearVelocity().Length() < player->maxSpeed)
        {
            if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
                impulse.x = -pbody->body->GetMass() * player->moveForce;
            }
            if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
                impulse.x = pbody->body->GetMass() * player->moveForce;
            }
            if (app->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
               impulse.y = -pbody->body->GetMass() * player->moveForce;
            }
            if (app->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
                impulse.y = pbody->body->GetMass() * player->moveForce;
            }
            pbody->body->ApplyLinearImpulse(impulse, pbody->body->GetWorldCenter(), true);
        }

        if (pbody->body->GetLinearVelocity().Length() < 1.0f)
        {
            StateMachineReference->ChangeState("idle");
        }
    }
    inline void Exit() override
    {

    }
};
#endif // __PLAYERMOVESTATE_H__