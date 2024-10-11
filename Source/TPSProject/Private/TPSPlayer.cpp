// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubSystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Bullet.h"

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Spring Arm�� �����ؼ� Root �� ���δ�.
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	// Spring Arm -> Target Arm Length : 400
	SpringArmComp->TargetArmLength = 400;
	// ��ġ (X=0.000000,Y=0.000000,Z=70.000000)
	SpringArmComp->SetRelativeLocation(FVector(0, 0, 70));

	// Camera�� �����ؼ� SpringArm�� ���δ�.
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
	
	// Mesh�� 3D ������ �ε��ؼ� �־��ش�.
	// ������ ����̸� �̿��ؼ� ���̷�Ż �޽��� �ε��Ѵ�.
	ConstructorHelpers::FObjectFinder<USkeletalMesh> tempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonWraith/Characters/Heroes/Wraith/Meshes/Wraith.Wraith'"));

	//����, �ε尡 �����ߴٸ�
	if (tempMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(tempMesh.Object);
		// �޽ø� ���� ������ ĳ������ ��ġ ����� ȸ���� �ߴ�.
		// ��ġ (X=0.000000,Y=0.000000,Z=-90.000000)
		// ȸ�� (Pitch=0.000000,Yaw=-90.000000,Roll=0.000000)
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));

		bUseControllerRotationYaw = false;
		SpringArmComp->bUsePawnControlRotation = true;
		CameraComp->bUsePawnControlRotation = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;

		JumpMaxCount = 2;
		GetCharacterMovement()->AirControl = 1;

		// ������Ʈ ����() ����20240819����
		HandGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BerettaPistol"));
		// ������ Mesh�� ���δ�.
		HandGun->SetupAttachment(GetMesh());
		HandGun->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		ConstructorHelpers::FObjectFinder<USkeletalMesh> tempHandGun(TEXT("/Script/Engine.SkeletalMesh'/Game/Resources/GunBeretta/source/9mm_Hand_gun.9mm_Hand_gun'"));

		if (tempHandGun.Succeeded())
		{
			HandGun->SetSkeletalMesh(tempHandGun.Object);
			HandGun->SetRelativeLocationAndRotation(FVector(1.4f, 40.5f, 136.2f),FRotator(0,-70,0));
			HandGun->SetRelativeScale3D(FVector(2.f));
		}

		// �������� ���� �����ؼ� Mesh�� ���δ�.
		SniperGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CheyTacRifle"));
		SniperGun->SetupAttachment(GetMesh());
		// ���µ� �ε��ؼ� �����Ѵ�.
		ConstructorHelpers::FObjectFinder<USkeletalMesh> tempSniperGun(TEXT("/Script/Engine.SkeletalMesh'/Game/Resources/GunCheyTac/source/model.model'"));

		if (tempSniperGun.Succeeded())
		{
			SniperGun->SetSkeletalMesh(tempSniperGun.Object);
			SniperGun->SetRelativeLocation(FVector(-10.f, 85.f, 160.f));
			SniperGun->SetRelativeScale3D(FVector(0.02f));

		}
		

	}



}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	auto pc = Cast<APlayerController>(Controller);
	if (pc)
	{
		auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());

		if (subsystem)
		{
			subsystem->AddMappingContext(IMC_TPS, 0);
		}
	}

	// ����(Hand Gun)���� �⺻ ����
	ChangeToHandGun(FInputActionValue());
	
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move();
}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// �Է� �Լ����� ��� ����

	auto PlayerInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (PlayerInput)
	{
		PlayerInput->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &ATPSPlayer::Turn);
		PlayerInput->BindAction(IA_LookUp, ETriggerEvent::Triggered, this, &ATPSPlayer::LookUp);
		PlayerInput->BindAction(IA_PlayerMove, ETriggerEvent::Triggered, this, &ATPSPlayer::PlayerMove);
		PlayerInput->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ATPSPlayer::InputJump);
		PlayerInput->BindAction(IA_Fire, ETriggerEvent::Started, this, &ATPSPlayer::InputFire);
		PlayerInput->BindAction(IA_HandGun, ETriggerEvent::Started, this, &ATPSPlayer::ChangeToHandGun);
		PlayerInput->BindAction(IA_SniperGun, ETriggerEvent::Started, this, &ATPSPlayer::ChangeToSniperGun);
	}
}

void ATPSPlayer::Turn(const FInputActionValue& inputValue)
{
	float value = inputValue.Get<float>();
	AddControllerYawInput(value);
}

void ATPSPlayer::LookUp(const FInputActionValue& inputValue)
{
	float value = inputValue.Get<float>();
	AddControllerPitchInput(value);
}

void ATPSPlayer::PlayerMove(const FInputActionValue& inputValue)
{
	FVector2D value = inputValue.Get<FVector2D>();
	//���� �Է� ó��
	Direction.X = value.X;
	//�¿� �Է� ó��
	Direction.Y = value.Y;
}

void ATPSPlayer::InputJump(const FInputActionValue& inputValue)
{
	Jump();
}

void ATPSPlayer::Move()
{
	Direction = FTransform(GetControlRotation()).TransformVector(Direction);
	// ��� � ���� : P (�̵��� ��ġ) = P0 + (���� ��ġ) + V(�ӵ�*����) * T (�ð�)
	//FVector P0 = GetActorLocation;
	//FVector VT = Direction * walkSpeed * DeltaTime;
	AddMovementInput(FVector(Direction.X, Direction.Y, 0).GetSafeNormal());
	Direction = FVector::ZeroVector;
}

void ATPSPlayer::InputFire(const FInputActionValue& inputValue)
{
	FTransform FirePosition = GetMesh()->GetSocketTransform(TEXT("FirePosition"));
	GetWorld()->SpawnActor<ABullet>(BulletFactory, FirePosition);
}

void ATPSPlayer::ChangeToHandGun(const FInputActionValue& inputValue)
{
	// ����(Hand Gun)���
	bUsingHandGun = true;
	SniperGun->SetVisibility(false);
	HandGun->SetVisibility(true);
}

void ATPSPlayer::ChangeToSniperGun(const FInputActionValue& inputValue)
{
	bUsingHandGun = false;
	SniperGun->SetVisibility(true);
	HandGun->SetVisibility(false);
}





	