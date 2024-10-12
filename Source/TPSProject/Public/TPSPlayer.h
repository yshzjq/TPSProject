// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSPlayer.generated.h"

UCLASS()
class TPSPROJECT_API ATPSPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// ������Ʈ - Spring Arm, Cemera
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArmComp;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class UCameraComponent* CameraComp;

	//������Ʈ - Weapon (����) ����20240819����
	UPROPERTY(EditAnywhere,Category = weapon)
	class USkeletalMeshComponent* HandGun;  

	UPROPERTY(EditAnywhere,Category = waepon)
	class USkeletalMeshComponent* SniperGun;

public:

	// �Է� ó�� �Լ� ����
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* IMC_TPS;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_LookUp;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Turn;

public:
	UPROPERTY(EditAnywhere,Category = BulletFactory)
	TSubclassOf<class ABullet> BulletFactory;

	// �¿� ȸ��
	void Turn(const struct FInputActionValue& inputValue);
	// ���� ȸ��
	void LookUp(const struct FInputActionValue& inputValue);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_PlayerMove;
	// �̵� �ӵ�
	UPROPERTY(EditDefaultsOnly, Category = "PlayerSetting")
	float walkSpeed = 600;
	// �̵� ����
	FVector Direction;

	void PlayerMove(const struct FInputActionValue& inputValue);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Jump;
	// ���� �Է� ó��

	void InputJump(const struct FInputActionValue& inputValue);

	void Move();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Fire;
	//�Ѿ� �߻� ó��
	void InputFire(const struct FInputActionValue& inputValue);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_HandGun;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_SniperGun;

	// ����(Hand Gun)�� ��� ������ Ȯ��
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TPS)
	bool bUsingHandGun = true;
	// ���� (Sniper Gun)�� ��� ������ Ȯ��
	bool bUsingSniperGun = true;
	// ����(Hand GUn) ���� ����
	void ChangeToHandGun(const struct FInputActionValue& inputValue);
	// thchd(Sniper Gun) ���� ����
	void ChangeToSniperGun(const struct FInputActionValue& inputValue);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Sniper;
	// �������� ���� ó��
	void SniperAim(const struct FInputActionValue& inputValue);
	//�������� ���� ������ ����
	bool bSniperAim = false;

	// ũ�ν����, �������� ���� UI ������ ������ �ִ�.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UUserWidget> CrosshairUIfactory;

	UPROPERTY(EditDefaultsOnly)
	class UUserWidget* CrosshairUI;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UUserWidget> SniperUIfactory;

	UPROPERTY(EditDefaultsOnly)
	class UUserWidget* SniperUI;

	// �Ѿ� ���� ȿ�� ����
	UPROPERTY(EditDefaultsOnly, Category = BulletEffect)
	class UParticleSystem* BulletEffectFactory;

	
};
