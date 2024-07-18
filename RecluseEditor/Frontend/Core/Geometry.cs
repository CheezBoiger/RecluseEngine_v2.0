//
using System;
using RecluseEditor;
using RecluseEditor.Math;

namespace RecluseEditor
{

    public interface Geometry
    {
        Vector3[] GetPositions(float Scale = 1.0f);
        Vector3[] GetNormals();
        Vector4[] GetUVs();
        uint[] GetIndices();
    }


    public class Cube : Geometry
    {
        public Cube()
        {

        }

        public Vector3[] GetPositions(float Scale = 1.0f)
        {
            Vector3[] v = new Vector3[36];
            return v;
        }

        public Vector3[] GetNormals()
        {
            return new Vector3[36];
        }

        public Vector4[] GetUVs()
        {
            return new Vector4[36];
        }

        public uint[] GetIndices()
        {
            return new uint[36];
        }
    }
}