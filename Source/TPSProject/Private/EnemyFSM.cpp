// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM.h"
#include "TPSPlayer.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UEnemyFSM::UEnemyFSM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	// ���忡�� ATPSPlayer Ÿ�� ã�ƿ���
	auto Actor = UGameplayStatics::GetActorOfClass(GetWorld(), ATPSPlayer::StaticClass());
	//ATPSPlayer Ÿ������ ĳ����
	Target = Cast<ATPSPlayer>(Actor);
	// �ҿ� ��ü ��������
	Me = Cast<AEnemy>(GetOwner());
	
}


// Called every frame
void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ���� â�� ���� �޼��� ���
	FString LogMsg = UEnum::GetValueAsString(mState);
	GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, LogMsg);

	switch (mState)
	{
	case EEnemyState::Idle:
		IdleState();
		break;
	case EEnemyState::Move:
		MoveState();
		break;
	case EEnemyState::Attack:
		AttackState();
		break;
	case EEnemyState::Damage:
		DamageState();
		break;
	case EEnemyState::Die:
		DieState();
		break;
	}
	
}

void UEnemyFSM::IdleState()
{
	// ���� �ð� ��ٷȴٰ� �̵� ���·� ��ȯ�ϰ� �ʹ�.
	//1. �ð��� ������ϱ�
	CurrentTime += GetWorld()->DeltaTimeSeconds;
	// 2. ���� ��� �ð��� ��� �ð��� �ʰ��ߴٸ�
	if (CurrentTime > IdleDelayTime)
	{
		//3. �̵� ���·� ��ȯ�ϰ� �ʹ�.
		mState = EEnemyState::Move;
		// ��� �ð� �ʱ�ȭ
		CurrentTime = 0;
	}
}

void UEnemyFSM::MoveState()
{
	// Ÿ���� �������� �̵��ϰ� �ʹ�.
	// 1. Ÿ���� �������� �ʿ�
	FVector Destination = Target->GetActorLocation();
	// 2. ������ �ʿ�
	FVector Dir = Destination - Me->GetActorLocation();
	// 3. �������� �̵��ϰ� �ʹ�.
	Me->AddMovementInput(Dir.GetSafeNormal());

	// Ÿ��� ��������� ���� ���·� ��ȯ�ϰ� �ʹ�.
	// 1. ���� �Ÿ��� ���� ���� �ȿ� ������
	if (Dir.Size() < AttackRange)
	{
		// 2. ���� ���·� ��ȯ�ϰ� �ʹ�.
		mState = EEnemyState::Attack;
	}
}

void UEnemyFSM::AttackState()
{
	// ���� �ð��� �� ���� �����ϰ� �ʹ�.
	// 1. �ð��� �귯�� �Ѵ�.
	CurrentTime += GetWorld()->DeltaTimeSeconds;
	// 2. ���� �ð��� �����ϱ�
	if (CurrentTime > AttackDelayTime)
	{
		// 3. �����ϰ� �ʹ�.
		UE_LOG(LogTemp, Warning, TEXT("Attack"));

	}
}

void UEnemyFSM::DamageState()
{
}

void UEnemyFSM::DieState()
{
}



