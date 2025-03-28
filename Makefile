# Makefile generated by MakefileGenerator.cs
# *DO NOT EDIT*

UNREALROOTPATH = /home/forklovee/Dev/Linux_Unreal_Engine_5.5.1

TARGETS = \
	LiveLinkHub-Linux-DebugGame  \
	LiveLinkHub-Linux-Development  \
	LiveLinkHub \
	DotNetPerforceLib \
	EventLoopUnitTests \
	UnrealEditor-Linux-DebugGame  \
	UnrealEditor-Linux-Development  \
	UnrealEditor \
	UnrealGame-Android-DebugGame  \
	UnrealGame-Android-Development  \
	UnrealGame-Android-Shipping  \
	UnrealGame-Linux-DebugGame  \
	UnrealGame-Linux-Development  \
	UnrealGame-Linux-Shipping  \
	UnrealGame-LinuxArm64-DebugGame  \
	UnrealGame-LinuxArm64-Development  \
	UnrealGame-LinuxArm64-Shipping  \
	UnrealGame \
	WheelMealMania-Android-DebugGame  \
	WheelMealMania-Android-Development  \
	WheelMealMania-Android-Shipping  \
	WheelMealMania-Linux-DebugGame  \
	WheelMealMania-Linux-Development  \
	WheelMealMania-Linux-Shipping  \
	WheelMealMania-LinuxArm64-DebugGame  \
	WheelMealMania-LinuxArm64-Development  \
	WheelMealMania-LinuxArm64-Shipping  \
	WheelMealMania \
	WheelMealManiaEditor-Linux-DebugGame  \
	WheelMealManiaEditor-Linux-Development  \
	WheelMealManiaEditor\
	configure

BUILD = "$(UNREALROOTPATH)/Engine/Build/BatchFiles/RunUBT.sh"

all: StandardSet

RequiredTools: CrashReportClient-Linux-Shipping CrashReportClientEditor-Linux-Shipping ShaderCompileWorker UnrealLightmass EpicWebHelper-Linux-Shipping

StandardSet: RequiredTools UnrealFrontend WheelMealManiaEditor UnrealInsights

DebugSet: RequiredTools UnrealFrontend-Linux-Debug WheelMealManiaEditor-Linux-Debug


LiveLinkHub-Linux-DebugGame:
	 $(BUILD) LiveLinkHub Linux DebugGame  $(ARGS)

LiveLinkHub-Linux-Development:
	 $(BUILD) LiveLinkHub Linux Development  $(ARGS)

LiveLinkHub: LiveLinkHub-Linux-Development

DotNetPerforceLib: DotNetPerforceLib-Linux-Development

EventLoopUnitTests: EventLoopUnitTests-Linux-Development

UnrealEditor-Linux-DebugGame:
	 $(BUILD) UnrealEditor Linux DebugGame  $(ARGS)

UnrealEditor-Linux-Development:
	 $(BUILD) UnrealEditor Linux Development  $(ARGS)

UnrealEditor: UnrealEditor-Linux-Development

UnrealGame-Android-DebugGame:
	 $(BUILD) UnrealGame Android DebugGame  $(ARGS)

UnrealGame-Android-Development:
	 $(BUILD) UnrealGame Android Development  $(ARGS)

UnrealGame-Android-Shipping:
	 $(BUILD) UnrealGame Android Shipping  $(ARGS)

UnrealGame-Linux-DebugGame:
	 $(BUILD) UnrealGame Linux DebugGame  $(ARGS)

UnrealGame-Linux-Development:
	 $(BUILD) UnrealGame Linux Development  $(ARGS)

UnrealGame-Linux-Shipping:
	 $(BUILD) UnrealGame Linux Shipping  $(ARGS)

UnrealGame-LinuxArm64-DebugGame:
	 $(BUILD) UnrealGame LinuxArm64 DebugGame  $(ARGS)

UnrealGame-LinuxArm64-Development:
	 $(BUILD) UnrealGame LinuxArm64 Development  $(ARGS)

UnrealGame-LinuxArm64-Shipping:
	 $(BUILD) UnrealGame LinuxArm64 Shipping  $(ARGS)

UnrealGame: UnrealGame-Linux-Development

WheelMealMania-Android-DebugGame:
	 $(BUILD) WheelMealMania Android DebugGame  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealMania-Android-Development:
	 $(BUILD) WheelMealMania Android Development  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealMania-Android-Shipping:
	 $(BUILD) WheelMealMania Android Shipping  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealMania-Linux-DebugGame:
	 $(BUILD) WheelMealMania Linux DebugGame  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealMania-Linux-Development:
	 $(BUILD) WheelMealMania Linux Development  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealMania-Linux-Shipping:
	 $(BUILD) WheelMealMania Linux Shipping  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealMania-LinuxArm64-DebugGame:
	 $(BUILD) WheelMealMania LinuxArm64 DebugGame  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealMania-LinuxArm64-Development:
	 $(BUILD) WheelMealMania LinuxArm64 Development  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealMania-LinuxArm64-Shipping:
	 $(BUILD) WheelMealMania LinuxArm64 Shipping  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealMania: WheelMealMania-Linux-Development

WheelMealManiaEditor-Linux-DebugGame:
	 $(BUILD) WheelMealManiaEditor Linux DebugGame  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealManiaEditor-Linux-Development:
	 $(BUILD) WheelMealManiaEditor Linux Development  -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" $(ARGS)

WheelMealManiaEditor: WheelMealManiaEditor-Linux-Development

configure:
	$(BUILD) -ProjectFiles -Project="/home/forklovee/Documents/Unreal Projects/WheelMealMania/WheelMealMania.uproject" -Game 

.PHONY: $(TARGETS)
