using UnrealBuildTool;

public class ZipPlatformFile : ModuleRules
{
	public ZipPlatformFile(ReadOnlyTargetRules Target) : base(Target)
	{
#if UE_5_2_OR_LATER
        IWYUSupport = IWYUSupport.None;
#else
		bEnforceIWYU = false;
#endif
		bUseUnity = false;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnableUndefinedIdentifierWarnings = false;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			});

        PrivateDependencyModuleNames.AddRange(new string[] {
			"zlib"
			});
        
        PrivateDefinitions.AddRange(new string[] {
	        "_FILE_OFFSET_BITS=64"
        });
	}
}
