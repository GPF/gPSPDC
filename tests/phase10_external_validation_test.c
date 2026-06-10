#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static char *read_text_file(const char *path)
{
  FILE *fp = fopen(path, "rb");
  long size;
  char *buffer;

  if(fp == NULL)
  {
    printf("failed to open %s\n", path);
    failures++;
    return NULL;
  }

  if(fseek(fp, 0, SEEK_END) != 0 || (size = ftell(fp)) < 0 ||
   fseek(fp, 0, SEEK_SET) != 0)
  {
    printf("failed to size %s\n", path);
    fclose(fp);
    failures++;
    return NULL;
  }

  buffer = (char *)malloc((size_t)size + 1);
  if(buffer == NULL)
  {
    printf("failed to allocate %s\n", path);
    fclose(fp);
    failures++;
    return NULL;
  }

  if(fread(buffer, 1, (size_t)size, fp) != (size_t)size)
  {
    printf("failed to read %s\n", path);
    free(buffer);
    fclose(fp);
    failures++;
    return NULL;
  }

  buffer[size] = '\0';
  fclose(fp);
  return buffer;
}

static void expect_contains(const char *name, const char *text,
 const char *needle)
{
  if(text == NULL)
    return;

  if(strstr(text, needle) == NULL)
  {
    printf("%s failed: missing `%s`\n", name, needle);
    failures++;
  }
}

static void test_external_validation_doc_contract(void)
{
  char *doc = read_text_file("../EXTERNAL_VALIDATION.md");

  expect_contains("external validation P0 gba-tests", doc,
   "jsmolka/gba-tests");
  expect_contains("external validation P0 singlestep", doc,
   "SingleStepTests/ARM7TDMI");
  expect_contains("external validation miniz", doc, "richgel999/miniz");
  expect_contains("external validation stb", doc, "nothings/stb");
  expect_contains("external validation notices", doc,
   "THIRD_PARTY_NOTICES.md");
  expect_contains("external validation no generated ROMs", doc,
   "generated `.gba` test binaries");

  if(doc != NULL)
    free(doc);

  printf("external validation doc contract: ok\n");
}

static void test_fetch_script_contract(void)
{
  char *script = read_text_file("../scripts/fetch-external-validation.sh");
  char *gitignore = read_text_file("../.gitignore");
  char *gitattributes = read_text_file("../.gitattributes");
  char *readme = read_text_file("../README.md");
  char *doc = read_text_file("../EXTERNAL_VALIDATION.md");
  char *lock = read_text_file("../EXTERNAL_VALIDATION_LOCK.md");

  expect_contains("fetch doc portable invocation", doc,
   "sh scripts/fetch-external-validation.sh");
  expect_contains("external validation lock doc", doc,
   "EXTERNAL_VALIDATION_LOCK.md");
  expect_contains("external validation lock miniz", lock,
   "2ea4e81e1593c48112ef7fdf1da5562704acfdd2");
  expect_contains("external validation lock gba tests", lock,
   "a7113b67e63f83a9b321696ddd7042ccfad6c881");
  expect_contains("fetch script uses ignored external dir", script,
   "external/gba-validation");
  expect_contains("fetch script dry run", script, "DRY_RUN");
  expect_contains("fetch script dry run message", script, "Would fetch");
  expect_contains("fetch script gba-tests", script,
   "https://github.com/jsmolka/gba-tests.git");
  expect_contains("fetch script singlestep", script,
   "https://github.com/SingleStepTests/ARM7TDMI.git");
  expect_contains("fetch script miniz", script,
   "https://github.com/richgel999/miniz.git");
  expect_contains("fetch script updates existing clones", script,
   "git -C \"$dest\" fetch --depth=1 origin");
  expect_contains("gitignore external", gitignore, "external/");
  expect_contains("gitattributes shell LF", gitattributes,
   "*.sh text eol=lf");
  expect_contains("README external validation", readme,
   "EXTERNAL_VALIDATION.md");

  if(script != NULL)
    free(script);
  if(gitignore != NULL)
    free(gitignore);
  if(gitattributes != NULL)
    free(gitattributes);
  if(readme != NULL)
    free(readme);
  if(doc != NULL)
    free(doc);
  if(lock != NULL)
    free(lock);

  printf("external validation fetch contract: ok\n");
}

static void test_miniz_integration_contract(void)
{
  char *zip_c = read_text_file("../zip.c");
  char *readme = read_text_file("../README.md");
  char *doc = read_text_file("../EXTERNAL_VALIDATION.md");
  char *notices = read_text_file("../THIRD_PARTY_NOTICES.md");
  char *export_h = read_text_file("../third_party/miniz/miniz_export.h");
  char *license = read_text_file("../third_party/miniz/LICENSE");
  char *dc_makefile = read_text_file("../dc/Makefile");
  char *x86_makefile = read_text_file("../x86/Makefile");
  char *gp2x_makefile = read_text_file("../gp2x/Makefile");
  char *psp_makefile = read_text_file("../psp/Makefile");

  expect_contains("miniz zip switch", zip_c, "GPSP_USE_MINIZ");
  expect_contains("miniz no stdio", zip_c, "MINIZ_NO_STDIO");
  expect_contains("miniz included into zip", zip_c,
   "third_party/miniz/miniz.c");
  expect_contains("README miniz build flag", readme, "USE_MINIZ=1");
  expect_contains("external validation miniz vendored", doc,
   "third_party/miniz/");
  expect_contains("third party miniz notice", notices, "## miniz");
  expect_contains("third party miniz commit", notices, "2ea4e81");
  expect_contains("miniz export shim", export_h, "#define MINIZ_EXPORT");
  expect_contains("miniz license retained", license,
   "Copyright 2010-2014 Rich Geldreich");
  expect_contains("dc miniz make flag", dc_makefile, "USE_MINIZ");
  expect_contains("dc miniz drops zlib", dc_makefile, "KOS_ZIP_LIBS =");
  expect_contains("x86 miniz make flag", x86_makefile, "USE_MINIZ");
  expect_contains("gp2x miniz make flag", gp2x_makefile, "USE_MINIZ");
  expect_contains("psp miniz make flag", psp_makefile, "USE_MINIZ");
  expect_contains("dc zip explicit rom path", zip_c,
   "snprintf(open_path, sizeof(open_path), \"/cd/gbaDC/%s\", filename)");

  if(zip_c != NULL)
    free(zip_c);
  if(readme != NULL)
    free(readme);
  if(doc != NULL)
    free(doc);
  if(notices != NULL)
    free(notices);
  if(export_h != NULL)
    free(export_h);
  if(license != NULL)
    free(license);
  if(dc_makefile != NULL)
    free(dc_makefile);
  if(x86_makefile != NULL)
    free(x86_makefile);
  if(gp2x_makefile != NULL)
    free(gp2x_makefile);
  if(psp_makefile != NULL)
    free(psp_makefile);

  printf("miniz integration contract: ok\n");
}

int main(void)
{
  failures = 0;

  test_external_validation_doc_contract();
  test_fetch_script_contract();
  test_miniz_integration_contract();

  if(failures != 0)
  {
    printf("phase10_external_validation_test: %d failure(s)\n", failures);
    return 1;
  }

  return 0;
}
