// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnim.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API UPlayerAnim : public UAnimInstance
{
	GENERATED_BODY()
public:
	// 매 프레임 갱신되는 함수
	virtual void NativeUpdateAnimation(float Deltatime) override;

	UPROPERTY()
	class ATPSPlayer* Player;
	
public:
	// 이동 속도
	// 앞뒤로 움직이는 속력
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = PlayerAnim)
	float Speed = 0;
	// 좌우로 움직이는 속력
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = PlayerAnim)
	float Direction;
	UPROPERTY(EditDefaultsOnly, BLueprintReadwrite,Category = PlayerAnim)
	bool IsInAir;

	UPROPERTY(EditDefaultsOnly,Category = PlayerAnim)
	class UAnimMontage* AttackAnimMontage;
	// 공격 애니메이션 함수
	void PlayAttackAnim();
};
