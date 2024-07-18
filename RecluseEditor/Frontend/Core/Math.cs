//
using System;
using System.CodeDom;
using RecluseEditor;

namespace RecluseEditor
{
    namespace Math
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

            public static Vector2 operator+(Vector2 v0, Vector2 v1)
            {
                return new Vector2(v0.x + v1.x, v0.y + v1.y);
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

            public static Vector3 operator+(Vector3 v1, Vector3 v2)
            {
                return new Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
            }

            public static Vector3 operator-(Vector3 v1, Vector3 v2)
            {
                return new Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
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
    }
}