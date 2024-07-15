using Recluse;
using Recluse.CSharp;
using RecluseEditor.Graphics;
using Recluse.CSharp.Pipeline;
using System;
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
            ShaderProgramBuilder ProgramBuilder = new ShaderProgramBuilder();

            RasterShaderProgramDescription GridDescription = new RasterShaderProgramDescription();
            GridDescription.Language = ShaderLanguage.Hlsl;

            //ProgramBuilder.PushDescription(GridDescription, (ulong)Shaders.Grid);
            //ProgramBuilder.Build(ShaderIntermediateLanguage.Spirv);
            //ProgramBuilder.SaveToDisk(shaderProgramFile);
            ProgramBuilder.Dispose();
            return true;
        }


        public static bool CleanUp()
        {
            return false;
        }
    }
}