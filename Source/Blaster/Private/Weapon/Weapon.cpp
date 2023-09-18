// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;

	skeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("skeletalMeshComp"));
	//skeletalMeshComp->SetupAttachment(RootComponent);
	SetRootComponent(skeletalMeshComp);

	skeletalMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	skeletalMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	skeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	sphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("sphereComp"));
	sphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	sphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	sphereComp->SetupAttachment(RootComponent);

	pickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("pickupWidget"));
	pickupWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// on server
	if (HasAuthority())
	{
		sphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		sphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		sphereComp->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		sphereComp->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
	
	if (pickupWidget)
	{
		pickupWidget->SetVisibility(false);
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, weaponState);
}

void AWeapon::SetWeaponState(EWeaponState state)
{
	weaponState = state;
	switch (weaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickWidget(false);
		sphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	default:
		break;
	}
}

void AWeapon::ShowPickWidget(bool bIsShow)
{
	if (pickupWidget)
	{
		pickupWidget->SetVisibility(bIsShow);
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* blasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (blasterCharacter)
	{
		blasterCharacter->SetOverlappingWeapon(this);
		//UE_LOG(LogTemp, Warning, TEXT("OnSphereOverlap"));
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* blasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (blasterCharacter)
	{
		blasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (weaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickWidget(false);
		break;
	default:
		break;
	}
}

