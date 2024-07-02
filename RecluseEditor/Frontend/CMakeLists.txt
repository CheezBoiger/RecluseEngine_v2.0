cmake_minimum_required(VERSION 3.8 )

set(RECLUSE_EDITOR_FRONTEND "RecluseEditor")

project(${RECLUSE_EDITOR_FRONTEND} VERSION 0.1.0 LANGUAGES CSharp)


include(CSharpUtilities)

add_executable(${RECLUSE_EDITOR_FRONTEND}
	AssemblyInfo.cs
	Content/MainWindow.xaml
	Backend/MainWindow.cs
	Content/Main.xaml
	Backend/Main.cs
	Content/SettingsWindow.xaml
	Backend/SettingsWindow.cs
	Content/NodeGraphWindow.xaml
	Backend/NodeGraphWindow.cs
	Content/ShaderGraphWindow.xaml
	Backend/ShaderGraphWindow.cs
	Content/SplashWindow.xaml
	Backend/SplashWindow.cs
	
	Content/Properties/SplashResources.Designer.cs
	Content/Properties/SplashResources.resx
	
	Content/Resources/Logo.jpg
	
	Core/Probe.cs
	Core/Geometry.cs
	Core/Renderer.cs)
	
csharp_set_designer_cs_properties(
	Content/Properties/SplashResources.Designer.cs
	Content/Properties/SplashResources.resx
	
	AssemblyInfo.cs
)
	
csharp_set_xaml_cs_properties(
	Content/SplashWindow.xaml
	Backend/SplashWindow.cs

	Content/MainWindow.xaml
	Backend/MainWindow.cs
	
	Content/Main.xaml
	Backend/Main.cs
	
	Content/SettingsWindow.xaml
	Backend/SettingsWindow.cs
	
	Content/NodeGraphWindow.xaml
	Backend/NodeGraphWindow.cs
	
	Content/ShaderGraphWindow.xaml
	Backend/ShaderGraphWindow.cs
	)
	
set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Content/Main.xaml PROPERTY VS_XAML_TYPE "ApplicationDefinition")
set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Content/Resources/Logo.jpg PROPERTY VS_TOOL_OVERRIDE "Resource")
	
set_property(TARGET ${RECLUSE_EDITOR_FRONTEND} PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8")
set_property(TARGET ${RECLUSE_EDITOR_FRONTEND} PROPERTY WIN32_EXECUTABLE TRUE)

set_property(TARGET ${RECLUSE_EDITOR_FRONTEND} PROPERTY VS_DOTNET_REFERENCES
	"Microsoft.CSharp"
	"PresentationCore"
	"PresentationFramework"
	"System"
	"System.Core"
	"System.Data"
	"System.Data.DataSetExtensions"
	"System.Net.Http"
	"System.Xaml"
	"System.Xml"
	"System.Xml.Linq"
	"WindowsBase"
	"System.Windows"
	"${CMAKE_SOURCE_DIR}/Recluse/bin/RecluseCSharpGraphics.dll")