using Recluse;
using Recluse.CSharp;
using RecluseEditor.Graphics;
using Recluse.CSharp.Pipeline;
using System;

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

        public static bool Initialize(IGraphicsDevice device)
        {
            ShaderProgramBuilder ProgramBuilder = new ShaderProgramBuilder();

            RasterShaderProgramDescription GridDescription = new RasterShaderProgramDescription();
            GridDescription.ProgramId = (ulong)Shaders.Grid;
            GridDescription.Language = ShaderLanguage.Hlsl;

            ProgramBuilder.PushDescription(GridDescription);
            ProgramBuilder.Build(ShaderIntermediateLanguage.Spirv);
            return false;
        }


        public static bool CleanUp()
        {
            return false;
        }
    }
}