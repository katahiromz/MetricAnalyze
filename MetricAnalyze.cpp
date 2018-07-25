// This file is public domain software (PDS).
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <windows.h>
#include <cstdio>
#include <cassert>
#include <vector>
#include <string>

struct NAME_AND_FILE
{
    const char *name;
    const char *file;
};

NAME_AND_FILE g_pairs[] =
{
    { "FreeMono", "FreeMono.ttf" },
    { "FreeSans", "FreeSans.ttf" },
    { "FreeSerif", "FreeSerif.ttf" },
    { "DejaVu Serif", "DejaVuSerif.ttf" },
    { "DejaVu Sans", "DejaVuSans.ttf" },
    { "DejaVu Sans Mono", "DejaVuSansMono.ttf" },
    { "Ubuntu Mono", "UbuntuMono-R.ttf" },
    { "Liberation Sans", "LiberationSans-Regular.ttf" },
    { "Liberation Serif", "LiberationSerif-Regular.ttf" },
    { "Liberation Mono", "LiberationMono-Regular.ttf" },
    { "Libre Franklin", "LibreFranklin-Regular.ttf" },
};
size_t g_pair_count = sizeof(g_pairs) / sizeof(g_pairs[0]);

HDC g_hDC;
FT_Library g_library;

BOOL TestWin(const char *font_name, LONG lfHeight, TEXTMETRIC& tm)
{
    LOGFONTA lf;

    ZeroMemory(&lf, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = lfHeight;
    lstrcpyA(lf.lfFaceName, font_name);
    HFONT hFont = CreateFontIndirectA(&lf);

    HGDIOBJ hFontOld = SelectObject(g_hDC, hFont);
    {
        CHAR szFace[64];
        GetTextFaceA(g_hDC, 64, szFace);
        if (lstrcmpiA(szFace, font_name) != 0)
        {
            fprintf(stderr, "%s and %s: mismatched\n", font_name, szFace);
            return FALSE;
        }
        GetTextMetrics(g_hDC, &tm);
    }
    SelectObject(g_hDC, hFontOld);
    DeleteObject(hFont);
    return TRUE;
}

FT_Face TestFT(const char *font_file)
{
    FT_Face face = NULL;
    FT_New_Face(g_library, font_file, 0, &face);
    return face;
}

template <typename T_STR_CONTAINER>
inline void
mstr_split(T_STR_CONTAINER& container,
           const typename T_STR_CONTAINER::value_type& str,
           const typename T_STR_CONTAINER::value_type& chars)
{
    container.clear();
    size_t i = 0, k = str.find_first_of(chars);
    while (k != T_STR_CONTAINER::value_type::npos)
    {
        container.push_back(str.substr(i, k - i));
        i = k + 1;
        k = str.find_first_of(chars, i);
    }
    container.push_back(str.substr(i));
}

template <typename T_STR_CONTAINER>
inline typename T_STR_CONTAINER::value_type
mstr_join(const T_STR_CONTAINER& container,
          const typename T_STR_CONTAINER::value_type& sep)
{
    typename T_STR_CONTAINER::value_type result;
    typename T_STR_CONTAINER::const_iterator it, end;
    it = container.begin();
    end = container.end();
    if (it != end)
    {
        result = *it;
        for (++it; it != end; ++it)
        {
            result += sep;
            result += *it;
        }
    }
    return result;
}

int main(void)
{
    const char *text = "This is a sample text.";

    g_hDC = CreateCompatibleDC(NULL);
    FT_Init_FreeType(&g_library);

    INT nTotalScore = 0;
    TEXTMETRIC tm;

    FILE *fp = fopen("MetricAnalyze.txt", "w");

    const char *field_names[] =
    {
        "Font Name",
        // LOGFONT
        "lfHeight",
        // TEXTMETRIC
        "tmHeight", "tmAscent", "tmDescent", "tmInternalLeading", "tmExternalLeading",
        // FT_Face
        "units_per_EM", "ascender", "descender", "height",
        // TT_OS2
        "version", "xAvgCharWidth", "sTypoAscender", "sTypoDescender", "sTypoLineGap", "usWinAscent", "usWinDescent", "sxHeight", "sCapHeight",
        // TT_HoriHeader
        "Version", "Ascender", "Descender", "Line_Gap", "advance_Width_Max"
    };

    std::vector<std::string> fields;
    for (size_t m = 0; m < sizeof(field_names) / sizeof(field_names[0]); ++m)
    {
        fields.push_back(field_names[m]);
    }

    std::string str = mstr_join(fields, "\t");
    fprintf(fp, "%s\n", str.c_str());

    for (size_t k = 0; k < g_pair_count; ++k)
    {
        char szPath[MAX_PATH];
        GetWindowsDirectoryA(szPath, MAX_PATH);
        lstrcatA(szPath, "\\Fonts\\");
        lstrcatA(szPath, g_pairs[k].file);
        if (GetFileAttributesA(szPath) == 0xFFFFFFFF)
        {
            printf("%s: skipped\n", g_pairs[k].file);
            continue;
        }

        static const LONG aHeights[] = { 100, 1000, 10000, -100, -1000, -10000 };

        for (size_t i = 0; i < sizeof(aHeights) / sizeof(aHeights[0]); ++i)
        {
            LONG lfHeight = aHeights[i];
            if (TestWin(g_pairs[k].name, lfHeight, tm))
            {
                if (FT_Face face = TestFT(szPath))
                {
                    TT_OS2 *pOS2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);
                    TT_HoriHeader *pHori = (TT_HoriHeader *)FT_Get_Sfnt_Table(face, FT_SFNT_HHEA);

                    std::vector<std::string> an1 =
                    {
                        std::to_string(lfHeight),
                        std::to_string(tm.tmHeight),
                        std::to_string(tm.tmAscent),
                        std::to_string(tm.tmDescent),
                        std::to_string(tm.tmInternalLeading),
                        std::to_string(tm.tmExternalLeading),
                        std::to_string(face->units_per_EM),
                        std::to_string(face->ascender),
                        std::to_string(face->descender),
                        std::to_string(face->height),
                    };

                    fprintf(fp, "%s\t", g_pairs[k].name);

                    str = mstr_join(an1, "\t");
                    fprintf(fp, "%s\t", str.c_str());

                    if (pOS2)
                    {
                        std::vector<std::string> an2 =
                        {
                            std::to_string(pOS2->version),
                            std::to_string(pOS2->xAvgCharWidth),
                            std::to_string(pOS2->sTypoAscender),
                            std::to_string(pOS2->sTypoDescender),
                            std::to_string(pOS2->sTypoLineGap),
                            std::to_string(pOS2->usWinAscent),
                            std::to_string(pOS2->usWinDescent),
                            std::to_string(pOS2->sxHeight),
                            std::to_string(pOS2->sCapHeight),
                        };
                        str = mstr_join(an2, "\t");
                        fprintf(fp, "%s\t", str.c_str());
                    }
                    else
                    {
                        fprintf(fp, "-\t-\t-\t-\t-\t-\t-\t-\t");
                    }

                    if (pHori)
                    {
                        std::vector<std::string> an3 =
                        {
                            std::to_string(pHori->Version),
                            std::to_string(pHori->Ascender),
                            std::to_string(pHori->Descender),
                            std::to_string(pHori->Line_Gap),
                            std::to_string(pHori->advance_Width_Max),
                        };
                        str = mstr_join(an3, "\t");
                        fprintf(fp, "%s\t", str.c_str());
                    }
                    else
                    {
                        fprintf(fp, "-\t-\t-\t-\t-");
                    }

                    fprintf(fp, "\n");

                    FT_Done_Face(face);
                }
            }
        }
    }

    fclose(fp);

    DeleteDC(g_hDC);
    FT_Done_FreeType(g_library);

    return 0;
}
