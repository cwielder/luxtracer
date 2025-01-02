-- premake5.lua
workspace "LumiTracer"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "LumiTracer"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "LumiTracer"
