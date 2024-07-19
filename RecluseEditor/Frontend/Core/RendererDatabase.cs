using Recluse;
using Recluse.CSharp;
using RecluseEditor.Graphics;
using Recluse.CSharp.Pipeline;
using System;
using System.IO;
using System.Security.Policy;

namespace RecluseEditor
{
    public static class Database
    {
        private static string shaderProgramFile = "EditorShaderPrograms.txt";
        public enum Shaders {
            Grid = 999,
            Debug = 998,
        }

        public enum Vertex
        {
            PositionOnly,
            Position_Normal,
            Position_Normal_UV0,
            Position_Normal_UV0_UV1
        }

        public static bool Initialize(IGraphicsDevice Device)
        {
            ShaderCompiler Compiler = ShaderCompiler.GLSLang;
            if (Device.GetApi() == GraphicsApi.Direct3D12)
                Compiler = ShaderCompiler.DXC;
            ShaderProgramBuilder ProgramBuilder = new ShaderProgramBuilder(Compiler);

            VertexRasterShaderProgramDescription GridDescription = new VertexRasterShaderProgramDescription();
            GridDescription.Language = ShaderLanguage.Hlsl;
            if (File.Exists("ShaderGrid.vs.hlsl"))
            {
                GridDescription.VS = File.ReadAllText("ShaderGrid.vs.hlsl");
                GridDescription.VSName = "MainVs";
            }

            if (File.Exists("ShaderGrid.ps.hlsl"))
            {
                GridDescription.PS = File.ReadAllText("ShaderGrid.ps.hlsl");
                GridDescription.PSName = "MainPs";
            }

            ProgramBuilder.PushDescription(GridDescription, (ulong)Shaders.Grid);
            if (ProgramBuilder.Build(ShaderIntermediateLanguage.Dxil))
            {
                ProgramBuilder.LoadToRuntime(Device);
            }
            return true;
        }


        public static bool CleanUp()
        {
            return false;
        }
    }
}