using System;
using System.Runtime.InteropServices;

namespace dotnet_test
{
    class Program
    {
        [DllImport("libtest.so")]
        public static extern void show_message();

        static void Main(string[] args)
        {
            show_message();
        }
    }
}
