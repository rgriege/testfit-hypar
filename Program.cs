using System;
using System.Runtime.InteropServices;

namespace dotnet_test
{
    [StructLayout(LayoutKind.Sequential)]
    struct v2f
    {
        public float x;
        public float y;
    }

    class Program
    {
        [DllImport("libtfmp.so")]
        public static extern int tfmp_generate_bldg(v2f[] boundary, uint n,
                                                    float mass_width,
                                                    float mass_height,
                                                    float aspect_ratio,
                                                    out IntPtr poly_verts,
                                                    out uint num_verts,
                                                    out IntPtr poly_lengths,
                                                    out uint num_lengths);

        [DllImport("libtfmp.so")]
        public static extern int tfmp_free(IntPtr poly_verts,
                                           IntPtr poly_lengths);

        static void Main(string[] args)
        {
            v2f[] boundary = {
                new v2f{ x=0.0f,   y=0.0f },
                new v2f{ x=200.0f, y=0.0f },
                new v2f{ x=200.0f, y=200.0f },
                new v2f{ x=0.0f,   y=200.0f },
            };
						IntPtr poly_verts_;
						IntPtr poly_lengths_;
						uint num_verts;
						uint num_lengths;
						int ret;

            ret = tfmp_generate_bldg(boundary, (uint)boundary.Length,
                                     60.0f, 40.0f, 0.5f,
                                     out poly_verts_, out num_verts,
                                     out poly_lengths_, out num_lengths);

            if (ret != 0) {
                Console.WriteLine("tfmp_generate_bldg() failed");
                return;
            }

            v2f[] poly_verts = new v2f[num_verts];
            uint[] poly_lengths = new uint[num_lengths];
						for (int i = 0; i < num_verts; ++i)
						{
							IntPtr poly_vert_ = IntPtr.Add(poly_verts_, i * Marshal.SizeOf<v2f>());
							poly_verts[i] = Marshal.PtrToStructure<v2f>(poly_vert_);
						}
						for (int i = 0; i < num_lengths; ++i)
						{
							IntPtr poly_length_ = IntPtr.Add(poly_lengths_, i * sizeof(uint));
							poly_lengths[i] = Marshal.PtrToStructure<uint>(poly_length_);
						}

            tfmp_free(poly_verts_, poly_lengths_);

            uint vert_start = 0;
						for (uint i = 0; i < num_lengths; ++i)
            {
                Console.Write("Polygon: ");
                for (uint j = 0; j < poly_lengths[i]; ++j)
                    Console.Write("<{0},{1}>,", poly_verts[vert_start+j].x,
                                  poly_verts[vert_start+j].y);
                Console.Write("\n");
                vert_start += poly_lengths[i];
            }
        }
    }
}
