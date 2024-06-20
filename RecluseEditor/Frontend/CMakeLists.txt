cmake_minimum_required(VERSION 3.8 )

set(RECLUSE_EDITOR_FRONTEND "RecluseEditor")

project(${RECLUSE_EDITOR_FRONTEND} VERSION 0.1.0 LANGUAGES CSharp)


include(CSharpUtilities)

add_executable(${RECLUSE_EDITOR_FRONTEND}
	MainWindow.xaml
	MainWindow.xaml.cs
	Main.xaml
	Main.xaml.cs)
	
csharp_set_xaml_cs_properties(
	MainWindow.xaml
	MainWindow.xaml.cs
	Main.xaml
	Main.xaml.cs)
	
set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Main.xaml PROPERTY VS_XAML_TYPE "ApplicationDefinition")
	
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