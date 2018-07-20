using Amazon.Lambda.Core;
using Hypar.Elements;
using Hypar.GeoJSON;
using Hypar.Geometry;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using Testfit;

[assembly: LambdaSerializer(typeof(Amazon.Lambda.Serialization.Json.JsonSerializer))]
namespace Hypar
{
    public class Function
    {
        public Dictionary<string,object> Handler(Dictionary<string,object> input, ILambdaContext context)
        {
            var width = float.Parse(input["width"].ToString());
            var height = float.Parse(input["height"].ToString());
            var aspect_ratio = float.Parse(input["aspectRatio"].ToString());
            var features = ((JArray)input["boundary"]).ToObject<Feature[]>();
            var outline = (Polygon)features[0].Geometry;
            var origin = outline.Coordinates[0][0];
            var offset = origin.ToVectorMeters();
            var plines = outline.ToPolylines();
            var pline = plines[0];
            var boundary = new Polyline(pline.Vertices.Select(v=>new Vector3(v.X - offset.X, v.Y - offset.Y, v.Z)));

            var building = Building.Make(boundary.ToArray(), width, height, aspect_ratio);

            var model = new Model();
            model.Origin = origin;
            var mesh = new MeshElement(Mesh.Extrude(building.Polylines, height));
            model.AddElement(mesh);
            return model.ToHypar();
        }
    }
};
