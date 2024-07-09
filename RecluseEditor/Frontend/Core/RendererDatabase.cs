using Recluse;
using Recluse.CSharp;
using RecluseEditor.Graphics;
using System;

namespace RecluseEditor
{
    public static class Database
    {
        enum Shaders {
            Grid,
            Debug,
        }

        public static bool Initialize(IGraphicsDevice device)
        {
            return false;
        }


        public static bool CleanUp()
        {
            return false;
        }
    }
}