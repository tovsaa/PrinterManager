#include <windows.h>
#include <string>
#include <cstdlib>


void convertWordToPDF(const std::string& inputPath, const std::string& outputPath) {
    std::string psCommand =
        "powershell -Command \""
        "$word = New-Object -ComObject Word.Application; "
        "$word.Visible = $false; "
        "$doc = $word.Documents.Open('" + inputPath + "'); "
                      "$doc.SaveAs('" + outputPath + "', 17); "
                       "$doc.Close(); $word.Quit();"
                       "\"";

    system(psCommand.c_str());
}
