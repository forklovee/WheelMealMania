// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InputBufferComponent.h"

UInputBufferComponent::UInputBufferComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UInputBufferComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInputBufferComponent::PushInput(FString InputName)
{
	Buffer.Push(InputName);
}


