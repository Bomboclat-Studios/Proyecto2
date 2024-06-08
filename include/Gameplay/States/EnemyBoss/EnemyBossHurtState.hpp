#ifndef __ENEMYBOSSHURTSTATE_H__
#define __ENEMYBOSSHURTSTATE_H__

#include "Gameplay/Entities/Enemies/EnemyBoss.h"
#include "Utils/State.h"
#include "Utils/SString.h"
#include "Utils/Defs.h"

class EnemyBossHurtState : public State<EnemyBoss> {
private:
    EnemyBoss* enemyboss;

public:
    EnemyBossHurtState(SString name) : State(name) {}
    inline void Enter() override
    {
        enemyboss = StateMachineReference->owner;
    }
    inline void Update(float dt) override
    {

        //Animation
        app->render->DrawTexture(enemyboss->bossDamage.texture, enemyboss->position.x - 60, enemyboss->position.y - 150, &enemyboss->bossDamage.GetCurrentFrame(), 1.0f, enemyboss->pbody->body->GetAngle() * RADTODEG, 1.0f, 1.0f, enemyboss->flip);
        enemyboss->bossDamage.Update(dt);

        if (enemyboss->bossDamage.GetCurrentFrameCount() >= 3) {
            enemyboss->hurtTimer.Start();
            StateMachineReference->ChangeState("move");
        }
    }
    inline void Exit() override
    {

    }
};
#endif // __ENEMYBOSSHURTSTATE_H__