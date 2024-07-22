#pragma once

UENUM(BlueprintType)

enum class EEnemyState : uint8
{
	EES_NoState UMETA(DisplayName = "NoState"),
	EES_HitReacting UMETA(DisplayName = "HitReacting"),
	EES_Attacking UMETA(DisplayName = "Attacking")

};