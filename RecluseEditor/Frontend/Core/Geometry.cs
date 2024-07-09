//
using System;
using RecluseEditor;

namespace RecluseEditor
{
    public class Vector2
    {
        public float x { get; set; }
        public float y { get; set; }

        public Vector2(float x = 0.0f, float y = 0.0f)
        {
            this.x = x;
            this.y = y;
        }
    }

    public class Vector3
    {
        public float x { get; set; }
        public float y { get; set; }
        public float z { get; set; }

        public Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }
    }

    public class Vector4
    {
        public float x { get; set; }
        public float y { get; set; }
        public float z { get; set; }
        public float w { get; set; }

        public Vector4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        
    }
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