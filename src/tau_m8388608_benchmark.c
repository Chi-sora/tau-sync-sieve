/*
 * tau-sync sieve
 * Author: Chisora
 * SPDX-License-Identifier: MIT
 */

/* tau_m8388608_benchmark.c
   ASCII only. Windows MinGW64 / cmd.exe.
   Purpose:
     Compare tau-only certificate classification with standard direct checks.
     Add TWIN companion certificate mode.
     M default = 8388608.
   Notes:
     tau-only classification mode does not call is_prime.
     twin certificate mode does not call is_prime.
     standard validation uses deterministic Miller-Rabin + cbrt trial division.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
static double now_sec(void){ LARGE_INTEGER f,c; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&c); return (double)c.QuadPart/(double)f.QuadPart; }
#else
static double now_sec(void){ return (double)clock()/CLOCKS_PER_SEC; }
#endif

typedef unsigned long long u64;
typedef __uint128_t u128;

enum { CLS_UNKNOWN=0, CLS_PRIME=1, CLS_SEMI=2, CLS_ALMOST=3, CLS_CONFLICT=9 };

typedef struct { u64 x; unsigned char cls; unsigned char scope; unsigned char tau_label; } XRow;

typedef struct { u64 key; unsigned char val; unsigned char used; } MapEntry;

typedef struct { MapEntry *tab; size_t cap; size_t size; } HashMap;

static u64 hash64(u64 x){
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    x ^= x >> 31; return x;
}
static int map_init(HashMap *m, size_t cap_pow2){
    m->cap = 1ULL << cap_pow2; m->size=0;
    m->tab = (MapEntry*)calloc(m->cap, sizeof(MapEntry));
    return m->tab!=NULL;
}
static void map_free(HashMap *m){ free(m->tab); m->tab=NULL; m->cap=m->size=0; }
static unsigned char map_get(HashMap *m, u64 key){
    size_t mask=m->cap-1, i=(size_t)hash64(key)&mask;
    for(;;){
        MapEntry *e=&m->tab[i];
        if(!e->used) return 0;
        if(e->key==key) return e->val;
        i=(i+1)&mask;
    }
}
static void map_put_merge(HashMap *m, u64 key, unsigned char val){
    size_t mask=m->cap-1, i=(size_t)hash64(key)&mask;
    for(;;){
        MapEntry *e=&m->tab[i];
        if(!e->used){ e->used=1; e->key=key; e->val=val; m->size++; return; }
        if(e->key==key){ if(e->val!=val) e->val=CLS_CONFLICT; return; }
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
    long double xd=(long double)n;
    u64 lo=0, hi=(u64)(powl(xd, 1.0L/3.0L))+4;
    while((u128)hi*hi*hi <= n) hi*=2;
    while(lo+1<hi){ u64 mid=lo+(hi-lo)/2; if((u128)mid*mid*mid <= n) lo=mid; else hi=mid; }
    return lo;
}

static unsigned char standard_classify(u64 n){
    if(is_prime64(n)) return CLS_PRIME;
    u64 lim=icbrt_u64(n);
    if(lim<2) return CLS_SEMI;
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

static int load_tau_rows(const char *path, XRow **rows_out, size_t *n_out, HashMap *map){
    FILE *f=fopen(path,"rb"); if(!f){ fprintf(stderr,"cannot open %s\n",path); return 0; }
    size_t cap=262144,n=0; XRow *rows=(XRow*)malloc(cap*sizeof(XRow)); if(!rows){ fclose(f); return 0; }
    char line[4096]; fgets(line,sizeof(line),f);
    while(fgets(line,sizeof(line),f)){
        char *fld[8]; int nf=split_csv(line,fld,8); if(nf<4) continue;
        u64 x=strtoull(fld[2],NULL,10); unsigned char c=parse_cls(fld[3]); if(!c) continue;
        if(n==cap){ cap*=2; rows=(XRow*)realloc(rows,cap*sizeof(XRow)); if(!rows){ fclose(f); return 0; } }
        rows[n].x=x; rows[n].cls=c; rows[n].scope=parse_scope_id(fld[0]); rows[n].tau_label=parse_tau_label_id(fld[1]); n++;
        map_put_merge(map,x,c);
    }
    fclose(f); *rows_out=rows; *n_out=n; return 1;
}


static int load_twin_cert(const char *path, HashMap *twin_map){
    FILE *f=fopen(path,"rb");
    if(!f){ fprintf(stderr,"cannot open twin cert %s\n",path); return 0; }
    char line[4096];
    if(!fgets(line,sizeof(line),f)){ fclose(f); return 0; }
    while(fgets(line,sizeof(line),f)){
        char *fld[8]; int nf=split_csv(line,fld,8); if(nf<4) continue;
        u64 x=strtoull(fld[0],NULL,10);
        int cert=atoi(fld[2]);
        map_put_merge(twin_map,x,(unsigned char)(cert?1:2));
    }
    fclose(f);
    return 1;
}

static void run_twin_certificate(HashMap *class_map, HashMap *twin_map, const char *outcsv, int validate_direct){
    u64 prime_count=0, covered=0, unknown=0, twin_true=0, non_twin=0;
    u64 direct_true=0, fail=0;
    double st=now_sec();
    for(size_t i=0;i<class_map->cap;i++){
        MapEntry *e=&class_map->tab[i];
        if(!e->used || e->val!=CLS_PRIME) continue;
        prime_count++;
        unsigned char c=map_get(twin_map,e->key);
        if(c==0){ unknown++; continue; }
        covered++;
        int tau_twin=(c==1);
        if(tau_twin) twin_true++; else non_twin++;
        if(validate_direct){
            int dt=is_prime64(e->key+2ULL);
            if(dt) direct_true++;
            if(dt!=tau_twin) fail++;
        }
    }
    double sec=now_sec()-st;
    FILE *o=fopen(outcsv,"a");
    fprintf(o,"twin_certificate,prime_x,%llu,covered,%llu,unknown,%llu,twin_true,%llu,non_twin,%llu,seconds,%.9f\n",
        (unsigned long long)prime_count,(unsigned long long)covered,(unsigned long long)unknown,
        (unsigned long long)twin_true,(unsigned long long)non_twin,sec);
    if(validate_direct){
        fprintf(o,"twin_certificate_validation,covered,%llu,direct_true,%llu,fail,%llu,seconds,%.9f\n",
            (unsigned long long)covered,(unsigned long long)direct_true,(unsigned long long)fail,sec);
    }
    fclose(o);
    printf("twin_cert covered=%llu unknown=%llu twin=%llu fail=%llu seconds=%.6f\n",
        (unsigned long long)covered,(unsigned long long)unknown,(unsigned long long)twin_true,(unsigned long long)fail,sec);
}

static void run_tau_only_classification(XRow *rows, size_t n, u64 M, const char *outcsv){
    double st=now_sec();
    u64 cnt[4]={0,0,0,0};
    for(u64 i=0;i<M;i++){ unsigned char c=rows[i % n].cls; if(c>=1 && c<=3) cnt[c]++; }
    double sec=now_sec()-st;
    FILE *o=fopen(outcsv,"a");
    fprintf(o,"classification,tau_only_M,%llu,seconds,%.9f,rows_per_sec,%.3f\n",(unsigned long long)M,sec,(double)M/sec);
    fprintf(o,"classification,tau_only_M,%llu,prime,%llu,semiprime,%llu,almost_prime,%llu\n",(unsigned long long)M,(unsigned long long)cnt[1],(unsigned long long)cnt[2],(unsigned long long)cnt[3]);
    fclose(o);
    printf("tau_only M=%llu seconds=%.6f rows_per_sec=%.3f\n",(unsigned long long)M,sec,(double)M/sec);
}

static void run_standard_unique(HashMap *map, const char *outcsv, int limit){
    u64 checked=0, fail=0, cnt[4]={0,0,0,0};
    double st=now_sec();
    for(size_t i=0;i<map->cap;i++){
        MapEntry *e=&map->tab[i]; if(!e->used || e->val==CLS_CONFLICT) continue;
        if(limit>0 && (int)checked>=limit) break;
        unsigned char d=standard_classify(e->key);
        checked++; if(e->val>=1 && e->val<=3) cnt[e->val]++;
        if(d!=e->val) fail++;
        if((checked & 4095ULL)==0){ fprintf(stderr,"\rstandard checked=%llu fail=%llu      ",(unsigned long long)checked,(unsigned long long)fail); fflush(stderr); }
    }
    fprintf(stderr,"\n");
    double sec=now_sec()-st;
    FILE *o=fopen(outcsv,"a");
    fprintf(o,"classification,standard_unique,%llu,seconds,%.9f,fail,%llu\n",(unsigned long long)checked,sec,(unsigned long long)fail);
    fprintf(o,"classification,standard_unique,%llu,prime,%llu,semiprime,%llu,almost_prime,%llu\n",(unsigned long long)checked,(unsigned long long)cnt[1],(unsigned long long)cnt[2],(unsigned long long)cnt[3]);
    fclose(o);
    printf("standard unique checked=%llu fail=%llu seconds=%.6f\n",(unsigned long long)checked,(unsigned long long)fail,sec);
}

static void run_twin(HashMap *map, const char *outcsv, int direct_all){
    u64 prime_count=0, covered=0, unknown=0, tau_true=0, direct_true=0, fail=0;
    double st=now_sec();
    for(size_t i=0;i<map->cap;i++){
        MapEntry *e=&map->tab[i]; if(!e->used || e->val!=CLS_PRIME) continue;
        prime_count++;
        unsigned char c2=map_get(map,e->key+2);
        if(!c2 || c2==CLS_CONFLICT){ unknown++; continue; }
        covered++;
        int tt=(c2==CLS_PRIME);
        int dt=is_prime64(e->key+2); /* direct comparison for covered. e->key prime is certificate. */
        tau_true += tt; direct_true += dt;
        if(tt!=dt) fail++;
    }
    double sec=now_sec()-st;
    FILE *o=fopen(outcsv,"a");
    fprintf(o,"twin,covered,%llu,unknown,%llu,tau_true,%llu,direct_true,%llu,fail,%llu,seconds,%.9f\n",(unsigned long long)covered,(unsigned long long)unknown,(unsigned long long)tau_true,(unsigned long long)direct_true,(unsigned long long)fail,sec);
    if(direct_all){
        u64 all_true=0; double st2=now_sec();
        for(size_t i=0;i<map->cap;i++){ MapEntry *e=&map->tab[i]; if(e->used && e->val==CLS_PRIME) all_true += is_prime64(e->key+2); }
        double sec2=now_sec()-st2;
        fprintf(o,"twin,direct_all_prime_x,%llu,prime_x,%llu,seconds,%.9f\n",(unsigned long long)all_true,(unsigned long long)prime_count,sec2);
    }
    fclose(o);
    printf("twin covered=%llu unknown=%llu fail=%llu seconds=%.6f\n",(unsigned long long)covered,(unsigned long long)unknown,(unsigned long long)fail,sec);
}

static void run_goldbach(const char *events, const char *outcsv, int limit){
    FILE *f=fopen(events,"rb"); if(!f){ fprintf(stderr,"cannot open %s\n",events); return; }
    char line[8192]; fgets(line,sizeof(line),f);
    u64 checked=0, fail=0; double st=now_sec();
    while(fgets(line,sizeof(line),f)){
        char *fld[32]; int nf=split_csv(line,fld,32); if(nf<20) continue;
        u64 N55=strtoull(fld[4],NULL,10); int t55=atoi(fld[17]);
        if(t55>0){ u64 q=6ULL*(u64)t55-1ULL; u64 x=N55-q; checked++; if(q+x!=N55 || !is_prime64(q) || !is_prime64(x)) fail++; }
        u64 N15=strtoull(fld[5],NULL,10); int j15=atoi(fld[18]);
        if(j15>0){ u64 q=6ULL*(u64)j15+1ULL; u64 x=N15-q; checked++; if(q+x!=N15 || !is_prime64(q) || !is_prime64(x)) fail++; }
        if(limit>0 && (int)checked>=limit) break;
        if((checked & 8191ULL)==0){ fprintf(stderr,"\rgoldbach checked=%llu fail=%llu      ",(unsigned long long)checked,(unsigned long long)fail); fflush(stderr); }
    }
    fprintf(stderr,"\n"); fclose(f); double sec=now_sec()-st;
    FILE *o=fopen(outcsv,"a");
    fprintf(o,"goldbach,endpoint_checks,%llu,fail,%llu,seconds,%.9f\n",(unsigned long long)checked,(unsigned long long)fail,sec);
    fclose(o);
    printf("goldbach checked=%llu fail=%llu seconds=%.6f\n",(unsigned long long)checked,(unsigned long long)fail,sec);
}


static void build_standard_class_map(HashMap *tau_map, HashMap *direct_map){
    for(size_t i=0;i<tau_map->cap;i++){
        MapEntry *e=&tau_map->tab[i];
        if(!e->used || e->val==CLS_CONFLICT) continue;
        unsigned char d=standard_classify(e->key);
        map_put_merge(direct_map,e->key,d);
    }
}
static void build_direct_twin_map(HashMap *tau_map, HashMap *direct_twin_map){
    for(size_t i=0;i<tau_map->cap;i++){
        MapEntry *e=&tau_map->tab[i];
        if(!e->used || e->val!=CLS_PRIME) continue;
        int twin=is_prime64(e->key+2ULL);
        map_put_merge(direct_twin_map,e->key,(unsigned char)(twin?1:2));
    }
}
static void run_all_rows_comparison(XRow *rows, size_t n, HashMap *tau_map, const char *twin_cert_csv, u64 M, const char *outcsv, const char *summary_csv){
    HashMap direct_map, twin_cert_map, direct_twin_map;
    if(!map_init(&direct_map,20)){ fprintf(stderr,"direct map init failed\n"); return; }
    if(!map_init(&twin_cert_map,18)){ fprintf(stderr,"twin cert map init failed\n"); map_free(&direct_map); return; }
    if(!map_init(&direct_twin_map,18)){ fprintf(stderr,"direct twin map init failed\n"); map_free(&direct_map); map_free(&twin_cert_map); return; }
    double st_build=now_sec();
    build_standard_class_map(tau_map,&direct_map);
    build_direct_twin_map(tau_map,&direct_twin_map);
    int twin_loaded=load_twin_cert(twin_cert_csv,&twin_cert_map);
    double build_sec=now_sec()-st_build;
    if(!twin_loaded){ fprintf(stderr,"warning: twin cert not loaded; twin_cert columns will be unknown\n"); }

    FILE *o=fopen(outcsv,"wb");
    if(!o){ fprintf(stderr,"cannot write %s\n",outcsv); goto cleanup; }
    static char obuf[1<<20]; setvbuf(o,obuf,_IOFBF,sizeof(obuf));
    fprintf(o,"row_id,scope_id,tau_label_id,x,tau_class,standard_class,class_match,twin_cert,twin_direct,twin_match\n");
    u64 class_fail=0,twin_fail=0,twin_unknown=0,twin_cert_true=0,twin_direct_true=0;
    u64 cnt_tau[4]={0,0,0,0}, cnt_std[4]={0,0,0,0};
    double st=now_sec();
    for(u64 i=0;i<M;i++){
        XRow r=rows[i % n];
        unsigned char std=map_get(&direct_map,r.x);
        int class_match=(std==r.cls);
        if(!class_match) class_fail++;
        if(r.cls>=1 && r.cls<=3) cnt_tau[r.cls]++;
        if(std>=1 && std<=3) cnt_std[std]++;
        int twin_cert=0, twin_direct=0, twin_match=1;
        if(r.cls==CLS_PRIME){
            unsigned char tc=map_get(&twin_cert_map,r.x);
            unsigned char td=map_get(&direct_twin_map,r.x);
            twin_cert = (tc==1)?1:(tc==2?0:9);
            twin_direct = (td==1)?1:(td==2?0:9);
            if(twin_cert==9 || twin_direct==9){ twin_unknown++; twin_match=0; }
            else { twin_match=(twin_cert==twin_direct); if(!twin_match) twin_fail++; }
            if(twin_cert==1) twin_cert_true++;
            if(twin_direct==1) twin_direct_true++;
        }else{
            twin_cert=-1; twin_direct=-1; twin_match=1;
        }
        fprintf(o,"%llu,%u,%u,%llu,%s,%s,%d,%d,%d,%d\n",
            (unsigned long long)i,(unsigned)r.scope,(unsigned)r.tau_label,(unsigned long long)r.x,
            class_name(r.cls),class_name(std),class_match,twin_cert,twin_direct,twin_match);
        if((i & 0x3FFFFULL)==0){ fprintf(stderr,"\rallrows %llu/%llu class_fail=%llu twin_fail=%llu     ",
            (unsigned long long)i,(unsigned long long)M,(unsigned long long)class_fail,(unsigned long long)twin_fail); fflush(stderr); }
    }
    fprintf(stderr,"\n");
    fclose(o);
    double sec=now_sec()-st;
    FILE *s=fopen(summary_csv,"wb");
    if(s){
        fprintf(s,"section,metric,value\n");
        fprintf(s,"all_rows,M,%llu\n",(unsigned long long)M);
        fprintf(s,"all_rows,build_seconds,%.9f\n",build_sec);
        fprintf(s,"all_rows,direct_class_unique,%llu\n",(unsigned long long)direct_map.size);
        fprintf(s,"all_rows,direct_twin_unique,%llu\n",(unsigned long long)direct_twin_map.size);
        fprintf(s,"all_rows,twin_cert_unique,%llu\n",(unsigned long long)twin_cert_map.size);
        fprintf(s,"all_rows,write_seconds,%.9f\n",sec);
        fprintf(s,"all_rows,rows_per_sec,%.3f\n",(double)M/sec);
        fprintf(s,"classification,class_fail,%llu\n",(unsigned long long)class_fail);
        fprintf(s,"classification,tau_prime,%llu\n",(unsigned long long)cnt_tau[CLS_PRIME]);
        fprintf(s,"classification,tau_semiprime,%llu\n",(unsigned long long)cnt_tau[CLS_SEMI]);
        fprintf(s,"classification,tau_almost_prime,%llu\n",(unsigned long long)cnt_tau[CLS_ALMOST]);
        fprintf(s,"classification,std_prime,%llu\n",(unsigned long long)cnt_std[CLS_PRIME]);
        fprintf(s,"classification,std_semiprime,%llu\n",(unsigned long long)cnt_std[CLS_SEMI]);
        fprintf(s,"classification,std_almost_prime,%llu\n",(unsigned long long)cnt_std[CLS_ALMOST]);
        fprintf(s,"twin,twin_fail,%llu\n",(unsigned long long)twin_fail);
        fprintf(s,"twin,twin_unknown,%llu\n",(unsigned long long)twin_unknown);
        fprintf(s,"twin,twin_cert_true,%llu\n",(unsigned long long)twin_cert_true);
        fprintf(s,"twin,twin_direct_true,%llu\n",(unsigned long long)twin_direct_true);
        fclose(s);
    }
    printf("all rows written=%s M=%llu class_fail=%llu twin_fail=%llu seconds=%.6f\n",outcsv,(unsigned long long)M,(unsigned long long)class_fail,(unsigned long long)twin_fail,sec);
cleanup:
    map_free(&direct_map); map_free(&twin_cert_map); map_free(&direct_twin_map);
}

int main(int argc, char **argv){
    const char *tau_csv="tau_only_classification_rows.csv";
    const char *events_csv="lapse_clone_full_events.csv";
    const char *outcsv="m8388608_tau_benchmark_results.csv";
    u64 M=8388608ULL; int standard_limit=0, goldbach_limit=0, direct_all_twin=1; int skip_standard=0, skip_goldbach=0, skip_twin=0, skip_classification=0;
    int use_twin_cert=0, validate_twin_cert=0; int all_rows=0; const char *all_rows_csv="m8388608_all_rows_comparison.csv"; const char *all_rows_summary="m8388608_all_rows_summary.csv"; const char *twin_cert_csv="twin_companion_certificate.csv";
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i],"--M")==0 && i+1<argc) M=strtoull(argv[++i],NULL,10);
        else if(strcmp(argv[i],"--tau-csv")==0 && i+1<argc) tau_csv=argv[++i];
        else if(strcmp(argv[i],"--events-csv")==0 && i+1<argc) events_csv=argv[++i];
        else if(strcmp(argv[i],"--out")==0 && i+1<argc) outcsv=argv[++i];
        else if(strcmp(argv[i],"--standard-limit")==0 && i+1<argc) standard_limit=atoi(argv[++i]);
        else if(strcmp(argv[i],"--goldbach-limit")==0 && i+1<argc) goldbach_limit=atoi(argv[++i]);
        else if(strcmp(argv[i],"--skip-standard")==0) skip_standard=1;
        else if(strcmp(argv[i],"--skip-goldbach")==0) skip_goldbach=1;
        else if(strcmp(argv[i],"--skip-twin")==0) skip_twin=1;
        else if(strcmp(argv[i],"--skip-classification")==0) skip_classification=1;
        else if(strcmp(argv[i],"--twin-cert")==0 && i+1<argc){ twin_cert_csv=argv[++i]; use_twin_cert=1; }
        else if(strcmp(argv[i],"--validate-twin-cert")==0){ validate_twin_cert=1; }
        else if(strcmp(argv[i],"--all-rows")==0){ all_rows=1; }
        else if(strcmp(argv[i],"--all-rows-out")==0 && i+1<argc){ all_rows_csv=argv[++i]; }
        else if(strcmp(argv[i],"--all-rows-summary")==0 && i+1<argc){ all_rows_summary=argv[++i]; }
    }
    FILE *o=fopen(outcsv,"w"); if(!o){ fprintf(stderr,"cannot write %s\n",outcsv); return 2; }
    fprintf(o,"section,metric1,value1,metric2,value2,metric3,value3,metric4,value4\n"); fclose(o);
    HashMap map; if(!map_init(&map,20)){ fprintf(stderr,"map init failed\n"); return 2; }
    XRow *rows=NULL; size_t n=0;
    if(!load_tau_rows(tau_csv,&rows,&n,&map)){ fprintf(stderr,"load failed\n"); return 2; }
    printf("loaded tau rows=%llu unique_map=%llu\n",(unsigned long long)n,(unsigned long long)map.size);
    if(all_rows){ printf("strict all-rows mode: M=%llu; building standard direct maps first\n",(unsigned long long)M); run_all_rows_comparison(rows,n,&map,twin_cert_csv,M,all_rows_csv,all_rows_summary); free(rows); map_free(&map); return 0; }
    if(!skip_classification) run_tau_only_classification(rows,n,M,outcsv);
    if(!skip_standard) run_standard_unique(&map,outcsv,standard_limit);
    if(!skip_goldbach) run_goldbach(events_csv,outcsv,goldbach_limit);
    if(!skip_twin){
        if(use_twin_cert){
            HashMap twin_map;
            if(!map_init(&twin_map,18)){ fprintf(stderr,"twin map init failed\n"); return 2; }
            if(!load_twin_cert(twin_cert_csv,&twin_map)){ map_free(&twin_map); return 2; }
            run_twin_certificate(&map,&twin_map,outcsv,validate_twin_cert);
            map_free(&twin_map);
        }else{
            run_twin(&map,outcsv,direct_all_twin);
        }
    }
    free(rows); map_free(&map);
    printf("done output=%s\n",outcsv);
    return 0;
}
