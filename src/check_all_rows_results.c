/*
 * tau-sync sieve
 * Author: Chisora
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 4096

static int split_csv_simple(char *line, char **cols, int max_cols){
    int n = 0;
    char *p = line;
    while(n < max_cols){
        cols[n++] = p;
        char *c = strchr(p, ',');
        if(!c) break;
        *c = '\0';
        p = c + 1;
    }
    return n;
}

int main(int argc, char **argv){
    const char *infile = argc >= 2 ? argv[1] : "m8388608_all_rows_comparison.csv";
    unsigned long long expected_rows = argc >= 3 ? strtoull(argv[2], NULL, 10) : 8388608ULL;
    const char *outfile = argc >= 4 ? argv[3] : "all_rows_check_summary.csv";
    FILE *fp = fopen(infile, "rb");
    if(!fp){
        fprintf(stderr, "ERROR: cannot open input CSV: %s\n", infile);
        return 2;
    }
    FILE *out = fopen(outfile, "wb");
    if(!out){
        fclose(fp);
        fprintf(stderr, "ERROR: cannot open output CSV: %s\n", outfile);
        return 3;
    }

    char buf[BUF_SIZE];
    if(!fgets(buf, sizeof(buf), fp)){
        fprintf(stderr, "ERROR: empty CSV\n");
        fprintf(out, "metric,value\nerror,empty_csv\n");
        fclose(fp); fclose(out);
        return 4;
    }

    // Expected columns:
    // row_id,scope_id,tau_label_id,x,tau_class,standard_class,class_match,twin_cert,twin_direct,twin_match
    unsigned long long rows = 0;
    unsigned long long class_mismatch = 0;
    unsigned long long twin_mismatch = 0;
    unsigned long long class_match_ones = 0;
    unsigned long long twin_match_ones = 0;
    unsigned long long prime = 0, semiprime = 0, almost_prime = 0, unknown = 0;
    unsigned long long twin_true = 0, twin_false = 0, twin_unknown = 0;

    while(fgets(buf, sizeof(buf), fp)){
        size_t len = strlen(buf);
        while(len && (buf[len-1]=='\n' || buf[len-1]=='\r')) buf[--len] = '\0';
        if(len == 0) continue;
        char *cols[16];
        int n = split_csv_simple(buf, cols, 16);
        if(n < 10){
            fprintf(stderr, "ERROR: malformed row near row %llu\n", rows + 1);
            continue;
        }
        rows++;
        const char *tau_class = cols[4];
        int cm = atoi(cols[6]);
        const char *twc = cols[7];
        int tm = atoi(cols[9]);

        if(cm == 1) class_match_ones++; else class_mismatch++;
        if(tm == 1) twin_match_ones++; else twin_mismatch++;

        if(strcmp(tau_class, "prime") == 0) prime++;
        else if(strcmp(tau_class, "semiprime") == 0) semiprime++;
        else if(strcmp(tau_class, "almost_prime") == 0) almost_prime++;
        else unknown++;

        if(strcmp(twc, "1") == 0 || strcmp(twc, "true") == 0 || strcmp(twc, "TWIN") == 0) twin_true++;
        else if(strcmp(twc, "0") == 0 || strcmp(twc, "false") == 0 || strcmp(twc, "NON_TWIN") == 0) twin_false++;
        else twin_unknown++;
    }

    fprintf(out, "metric,value\n");
    fprintf(out, "rows,%llu\n", rows);
    fprintf(out, "expected_rows,%llu\n", expected_rows);
    fprintf(out, "rows_ok,%d\n", rows == expected_rows ? 1 : 0);
    fprintf(out, "class_match,%llu\n", class_match_ones);
    fprintf(out, "class_mismatch,%llu\n", class_mismatch);
    fprintf(out, "twin_match,%llu\n", twin_match_ones);
    fprintf(out, "twin_mismatch,%llu\n", twin_mismatch);
    fprintf(out, "prime,%llu\n", prime);
    fprintf(out, "semiprime,%llu\n", semiprime);
    fprintf(out, "almost_prime,%llu\n", almost_prime);
    fprintf(out, "unknown_class,%llu\n", unknown);
    fprintf(out, "twin_true,%llu\n", twin_true);
    fprintf(out, "twin_false,%llu\n", twin_false);
    fprintf(out, "twin_unknown,%llu\n", twin_unknown);
    fprintf(out, "ok,%d\n", (rows == expected_rows && class_mismatch == 0 && twin_mismatch == 0) ? 1 : 0);

    fclose(fp);
    fclose(out);

    printf("rows=%llu expected=%llu rows_ok=%d\n", rows, expected_rows, rows == expected_rows ? 1 : 0);
    printf("class_mismatch=%llu twin_mismatch=%llu\n", class_mismatch, twin_mismatch);
    printf("summary=%s\n", outfile);
    return (rows == expected_rows && class_mismatch == 0 && twin_mismatch == 0) ? 0 : 1;
}
