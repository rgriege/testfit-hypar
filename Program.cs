using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Hypar.Elements;
using Hypar.Geometry;

namespace dotnet_test
{
    [StructLayout(LayoutKind.Sequential)]
    struct v2f
    {
        public float x;
        public float y;
    }

    class MeshElement : Proxy
    {
        private Mesh _mesh;

        public MeshElement(Mesh mesh) : base(null, null)
        {
            this._mesh = mesh;
        }

        public override Mesh Tessellate()
        {
            return this._mesh;
        }
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
						IntPtr poly_verts;
						IntPtr poly_lengths;
						uint num_verts;
						uint num_lengths;
						int ret;

            ret = tfmp_generate_bldg(boundary, (uint)boundary.Length,
                                     60.0f, 40.0f, 0.5f,
                                     out poly_verts, out num_verts,
                                     out poly_lengths, out num_lengths);

            if (ret != 0) {
                Console.WriteLine("tfmp_generate_bldg() failed");
                return;
            }

            List<Polyline> polylines = new List<Polyline>();
            int poly_vert_start = 0;
            for (int i = 0; i < num_lengths; ++i)
            {
                IntPtr lengthp = IntPtr.Add(poly_lengths, i * sizeof(uint));
                int length = (int)Marshal.PtrToStructure<uint>(lengthp);
                List<Vector3> verts = new List<Vector3>();
                for (int j = 0; j < length; ++j)
                {
                    IntPtr vertp = IntPtr.Add(poly_verts, (poly_vert_start + j) * Marshal.SizeOf<v2f>());
                    v2f vert = Marshal.PtrToStructure<v2f>(vertp);
                    verts.Add(new Vector3(vert.x, vert.y));
                }
                verts.Reverse();
                polylines.Add(new Polyline(verts));
                poly_vert_start += length;
            }

            tfmp_free(poly_verts, poly_lengths);

            var model = new Model();
            var mesh = new MeshElement(Mesh.Extrude(polylines, 10.0));
            model.AddElement(mesh);
            model.SaveGlb("test.glb");
        }
    }
}
