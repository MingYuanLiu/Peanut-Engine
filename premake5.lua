-- premake5.lua
workspace "Peanut_Engine"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Peanut_Engine"
    location "Peanut_Engine"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "10.0.17763.0"
        
        defines
        {
            "PEANUTENGINE_EXPORTS",
            "PE_PLATFORM_WINDOWS",
            "PE_BUILD_DLL"
        }

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
        }
    
    filter "configurations:Debug"
        defines "PE_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "PE_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "PE_RELEASE"
        optimize "On"    


project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Peanut_Engine/vendor/spdlog/include",
        "Peanut_Engine/src"
    }

    links
    {
        "Peanut_Engine"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "10.0.17763.0"
    
        defines
        {
            "PE_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "PE_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "PE_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "PE_RELEASE"
        optimize "On"    
