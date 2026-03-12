// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoMapTagComponent.h"

// Sets default values for this component's properties
UAutoMapTagComponent::UAutoMapTagComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UAutoMapTagComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UAutoMapTagComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

