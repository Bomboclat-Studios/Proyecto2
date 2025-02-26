#ifndef __MINISPIDERATTACKSTATE_H__
#define __MINISPIDERATTACKSTATE_H__

#include "Gameplay/Entities/Enemies/MiniSpider.h"
#include "Utils/State.h"
#include "Utils/SString.h"
#include "Utils/Defs.h"
#include "Gameplay/Entities/Player.h"

class Player;
class MiniSpiderAttackState : public State<MiniSpider> {
private:
    MiniSpider* minispider;
    Player* player;

public:
    MiniSpiderAttackState(SString name) : State(name) {}
    inline void Enter() override
    {

        minispider = StateMachineReference->owner;
    }
    inline void Update(float dt) override
    {

        player = app->entityManager->GetPlayerEntity();

        //Animation
        app->render->DrawTexture(minispider->spiderAttack.texture, minispider->position.x - 60, minispider->position.y - 60, &minispider->spiderAttack.GetCurrentFrame(), 1.0f, minispider->pbody->body->GetAngle() * RADTODEG, 0.5f, 1.0f, minispider->flip);
        minispider->spiderAttack.Update(dt);

        //Attack
        if (minispider->spiderAttack.GetCurrentFrameCount() == 5 && minispider->isTouchingPlayer) {
            player->vida -= minispider->dano;
        }

        //Sound
        if (minispider->spiderAttack.GetCurrentFrameCount() == 4) {
            app->audio->PlayFx(minispider->attackFx);
        }

        if (minispider->spiderAttack.GetCurrentFrameCount()>=9) {
            minispider->attackTimer.Start();
            StateMachineReference->ChangeState("idle");
        }

    }
    inline void Exit() override
    {
    }
};
#endif // __MINISPIDERATTACKSTATE_H__