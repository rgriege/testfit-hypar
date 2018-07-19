using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Hypar.Elements;
using Hypar.Geometry;

namespace Testfit
{
    [StructLayout(LayoutKind.Sequential)]
    struct v2f
    {
        public float x;
        public float y;
    }

    public class MeshElement : Proxy
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

    public class Building
    {
        public List<Polyline> Polylines {get;}

        [DllImport("libtfmp.so")]
        static extern int tfmp_generate_bldg(v2f[] boundary, uint n,
                                             float mass_width,
                                             float mass_height,
                                             float aspect_ratio,
                                             out IntPtr poly_verts,
                                             out uint num_verts,
                                             out IntPtr poly_lengths,
                                             out uint num_lengths);

        [DllImport("libtfmp.so")]
        static extern int tfmp_free(IntPtr poly_verts,
                                    IntPtr poly_lengths);

        Building() {}
        Building(List<Polyline> polylines)
        {
            Polylines = polylines;
        }

        public static Building Make(Vector3[] boundary, float width, float height, float aspect)
        {
            v2f[] boundary2 = new v2f[boundary.Length];
            for (int i = 0; i < boundary.Length; ++i)
            {
                boundary2[i].x = (float)boundary[i].X;
                boundary2[i].y = (float)boundary[i].Y;
            }
            IntPtr poly_verts;
            IntPtr poly_lengths;
            uint num_verts;
            uint num_lengths;
            int ret;

            ret = tfmp_generate_bldg(boundary2, (uint)boundary2.Length,
                                     width, height, aspect,
                                     out poly_verts, out num_verts,
                                     out poly_lengths, out num_lengths);

            if (ret != 0) {
                Console.WriteLine("tfmp_generate_bldg() failed");
                return new Building();
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
            return new Building(polylines);
        }
    }
}
