#ifndef __FLYINGENEMYDEADSTATE_H__
#define __FLYINGENEMYDEADSTATE_H__

#include "Gameplay/Entities/Enemies/FlyingEnemy.h"
#include "Utils/State.h"
#include "Utils/SString.h"
#include "Utils/Defs.h"

class FlyingEnemyDeadState : public State<FlyingEnemy> {
private:
    FlyingEnemy* flyingenemy;

public:
    FlyingEnemyDeadState(SString name) : State(name) {}
    inline void Enter() override
    {


        flyingenemy = StateMachineReference->owner;
    }
    inline void Update(float dt) override
    {


        //if (flyingenemy->reviveTimer.ReadSec() >= 5)
        //{
        //    StateMachineReference->ChangeState("idle");
        //    //enemyboss->health = 5;
        //}
    }
    inline void Exit() override
    {

    }
};
#endif // __FLYINGENEMYDEADSTATE_H__