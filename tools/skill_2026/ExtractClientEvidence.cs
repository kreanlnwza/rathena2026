using System;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using GRF.Core;
using Utilities.Services;

internal static class ExtractClientEvidence {
    private static readonly string[] EntryPaths = {
        @"data\luafiles514\lua files\skillinfoz\skillid.lub",
        @"data\luafiles514\lua files\skillinfoz\skillinfolist.lub",
        @"data\luafiles514\lua files\skillinfoz\skilldescript.lub",
        @"data\luafiles514\lua files\skillinfoz\skilldelaylist.lub",
        @"data\luafiles514\lua files\skillinfoz\skilltreeview.lub",
        @"data\luafiles514\lua files\skillinfoz\skillinfo_f.lub",
        @"data\luafiles514\lua files\skillinfoz\jobinheritlist.lub",
        @"data\luafiles514\lua files\stateicon\efstids.lub",
        @"data\luafiles514\lua files\stateicon\stateiconinfo.lub",
        @"data\luafiles514\lua files\stateicon\stateiconimginfo.lub"
    };

    private static int Main(string[] args) {
        if (args.Length != 2) {
            Console.Error.WriteLine("Usage: ExtractClientEvidence ARCHIVE OUTPUT_DIRECTORY");
            return 2;
        }

        string archivePath = Path.GetFullPath(args[0]);
        string outputPath = Path.GetFullPath(args[1]);
        EncodingService.DisplayEncoding = Encoding.GetEncoding(949);

        using (var archive = new GrfHolder()) {
            archive.Open(archivePath);
            if (archive.Header.GpakKeyDiscoveryFailed)
                throw new InvalidDataException("GPak key discovery failed: " + archive.Header.GpakKeyDiscoveryStatus);

            Directory.CreateDirectory(outputPath);
            Console.WriteLine("GPAK_STATUS\t" + archive.Header.GpakKeyDiscoveryStatus);
            foreach (string entryPath in EntryPaths) {
                FileEntry entry = archive.FileTable.Entries.FirstOrDefault(item => item.RelativePath == entryPath);
                if (entry == null) {
                    Console.WriteLine("MISSING\t" + entryPath);
                    continue;
                }

                byte[] bytes = entry.GetDecompressedData();
                string destination = Path.Combine(outputPath, entryPath);
                Directory.CreateDirectory(Path.GetDirectoryName(destination));
                File.WriteAllBytes(destination, bytes);
                Console.WriteLine("EXTRACTED\t" + bytes.LongLength + "\t" + Sha256(bytes) + "\t" + entryPath);
            }
        }
        return 0;
    }

    private static string Sha256(byte[] bytes) {
        using (SHA256 hash = SHA256.Create())
            return BitConverter.ToString(hash.ComputeHash(bytes)).Replace("-", "").ToLowerInvariant();
    }
}
