using Hypar;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using Xunit;

namespace test
{
    public class FunctionTest
    {
        private Dictionary<string,object> _data;

        public FunctionTest()
        {
            var str = File.ReadAllText("sample.json");
            _data = JsonConvert.DeserializeObject<Dictionary<string,object>>(str);
        }

        [Fact]
        public void Test()
        {
            var func = new Function();
            var result = func.Handler(_data, null);

            File.WriteAllText("sample.glb", result["model"].ToString());

            var json = JsonConvert.SerializeObject(result);
            Console.WriteLine(json);
        }
    }
}
