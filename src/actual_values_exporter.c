/*
 * tau-sync sieve
 * Author: Chisora
 * SPDX-License-Identifier: MIT
 */

/* actual_values_exporter.c
   ASCII only. Windows MinGW64 / cmd.exe.
   Purpose:
     Export actual x values for prime / semiprime / almost_prime.
     Compare tau-only certificate class with standard direct class.
     No Python required.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#if defined(_WIN32)
#include <windows.h>
static double now_sec(void){ LARGE_INTEGER f,c; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&c); return (double)c.QuadPart/(double)f.QuadPart; }
#else
#include <time.h>
static double now_sec(void){ return (double)clock()/CLOCKS_PER_SEC; }
#endif

typedef unsigned long long u64;
typedef __uint128_t u128;

enum { CLS_UNKNOWN=0, CLS_PRIME=1, CLS_SEMI=2, CLS_ALMOST=3, CLS_CONFLICT=9 };

typedef struct { u64 key; unsigned char cls; unsigned char scope; unsigned char tau_label; u64 count; unsigned char used; } Entry;
typedef struct { Entry *tab; size_t cap; size_t size; } Map;

static u64 hash64(u64 x){
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    x ^= x >> 31; return x;
}
static int map_init(Map *m, size_t pow2){
    m->cap=1ULL<<pow2; m->size=0;
    m->tab=(Entry*)calloc(m->cap,sizeof(Entry));
    return m->tab!=NULL;
}
static void map_free(Map *m){ free(m->tab); m->tab=NULL; m->cap=m->size=0; }
static void map_put_merge(Map *m, u64 key, unsigned char cls, unsigned char scope, unsigned char tau_label){
    size_t mask=m->cap-1, i=(size_t)hash64(key)&mask;
    for(;;){
        Entry *e=&m->tab[i];
        if(!e->used){
            e->used=1; e->key=key; e->cls=cls; e->scope=scope; e->tau_label=tau_label; e->count=1; m->size++; return;
        }
        if(e->key==key){
            e->count++;
            if(e->cls!=cls) e->cls=CLS_CONFLICT;
            return;
        }
        i=(i+1)&mask;
    }
}

static u64 modmul(u64 a,u64 b,u64 m){ return (u64)(((u128)a*b)%m); }
static u64 modpow(u64 a,u64 d,u64 m){
    u64 r=1;
    while(d){ if(d&1) r=modmul(r,a,m); a=modmul(a,a,m); d>>=1; }
    return r;
}
static int is_prime64(u64 n){
    static const unsigned small[] = {2,3,5,7,11,13,17,19,23,29,31,37,0};
    if(n<2) return 0;
    for(int i=0; small[i]; ++i){ unsigned p=small[i]; if(n%p==0) return n==p; }
    u64 d=n-1; int s=0; while((d&1)==0){ d>>=1; s++; }
    static const u64 bases[] = {2ULL,325ULL,9375ULL,28178ULL,450775ULL,9780504ULL,1795265022ULL,0};
    for(int i=0; bases[i]; ++i){
        u64 a=bases[i]%n; if(a==0) continue;
        u64 x=modpow(a,d,n);
        if(x==1 || x==n-1) continue;
        int ok=0;
        for(int r=1;r<s;r++){ x=modmul(x,x,n); if(x==n-1){ ok=1; break; } }
        if(!ok) return 0;
    }
    return 1;
}
static u64 icbrt_u64(u64 n){
    u64 lo=0, hi=3000000ULL;
    while((u128)hi*hi*hi <= n && hi < (1ULL<<32)) hi*=2;
    while(lo+1<hi){ u64 mid=lo+(hi-lo)/2; if((u128)mid*mid*mid<=n) lo=mid; else hi=mid; }
    return lo;
}
static unsigned char standard_classify(u64 n){
    if(is_prime64(n)) return CLS_PRIME;
    u64 lim=icbrt_u64(n);
    if(n%2==0){ u64 m=n/2; return is_prime64(m)?CLS_SEMI:CLS_ALMOST; }
    for(u64 p=3;p<=lim;p+=2){
        if(n%p==0){ u64 m=n/p; return is_prime64(m)?CLS_SEMI:CLS_ALMOST; }
    }
    return CLS_SEMI;
}
static unsigned char parse_cls(const char *s){
    if(strcmp(s,"prime")==0) return CLS_PRIME;
    if(strcmp(s,"semiprime")==0) return CLS_SEMI;
    if(strcmp(s,"almost_prime")==0) return CLS_ALMOST;
    return CLS_UNKNOWN;
}
static const char *class_name(unsigned char c){
    if(c==CLS_PRIME) return "prime";
    if(c==CLS_SEMI) return "semiprime";
    if(c==CLS_ALMOST) return "almost_prime";
    if(c==CLS_CONFLICT) return "conflict";
    return "unknown";
}
static const char *scope_name(unsigned char s){
    if(s==1) return "source15";
    if(s==2) return "clone55";
    if(s==3) return "next15";
    return "unknown";
}
static unsigned char parse_scope_id(const char *s){
    if(strcmp(s,"source15")==0) return 1;
    if(strcmp(s,"clone55")==0) return 2;
    if(strcmp(s,"next15")==0) return 3;
    return 0;
}
static unsigned char parse_tau_label_id(const char *s){
    if(strcmp(s,"prime")==0) return 1;
    if(strcmp(s,"semiprime")==0) return 2;
    if(strcmp(s,"almost_prime")==0) return 3;
    return 0;
}
static int split_csv(char *line, char **fields, int maxf){
    int n=0; char *p=line; fields[n++]=p;
    while(*p && n<maxf){
        if(*p==','){ *p=0; fields[n++]=p+1; }
        else if(*p=='\r' || *p=='\n'){ *p=0; break; }
        p++;
    }
    return n;
}
static int load_tau_rows(const char *path, Map *map, u64 *row_count){
    FILE *f=fopen(path,"rb"); if(!f){ fprintf(stderr,"cannot open %s\n",path); return 0; }
    char line[4096]; if(!fgets(line,sizeof(line),f)){ fclose(f); return 0; }
    u64 rows=0;
    while(fgets(line,sizeof(line),f)){
        char *fld[8]; int nf=split_csv(line,fld,8); if(nf<4) continue;
        u64 x=strtoull(fld[2],NULL,10); unsigned char c=parse_cls(fld[3]); if(!c) continue;
        map_put_merge(map,x,c,parse_scope_id(fld[0]),parse_tau_label_id(fld[1]));
        rows++;
    }
    fclose(f); *row_count=rows; return 1;
}
static void write_header(FILE *f){
    fprintf(f,"x,tau_only_class,standard_direct_class,match,occurrence_count,example_scope,twin_direct_if_prime\n");
}
int main(int argc, char **argv){
    const char *in="tau_only_classification_rows.csv";
    const char *prefix="m8388608_actual_values";
    if(argc>1) in=argv[1];
    if(argc>2) prefix=argv[2];
    Map map; if(!map_init(&map,20)){ fprintf(stderr,"map init failed\n"); return 2; }
    u64 source_rows=0; if(!load_tau_rows(in,&map,&source_rows)){ return 2; }
    char fn_all[256], fn_p[256], fn_s[256], fn_a[256], fn_sum[256];
    snprintf(fn_all,sizeof(fn_all),"%s_unique_all.csv",prefix);
    snprintf(fn_p,sizeof(fn_p),"%s_prime_values.csv",prefix);
    snprintf(fn_s,sizeof(fn_s),"%s_semiprime_values.csv",prefix);
    snprintf(fn_a,sizeof(fn_a),"%s_almost_prime_values.csv",prefix);
    snprintf(fn_sum,sizeof(fn_sum),"%s_summary.csv",prefix);
    FILE *fa=fopen(fn_all,"wb"), *fp=fopen(fn_p,"wb"), *fs=fopen(fn_s,"wb"), *fl=fopen(fn_a,"wb");
    if(!fa||!fp||!fs||!fl){ fprintf(stderr,"cannot write output csv\n"); return 2; }
    write_header(fa); write_header(fp); write_header(fs); write_header(fl);
    u64 cnt_tau[10]={0}, cnt_std[10]={0}, mismatch=0, twin_true=0, twin_checked=0;
    double st=now_sec();
    for(size_t i=0;i<map.cap;i++){
        Entry *e=&map.tab[i]; if(!e->used) continue;
        unsigned char std=standard_classify(e->key);
        int match=(std==e->cls);
        if(!match) mismatch++;
        if(e->cls<10) cnt_tau[e->cls]++;
        if(std<10) cnt_std[std]++;
        int twin=-1;
        if(e->cls==CLS_PRIME){ twin=is_prime64(e->key+2ULL); twin_checked++; if(twin) twin_true++; }
        FILE *dst = (e->cls==CLS_PRIME)?fp:((e->cls==CLS_SEMI)?fs:((e->cls==CLS_ALMOST)?fl:NULL));
        fprintf(fa,"%llu,%s,%s,%d,%llu,%s,%d\n",(unsigned long long)e->key,class_name(e->cls),class_name(std),match,(unsigned long long)e->count,scope_name(e->scope),twin);
        if(dst) fprintf(dst,"%llu,%s,%s,%d,%llu,%s,%d\n",(unsigned long long)e->key,class_name(e->cls),class_name(std),match,(unsigned long long)e->count,scope_name(e->scope),twin);
        if((i & 65535ULL)==0){ fprintf(stderr,"\rexport scanned bucket=%llu/%llu mismatch=%llu     ",(unsigned long long)i,(unsigned long long)map.cap,(unsigned long long)mismatch); fflush(stderr); }
    }
    fprintf(stderr,"\n"); fclose(fa); fclose(fp); fclose(fs); fclose(fl);
    double sec=now_sec()-st;
    FILE *sm=fopen(fn_sum,"wb");
    if(sm){
        fprintf(sm,"section,metric,value\n");
        fprintf(sm,"input,source_rows,%llu\n",(unsigned long long)source_rows);
        fprintf(sm,"unique,all,%llu\n",(unsigned long long)map.size);
        fprintf(sm,"tau,prime,%llu\n",(unsigned long long)cnt_tau[CLS_PRIME]);
        fprintf(sm,"tau,semiprime,%llu\n",(unsigned long long)cnt_tau[CLS_SEMI]);
        fprintf(sm,"tau,almost_prime,%llu\n",(unsigned long long)cnt_tau[CLS_ALMOST]);
        fprintf(sm,"standard,prime,%llu\n",(unsigned long long)cnt_std[CLS_PRIME]);
        fprintf(sm,"standard,semiprime,%llu\n",(unsigned long long)cnt_std[CLS_SEMI]);
        fprintf(sm,"standard,almost_prime,%llu\n",(unsigned long long)cnt_std[CLS_ALMOST]);
        fprintf(sm,"check,mismatch,%llu\n",(unsigned long long)mismatch);
        fprintf(sm,"twin,prime_x_checked,%llu\n",(unsigned long long)twin_checked);
        fprintf(sm,"twin,twin_true,%llu\n",(unsigned long long)twin_true);
        fprintf(sm,"timing,seconds,%.9f\n",sec);
        fclose(sm);
    }
    printf("actual values exported unique=%llu mismatch=%llu seconds=%.6f\n",(unsigned long long)map.size,(unsigned long long)mismatch,sec);
    map_free(&map);
    return mismatch?1:0;
}
