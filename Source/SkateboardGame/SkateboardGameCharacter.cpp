// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkateboardGameCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


//////////////////////////////////////////////////////////////////////////
// ASkateboardGameCharacter

ASkateboardGameCharacter::ASkateboardGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	SkateMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Skateboard"));
	SkateMeshComponent->SetupAttachment(RootComponent);
}

void ASkateboardGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	SkeletalMeshComponent = GetComponentByClass<USkeletalMeshComponent>();
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	CurrentStamina = MaxStamina;
	DefaultSkateTransform = SkateMeshComponent->GetRelativeTransform();
}

void ASkateboardGameCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	HandleStamina(DeltaSeconds);
}

void ASkateboardGameCharacter::HandleStamina(float DeltaSeconds)
{
	if(GetCharacterMovement()->Velocity != FVector::Zero() &&
		IsBoosting)
	{
		if(CurrentStamina > 0.1f)
			CurrentStamina -= StaminaDrowningMultiplier * DeltaSeconds;
		else
			ToggleMovementBoost(false);
	}
	else
	{
		if(CurrentStamina < MaxStamina)
		{
			const auto staminaRecovery = StaminaRecoverMultiplier * DeltaSeconds;
			CurrentStamina = FMath::Clamp(CurrentStamina + staminaRecovery, CurrentStamina, MaxStamina);
		}
	}

	//UE_LOG(LogTemp, Log, TEXT("Stamina: %f"), CurrentStamina);
}

void ASkateboardGameCharacter::ToggleMovementBoost(bool Activate)
{
	if(Activate)
		GetCharacterMovement()->MaxWalkSpeed = BoostWalkSpeed;
	else
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;

	IsBoosting = Activate;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASkateboardGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASkateboardGameCharacter::PlayJumpAnimation);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkateboardGameCharacter::Move);

		//Boost
		EnhancedInputComponent->BindAction(ForwardBoostAction, ETriggerEvent::Started, this, &ASkateboardGameCharacter::ForwardBoostStart);
		EnhancedInputComponent->BindAction(ForwardBoostAction, ETriggerEvent::Completed, this, &ASkateboardGameCharacter::ForwardBoostEnd);
		
		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASkateboardGameCharacter::Look);
	}

}

void ASkateboardGameCharacter::OnScored()
{
	CurrentScore++;
	OnScoredDelegate.Broadcast(CurrentScore);
}

void ASkateboardGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASkateboardGameCharacter::ForwardBoostStart(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Boost start!"));

	ToggleMovementBoost(true);
}

void ASkateboardGameCharacter::ForwardBoostEnd(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Boost end!"));

	ToggleMovementBoost(false);
	
}

void ASkateboardGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASkateboardGameCharacter::PlayJumpAnimation()
{
	UAnimMontage* Montage = JumpMontage.LoadSynchronous();
	if (!Montage)
		return;

	const auto AnimInstance = SkeletalMeshComponent->GetAnimInstance();

	if(AnimInstance->Montage_IsPlaying(Montage))
		return;
	
	AnimInstance->OnPlayMontageNotifyBegin.AddUniqueDynamic(this, &ASkateboardGameCharacter::OnJumpAnimStart);
	AnimInstance->OnPlayMontageNotifyEnd.AddUniqueDynamic(this, &ASkateboardGameCharacter::OnJumpAnimEnd);
	AnimInstance->Montage_Play(Montage,JumpMontageSpeed);
}

void ASkateboardGameCharacter::OnJumpAnimStart(FName NotifyName,
	const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if(NotifyName.ToString() != JumpNotifyName)
		return;

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
	AttachmentRules.LocationRule = EAttachmentRule::SnapToTarget;
	AttachmentRules.ScaleRule = EAttachmentRule::KeepWorld;
	AttachmentRules.RotationRule = EAttachmentRule::KeepRelative;

	SkateMeshComponent->AttachToComponent(SkeletalMeshComponent, AttachmentRules, SkateSocketName);
}

void ASkateboardGameCharacter::OnJumpAnimEnd(FName NotifyName,
	const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if(NotifyName.ToString() != JumpNotifyName)
		return;

	const auto AnimInstance = SkeletalMeshComponent->GetAnimInstance();
	AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ASkateboardGameCharacter::OnJumpAnimStart);
	AnimInstance->OnPlayMontageNotifyEnd.RemoveDynamic(this, &ASkateboardGameCharacter::OnJumpAnimEnd);
	
	SkateMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
	AttachmentRules.LocationRule = EAttachmentRule::KeepRelative;
	AttachmentRules.ScaleRule = EAttachmentRule::KeepWorld;
	AttachmentRules.RotationRule = EAttachmentRule::KeepRelative;
	
	SkateMeshComponent->AttachToComponent(GetCapsuleComponent(), AttachmentRules, SkateSocketName);
	SkateMeshComponent->SetRelativeTransform(DefaultSkateTransform);
}