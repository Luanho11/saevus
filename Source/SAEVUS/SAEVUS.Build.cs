// Copyright SAEVUS. Todos los derechos reservados.

using UnrealBuildTool;

public class SAEVUS : ModuleRules
{
	public SAEVUS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Permite incluir por carpeta de función, p. ej. "Movimiento/SAEVUSComponenteMovimiento.h".
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG"
		});

		// Tiempos por fotograma (hilo de juego/render, GPU) para la prueba de humo de depuración.
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"RenderCore",
			"RHI",
			// Interfaz construida en C++ (estilos, pinceles redondeados).
			"Slate",
			"SlateCore"
		});
	}
}
