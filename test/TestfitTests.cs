using Hypar.Elements;
using Hypar.Geometry;
using System;
using System.Collections.Generic;
using System.IO;
using Testfit;
using Xunit;

namespace test
{
    public class TestfitTest
    {
        [Fact]
        public void Test()
        {
            Vector3[] boundary = {
                new Vector3(0.0f,   0.0f),
                new Vector3(200.0f, 0.0f),
                new Vector3(200.0f, 200.0f),
                new Vector3(0.0f,   200.0f),
            };
            var height = 40.0f;
            var building = Building.Make(boundary, 60.0f, height, 0.5f);
            for (int i = 0; i < building.Polylines.Count; ++i)
                Console.WriteLine(building.Polylines[i].ToString());

            var model = new Model();
            var mesh = new MeshElement(Mesh.Extrude(building.Polylines, height));
            model.AddElement(mesh);
            model.SaveGlb("test.glb");
        }
    }
}
