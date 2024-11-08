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
#include "Kismet/KismetMathLibrary.h"
#include "EnemyFSM.h"
#include "PlayerAnim.h"

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
	SpringArmComp->SetRelativeLocation(FVector(0, 20, 80));

	// Camera를 생성해서 SpringArm에 붙인다.
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
	
	// Mesh에 3D 에셋을 로드해서 넣어준다.
	// 생성자 도우미를 이용해서 스켈레탈 메쉬를 로드한다.
	ConstructorHelpers::FObjectFinder<USkeletalMesh> tempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonWraith/Characters/Heroes/Wraith/Meshes/Wraith.Wraith'"));

	//만약, 로드가 성공했다면
	if (tempMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(tempMesh.Object);
		// 메시를 넣은 다음에 캐릭터의 위치 변경과 회전을 했다.
		// 위치 (X=0.000000,Y=0.000000,Z=-90.000000)
		// 회전 (Pitch=0.000000,Yaw=-90.000000,Roll=0.000000)
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));

		bUseControllerRotationYaw = true;
		SpringArmComp->bUsePawnControlRotation = true;
		CameraComp->bUsePawnControlRotation = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;

		JumpMaxCount = 2;
		GetCharacterMovement()->AirControl = 1;

		// 
		GetMesh()->SetupAttachment(GetMesh(), TEXT("FirePositon"));

	}



}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	// 초기 속도를 걷기로 설정
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// 시작할 때 두 개의 위젯을 생성한다.
	CrosshairUI = CreateWidget(GetWorld(), CrosshairUIfactory);
	SniperUI = CreateWidget(GetWorld(), SniperUIfactory);
	// 일반 조준 모드 CrosshairUI 화면에 표시
	CrosshairUI->AddToViewport();

	

	auto pc = Cast<APlayerController>(Controller);
	if (pc)
	{
		auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());

		if (subsystem)
		{
			subsystem->AddMappingContext(IMC_TPS, 0);
		}
	}
	
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
	// 입력 함수들을 모두 연결

	auto PlayerInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (PlayerInput)
	{
		PlayerInput->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &ATPSPlayer::Turn);
		PlayerInput->BindAction(IA_LookUp, ETriggerEvent::Triggered, this, &ATPSPlayer::LookUp);
		PlayerInput->BindAction(IA_PlayerMove, ETriggerEvent::Triggered, this, &ATPSPlayer::PlayerMove);
		PlayerInput->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ATPSPlayer::InputJump);
		// 달리기 애니메이션
		PlayerInput->BindAction(IA_Run, ETriggerEvent::Started, this, &ATPSPlayer::InputRun);
		PlayerInput->BindAction(IA_Run, ETriggerEvent::Completed, this, &ATPSPlayer::InputRun);


		PlayerInput->BindAction(IA_Fire, ETriggerEvent::Started, this, &ATPSPlayer::InputFire);
		PlayerInput->BindAction(IA_RifleMode, ETriggerEvent::Started, this, &ATPSPlayer::ChangeToRifle);
		PlayerInput->BindAction(IA_SniperMode, ETriggerEvent::Started, this, &ATPSPlayer::ChangeToSniper);
		// 스나이퍼 조준 모드
		PlayerInput->BindAction(IA_SniperAim, ETriggerEvent::Started, this, &ATPSPlayer::SniperAim);
		PlayerInput->BindAction(IA_SniperAim, ETriggerEvent::Completed, this, &ATPSPlayer::SniperAim);
		

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
	//상하 입력 처리
	Direction.X = value.X;
	//좌우 입력 처리
	Direction.Y = value.Y;
}

void ATPSPlayer::InputRun()
{
	auto Movement = GetCharacterMovement();
	// 만약 , 현재 달리기 모드라면
	if (Movement->MaxWalkSpeed > WalkSpeed)
	{
		// 걷기 속도로 전환
		Movement->MaxWalkSpeed = WalkSpeed;
	}
	else
	{
		Movement->MaxWalkSpeed = RunSpeed;
	}
}

void ATPSPlayer::InputJump(const FInputActionValue& inputValue)
{
	Jump();
}

void ATPSPlayer::Move()
{
	Direction = FTransform(GetControlRotation()).TransformVector(Direction);
	// 등속 운동 공식 : P (이동할 위치) = P0 + (현재 위치) + V(속도*방향) * T (시간)
	//FVector P0 = GetActorLocation;
	//FVector VT = Direction * walkSpeed * DeltaTime;
	AddMovementInput(FVector(Direction.X, Direction.Y, 0).GetSafeNormal());
	Direction = FVector::ZeroVector;
}

void ATPSPlayer::InputFire(const FInputActionValue& inputValue)
{
	auto Anim = Cast<UPlayerAnim>(GetMesh()->GetAnimInstance());
	Anim->PlayAttackAnim();

	//공통된 처리
	// 총알이 나가는 Transform을 가져옴
	FTransform FirePosition = GetMesh()->GetSocketTransform(TEXT("FirePosition"));
	
	// 그곳에 총알이 발사되는 이펙트를 생성

	
	
	//공통된 처리
	if (bUsingRifle)
	{
		//라이플 모드일때 일반 총알을 쏜다.
		GetWorld()->SpawnActor<ABullet>(BulletFactory, FirePosition);
		
	}
	else if(bUsingSniper)
	{
		//스나이퍼 모드일때

		//LineTrace의 시작 위치
		FVector StartPosition = CameraComp->GetComponentLocation();
		// LineTrace의 종료 위치
		FVector EndPosition = CameraComp->GetComponentLocation() + CameraComp->GetForwardVector() * 100000;
		// LineTrace 의 충돌 정보를 담을 변수
		FHitResult HitInfo;
		// 충돌 옵션 설정 변수
		FCollisionQueryParams Params;
		// 자기 자신(플레이어)는 충돌에서 제외
		Params.AddIgnoredActor(this);
		// Channel 필터를 이용한 LineTrace 충돌 검출
		// 충돌 정보, 시작 위치, 종료 위치, 검출 채널, 충돌 옵션
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, StartPosition, EndPosition, ECC_Visibility, Params);

		// 충돌 처리 -> 총알 파편 효과 재생
		if (bHit)
		{
			//총알 파편 효과 트랜스폼
			FTransform BulletTransform;
			// 부딪힌 위치 할당
			BulletTransform.SetLocation(HitInfo.ImpactPoint);
			BulletTransform.SetRotation(UKismetMathLibrary::Conv_VectorToQuaternion(HitInfo.ImpactNormal));
			// 총알 파편 효과 객체 생성
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletHitEffectFactory,BulletTransform);
			
		}

		// 부딪힌 물체의 컴포넌트에 물리가 적용되어 있다면 날려버린다.
		auto HitComp = HitInfo.GetComponent();
		// 1. 만약 컴포넌트에 물리가 적용되어 있다면
		if (HitComp && HitComp->IsSimulatingPhysics())
		{
			// 2. 조준한 방향이 필요
			FVector Dir = (EndPosition - StartPosition).GetSafeNormal();
			// 날려버릴 방향
			FVector force = Dir * HitComp->GetMass() * 500000;
			// 3. 그 방향으로 날려버리고 싶다.
			HitComp->AddForceAtLocation(force, HitInfo.ImpactPoint);
		}

		// 부딪힌 대상이 적인지 판단
		auto Enemy = HitInfo.GetActor()->GetDefaultSubobjectByName(TEXT("EnemyFSM"));

		if (Enemy)
		{
			auto EnemyFSM = Cast<UEnemyFSM>(Enemy);
			EnemyFSM->OnDamageProcess();
		}
	}
	
}

void ATPSPlayer::ChangeToRifle(const FInputActionValue& inputValue)
{
	// 권총(Hand Gun)사용
	bUsingRifle = true;
}

void ATPSPlayer::ChangeToSniper(const FInputActionValue& inputValue)
{
	// 소총 사용
	bUsingRifle = false;
	
}

void ATPSPlayer::SniperAim(const FInputActionValue& inputValue)
{
	// 스나이퍼 모드가 아닐 경우 처리하지 않는다.
	if (bUsingRifle)
	{
		return;
	}
	// Pressed(Started) 입력처리
	if(bSniperAim == false)
	{
		// 스나이퍼 조준 모드 활성화
		bSniperAim = true;
		// 스나이퍼 조준 UI 등록
		SniperUI->AddToViewport();
		CameraComp->SetFieldOfView(45.0f);
		CrosshairUI->RemoveFromParent();
	}
	else //Released(Completed) 입력 처리
	{
		bSniperAim = false;
		SniperUI->RemoveFromViewport();
		CameraComp->SetFieldOfView(90.f);
		CrosshairUI->AddToViewport();
	}

}





	