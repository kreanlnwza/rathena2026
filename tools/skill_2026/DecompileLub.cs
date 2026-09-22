using System;
using System.IO;
using System.Text;
using GRF.FileFormats.LubFormat;
using Utilities.Services;

internal static class DecompileLub {
    private static int Main(string[] args) {
        if (args.Length != 1 || !Directory.Exists(args[0])) {
            Console.Error.WriteLine("Usage: DecompileLub EXTRACTED_DIRECTORY");
            return 2;
        }

        EncodingService.DisplayEncoding = Encoding.GetEncoding(949);
        foreach (string path in Directory.EnumerateFiles(args[0], "*.lub", SearchOption.AllDirectories)) {
            byte[] bytes = File.ReadAllBytes(path);
            if (!Lub.IsCompiled(bytes)) continue;
            string destination = path + ".lua";
            File.WriteAllText(destination, new Lub(bytes).Decompile(), new UTF8Encoding(false));
            Console.WriteLine("DECOMPILED\t" + destination);
        }
        return 0;
    }
}
