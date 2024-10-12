// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubSystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Bullet.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

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
		HandGun->SetupAttachment(GetMesh(),TEXT("FirePosition"));
		HandGun->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		ConstructorHelpers::FObjectFinder<USkeletalMesh> tempHandGun(TEXT("/Script/Engine.SkeletalMesh'/Game/Resources/GunBeretta/source/9mm_Hand_gun.9mm_Hand_gun'"));

		if (tempHandGun.Succeeded())
		{
			HandGun->SetSkeletalMesh(tempHandGun.Object);
			HandGun->SetRelativeLocationAndRotation(FVector(-28.f, -7.25f, 146.f),FRotator(0,-160,0));
			HandGun->SetRelativeScale3D(FVector(2.f));
		}

		// �������� ���� �����ؼ� Mesh�� ���δ�.
		SniperGun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CheyTacRifle"));
		SniperGun->SetupAttachment(GetMesh(),TEXT("ShootPosition"));
		SniperGun->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// ���µ� �ε��ؼ� �����Ѵ�.
		ConstructorHelpers::FObjectFinder<USkeletalMesh> tempSniperGun(TEXT("/Script/Engine.SkeletalMesh'/Game/Resources/GunCheyTac/source/model.model'"));

		if (tempSniperGun.Succeeded())
		{
			SniperGun->SetSkeletalMesh(tempSniperGun.Object);
			SniperGun->SetRelativeLocationAndRotation(FVector(-15.f, 85.f, 160.f),FRotator(0.f,0.f,0.f));
			SniperGun->SetRelativeScale3D(FVector(0.02f));

		}
		

	}



}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	// ������ �� �� ���� ������ �����Ѵ�.
	CrosshairUI = CreateWidget(GetWorld(), CrosshairUIfactory);
	SniperUI = CreateWidget(GetWorld(), SniperUIfactory);

	

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
		// �������� ���� ���
		PlayerInput->BindAction(IA_Sniper, ETriggerEvent::Started, this, &ATPSPlayer::SniperAim);
		PlayerInput->BindAction(IA_Sniper, ETriggerEvent::Completed, this, &ATPSPlayer::SniperAim);
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
	if (bUsingHandGun)
	{
		FTransform FirePosition = HandGun->GetSocketTransform(TEXT("FirePosition"));
		GetWorld()->SpawnActor<ABullet>(BulletFactory, FirePosition);
	}
	else if(bUsingSniperGun)
	{
		FTransform ShootPosition = SniperGun->GetSocketTransform(TEXT("ShootPosition"));
		GetWorld()->SpawnActor<ABullet>(BulletFactory, ShootPosition);

		//LineTrace�� ���� ��ġ
		FVector StartPosition = CameraComp->GetComponentLocation();
		// LineTrace�� ���� ��ġ
		FVector EndPosition = CameraComp->GetComponentLocation() + CameraComp->GetForwardVector() * 100000;
		// LineTrace �� �浹 ������ ���� ����
		FHitResult HitInfo;
		// �浹 �ɼ� ���� ����
		FCollisionQueryParams Params;
		// �ڱ� �ڽ�(�÷��̾�)�� �浹���� ����
		Params.AddIgnoredActor(this);
		// Channel ���͸� �̿��� LineTrace �浹 ����
		// �浹 ����, ���� ��ġ, ���� ��ġ, ���� ä��, �浹 �ɼ�
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, StartPosition, EndPosition, ECC_Visibility, Params);

		// �浹 ó�� -> �Ѿ� ���� ȿ�� ���
		if (bHit)
		{
			//�Ѿ� ���� ȿ�� Ʈ������
			FTransform BulletTransform;
			// �ε��� ��ġ �Ҵ�
			BulletTransform.SetLocation(HitInfo.ImpactPoint);
			// �Ѿ� ���� ȿ�� ��ü ����
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletEffectFactory,BulletTransform);
			
		}

		// �ε��� ��ü�� ������Ʈ�� ������ ����Ǿ� �ִٸ� ����������.
		auto HitComp = HitInfo.GetComponent();
		// 1. ���� ������Ʈ�� ������ ����Ǿ� �ִٸ�
		if (HitComp && HitComp->IsSimulatingPhysics())
		{
			// 2. ������ ������ �ʿ�
			FVector Dir = (EndPosition - StartPosition).GetSafeNormal();
			// �������� ����
			FVector force = Dir * HitComp->GetMass() * 500000;
			// 3. �� �������� ���������� �ʹ�.
			HitComp->AddForceAtLocation(force, HitInfo.ImpactPoint);
		}
	}
	
}

void ATPSPlayer::ChangeToHandGun(const FInputActionValue& inputValue)
{
	// ����(Hand Gun)���
	bUsingHandGun = true;
	SniperGun->SetVisibility(false);
	HandGun->SetVisibility(true);

	//CrosshairUI->AddToViewport();
	//SniperUI->RemoveFromParent();

	//CameraComp->FieldOfView = 90;
}

void ATPSPlayer::ChangeToSniperGun(const FInputActionValue& inputValue)
{
	// ���� ���
	bUsingHandGun = false;
	SniperGun->SetVisibility(true);
	HandGun->SetVisibility(false);
	
	//SniperUI->AddToViewport();
	//CrosshairUI->RemoveFromParent();

	//CameraComp->FieldOfView = 45;
}

void ATPSPlayer::SniperAim(const FInputActionValue& inputValue)
{
	// �������� ��尡 �ƴ� ��� ó������ �ʴ´�.
	if (bUsingHandGun)
	{
		return;
	}
	// Pressed(Started) �Է�ó��
	if(bSniperAim == false)
	{
		// �������� ���� ��� Ȱ��ȭ
		bSniperAim = true;
		// �������� ���� UI ���
		SniperUI->AddToViewport();
		CameraComp->SetFieldOfView(45.0f);
	}
	else //Released(Completed) �Է� ó��
	{
		bSniperAim = false;
		SniperUI->RemoveFromViewport();
		CameraComp->SetFieldOfView(90.f);
	}

}





	