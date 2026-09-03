#include "cJSON.h"

#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct lconv g_fake_lconv;
static char g_comma_decimal[] = ",";

struct lconv *xwau_test_localeconv(void)
{
    memset(&g_fake_lconv, 0, sizeof g_fake_lconv);
    g_fake_lconv.decimal_point = g_comma_decimal;
    return &g_fake_lconv;
}

double xwau_test_strtod(const char *text, char **end)
{
    if (strncmp(text, "0,25", 4) == 0) {
        if (end) *end = (char *)text + 4;
        return 0.25;
    }

    if (strncmp(text, "0.25", 4) == 0) {
        if (end) *end = (char *)text + 1;
        return 0.0;
    }

    if (end) *end = (char *)text;
    return 0.0;
}

int main(int argc, char **argv)
{
    if (argc != 2 ||
        (strcmp(argv[1], "accept") != 0 && strcmp(argv[1], "reject") != 0)) {
        fprintf(stderr, "usage: %s accept|reject\n", argv[0]);
        return 2;
    }

    const int expect_accept = strcmp(argv[1], "accept") == 0;
    const char *json = "{\"value\":0.25}";
    cJSON *root = cJSON_ParseWithOpts(json, NULL, 1);

    if (!expect_accept) {
        if (root) {
            cJSON_Delete(root);
            fprintf(stderr, "FAIL: parser unexpectedly accepted dot decimal without locale adaptation\n");
            return 1;
        }
        puts("PASS: deterministic comma-decimal stub reproduces the pre-fix rejection");
        return 0;
    }

    if (!root) {
        fprintf(stderr, "FAIL: locale-aware parser rejected standard JSON dot decimal\n");
        return 1;
    }

    const cJSON *value = cJSON_GetObjectItemCaseSensitive(root, "value");
    if (!cJSON_IsNumber(value) || fabs(value->valuedouble - 0.25) > 1.0e-12) {
        cJSON_Delete(root);
        fprintf(stderr, "FAIL: locale-aware parser returned the wrong numeric value\n");
        return 1;
    }

    cJSON_Delete(root);
    puts("PASS: locale-aware parser accepts 0.25 under deterministic comma-decimal CRT semantics");
    return 0;
}
