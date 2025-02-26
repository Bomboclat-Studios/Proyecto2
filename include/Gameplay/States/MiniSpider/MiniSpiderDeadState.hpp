#ifndef __MINISPIDERDEADSTATE_H__
#define __MINISPIDERDEADSTATE_H__

#include "Gameplay/Entities/Enemies/MiniSpider.h"
#include "Utils/State.h"
#include "Utils/SString.h"
#include "Utils/Defs.h"

class MiniSpiderDeadState : public State<MiniSpider> {
private:
    MiniSpider* minispider;

public:
    MiniSpiderDeadState(SString name) : State(name) {}
    inline void Enter() override
    {

        minispider = StateMachineReference->owner;
    }
    inline void Update(float dt) override
    {

        //Animation
        app->render->DrawTexture(minispider->spiderDeath.texture, minispider->position.x - 60, minispider->position.y - 60, &minispider->spiderDeath.GetCurrentFrame(), 1.0f, minispider->pbody->body->GetAngle() * RADTODEG, 0.5f, 1.0f, minispider->flip);
        minispider->spiderDeath.Update(dt);

        if (minispider->spiderDeath.GetCurrentFrameCount() >= 12) {
            minispider->Disable();
            minispider->CleanUp();
        }
    }
    inline void Exit() override
    {
    }
};
#endif // __MINISPIDERDEADSTATE_H__