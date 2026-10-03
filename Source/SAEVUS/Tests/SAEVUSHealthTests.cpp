// Copyright SAEVUS. All Rights Reserved.

#include "Tests/SAEVUSMovementTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/SAEVUSHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

namespace SAEVUSHealthTests
{
	const FVector FloorCenter(0.f, 0.f, -50.f);
	const FVector FloorSize(4000.f, 4000.f, 100.f);
	const FVector StandLocation(0.f, 0.f, 150.f);
	const FVector PropLocation(500.f, 0.f, 100.f);

	constexpr float Hit = 30.f;
	constexpr float Overkill = 1000.f;
	constexpr float SmallHeal = 10.f;
	constexpr float EngineDamage = 25.f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSHealthRulesTest, "SAEVUS.Health.Rules", SAEVUSTests::TestFlags)
bool FSAEVUSHealthRulesTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSHealthTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	TestWorld.SpawnBlock(FloorCenter, FloorSize);
	ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(StandLocation);
	USAEVUSHealthComponent* Health = Character->GetHealth();
	const float Max = Health->GetMaxHealth();

	TestEqual(TEXT("Starts at full health"), Health->GetHealth(), Max);
	TestEqual(TEXT("Damage removes health"), Health->ApplyDamage(Hit, nullptr), Hit);
	TestEqual(TEXT("Health after the hit"), Health->GetHealth(), Max - Hit);
	TestEqual(TEXT("Heal adds health"), Health->Heal(SmallHeal), SmallHeal);
	TestEqual(TEXT("Heal never goes past max"), Health->Heal(Overkill), Hit - SmallHeal);
	TestEqual(TEXT("Zero or negative damage does nothing"), Health->ApplyDamage(-Hit, nullptr), 0.f);

	TestEqual(TEXT("Overkill only removes what is left"), Health->ApplyDamage(Overkill, nullptr), Max);
	TestTrue(TEXT("Dead at zero"), Health->IsDead());
	TestEqual(TEXT("No damage while dead"), Health->ApplyDamage(Hit, nullptr), 0.f);
	TestEqual(TEXT("No healing while dead"), Health->Heal(SmallHeal), 0.f);

	Health->ResetHealth();
	TestTrue(TEXT("Reset: alive at full health"), !Health->IsDead() && Health->GetHealth() == Max);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSHealthEngineDamageTest, "SAEVUS.Health.EngineDamage", SAEVUSTests::TestFlags)
bool FSAEVUSHealthEngineDamageTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSHealthTests;

	// Pawns ignore damage in a world without a game mode (APawn::ShouldTakeDamage), so the engine
	// damage path is checked on a plain actor; the rendered smoke test covers it on the player.
	SAEVUSTests::FScopedTestWorld TestWorld;
	AActor* Prop = TestWorld.SpawnActor<AActor>(PropLocation);
	USAEVUSHealthComponent* Health = NewObject<USAEVUSHealthComponent>(Prop);
	Health->RegisterComponent();
	const float Max = Health->GetMaxHealth();

	UGameplayStatics::ApplyDamage(Prop, EngineDamage, nullptr, nullptr, UDamageType::StaticClass());
	TestEqual(TEXT("Engine damage reaches the health component"), Health->GetHealth(), Max - EngineDamage);

	UGameplayStatics::ApplyDamage(Prop, EngineDamage, nullptr, Prop, UDamageType::StaticClass());
	TestEqual(TEXT("Self damage is ignored"), Health->GetHealth(), Max - EngineDamage);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSHealthCharacterDeathTest, "SAEVUS.Health.CharacterDeath", SAEVUSTests::TestFlags)
bool FSAEVUSHealthCharacterDeathTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSHealthTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	TestWorld.SpawnBlock(FloorCenter, FloorSize);
	ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(StandLocation);
	if (!TestTrue(TEXT("Character settled"), TestWorld.SettleOnGround()))
	{
		return false;
	}

	// No game mode and no controller here: death must still be safe.
	Character->GetHealth()->ApplyDamage(Overkill, nullptr);
	TestTrue(TEXT("Dead"), Character->GetHealth()->IsDead());
	TestEqual(TEXT("Movement stops"), Character->GetSAEVUSMovement()->MovementMode.GetValue(), MOVE_None);
	TestEqual(TEXT("Capsule stops colliding"), Character->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);

	const FVector Location = Character->GetActorLocation();
	Character->RequestJump();
	Character->RequestDash();
	TestWorld.Step();
	TestTrue(TEXT("A dead character doesn't move"), Character->GetActorLocation().Equals(Location));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
