// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SkateboardGameCharacter.generated.h"


UCLASS(config=Game)
class ASkateboardGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Move Boost Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ForwardBoostAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="SK8 - Movement" )
	float DefaultWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, Category="SK8 - Movement")
	float BoostWalkSpeed = 800.0f;

	UPROPERTY()
	bool IsBoosting = false;
	
	UPROPERTY(EditAnywhere, Category="SK8 - Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(VisibleAnywhere, Category= "SK8 - Stamina")
	float CurrentStamina = 0;

	UPROPERTY(EditAnywhere, Category="SK8 - Stamina")
	float StaminaDrowningMultiplier = 10.f;
	
	UPROPERTY(EditAnywhere, Category="SK8 - Stamina")
	float StaminaRecoverMultiplier = 2.f;
	
public:
	ASkateboardGameCharacter();
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	void ForwardBoostStart(const FInputActionValue& Value);
	void ForwardBoostEnd(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void HandleStamina(float DeltaSeconds);
	void ToggleMovementBoost(bool Activate);
};

