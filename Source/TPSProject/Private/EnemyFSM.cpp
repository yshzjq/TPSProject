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

	// 월드에서 ATPSPlayer 타깃 찾아오기
	auto Actor = UGameplayStatics::GetActorOfClass(GetWorld(), ATPSPlayer::StaticClass());
	//ATPSPlayer 타입으로 캐스팅
	Target = Cast<ATPSPlayer>(Actor);
	// 소요 객체 가져오기
	Me = Cast<AEnemy>(GetOwner());
	
}


// Called every frame
void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 실행 창에 상태 메세지 출력
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
	// 일정 시간 기다렸다가 이동 상태로 전환하고 싶다.
	//1. 시간이 흘렸으니까
	CurrentTime += GetWorld()->DeltaTimeSeconds;
	// 2. 만약 경과 시간이 대기 시간을 초과했다면
	if (CurrentTime > IdleDelayTime)
	{
		//3. 이동 상태로 전환하고 싶다.
		mState = EEnemyState::Move;
		// 경과 시간 초기화
		CurrentTime = 0;
	}
}

void UEnemyFSM::MoveState()
{
	// 타깃의 방향으로 이동하고 싶다.
	// 1. 타깃의 목적지가 필요
	FVector Destination = Target->GetActorLocation();
	// 2. 방향이 필요
	FVector Dir = Destination - Me->GetActorLocation();
	// 3. 방향으로 이동하고 싶다.
	Me->AddMovementInput(Dir.GetSafeNormal());

	// 타깃과 가까워지면 공격 상태로 전환하고 싶다.
	// 1. 만약 거리가 공격 범위 안에 들어오면
	if (Dir.Size() < AttackRange)
	{
		// 2. 공격 상태로 전환하고 싶다.
		mState = EEnemyState::Attack;
	}
}

void UEnemyFSM::AttackState()
{
	// 일정 시간에 한 번씩 공격하고 싶다.
	// 1. 시간이 흘러야 한다.
	CurrentTime += GetWorld()->DeltaTimeSeconds;
	// 2. 공격 시간이 됐으니까
	if (CurrentTime > AttackDelayTime)
	{
		// 3. 공격하고 싶다.
		UE_LOG(LogTemp, Warning, TEXT("Attack"));

	}
}

void UEnemyFSM::DamageState()
{
}

void UEnemyFSM::DieState()
{
}



