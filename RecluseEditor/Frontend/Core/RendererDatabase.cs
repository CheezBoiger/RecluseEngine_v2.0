using Recluse;
using Recluse.CSharp;
using RecluseEditor.Graphics;
using Recluse.CSharp.Pipeline;
using System;
using System.IO;
using System.Security.Policy;
using RecluseEditor.Math;
using Microsoft.CSharp;

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
            ShaderIntermediateLanguage Imm = ShaderIntermediateLanguage.Spirv;
            if (Device.GetApi() == GraphicsApi.Direct3D12)
            { 
                Compiler = ShaderCompiler.DXC;
                Imm = ShaderIntermediateLanguage.Dxil;
            }
            ShaderProgramBuilder ProgramBuilder = new ShaderProgramBuilder(Compiler);

            SetupVertexLayouts(Device);
            InitializeGridShader(ProgramBuilder);


            ProgramBuilder.Build(Imm);
            return ProgramBuilder.LoadToRuntime(Device);
        }


        public static bool CleanUp()
        {
            return false;
        }


        private static bool InitializeGridShader(ShaderProgramBuilder Builder)
        {
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

            Builder.PushDescription(GridDescription, (ulong)Shaders.Grid);
            return true;
        }


        private static bool SetupVertexLayouts(IGraphicsDevice Device)
        {
            IVertexInputLayout PositionOnlyLayout = new IVertexInputLayout();
            IVertexBinding PosBinding = new IVertexBinding();
            PosBinding.Rate = InputRate.PerVertex;
            PosBinding.Binding = 0;
            PosBinding.Stride = 16;
            
            PosBinding.VertexAttributes.Add(new IVertexAttribute(
                ResourceFormat.R32G32B32A32_Float, 
                unchecked((uint)IVertexAttribute.Helper.OffsetAppend), 
                0, 
                Semantic.Position, 
                0));

            PositionOnlyLayout.VertexBindings.Add(PosBinding);
            Device.MakeVertexLayout((ulong)Vertex.PositionOnly, PositionOnlyLayout);
            return false;
        }
    }
}