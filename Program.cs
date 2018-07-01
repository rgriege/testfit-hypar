using System;
using System.Runtime.InteropServices;

namespace dotnet_test
{
    struct v2f
    {
        public float x;
        public float y;
    }

    class Program
    {
        [DllImport("libtfmp.so")]
        public static extern void tfmp_generate_bldg(v2f[] boundary, uint n,
                                                     float mass_width,
                                                     float mass_height,
                                                     float aspect_ratio);

        static void Main(string[] args)
        {
            v2f[] boundary = { new v2f{ x=5.0f, y=42.0f } };
            tfmp_generate_bldg(boundary, (uint)boundary.Length,
                               60.0f, 40.0f, 0.5f);
        }
    }
}
