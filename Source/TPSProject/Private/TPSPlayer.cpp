// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Spring Arm을 생성해서 Root 에 붙인다.
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	// Spring Arm -> Target Arm Length : 400
	SpringArmComp->TargetArmLength = 400;
	// 위치 (X=0.000000,Y=0.000000,Z=70.000000)
	SpringArmComp->SetRelativeLocation(FVector(0, 0, 70));

	

}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

