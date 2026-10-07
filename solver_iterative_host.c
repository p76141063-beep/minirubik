/* Host reference and correctness checks; target search uses no recursion. */
#define main baseline_main
#include "solver.c"
#undef main
#include <time.h>

static uint16_t pt[3][PERMUTATIONS], ot[3][ORIENTATIONS];
static uint8_t pd[PERMUTATIONS], od[ORIENTATIONS], path[11];
static uint64_t candidates, entered;

typedef struct {
    uint16_t p, o, np, no;
    uint8_t previous, face, turn;
} frame_t;
static frame_t frames[11];

static void abstract_bfs(uint8_t *dist, unsigned count,
                         uint16_t table[3][count])
{
    uint16_t queue[PERMUTATIONS];
    unsigned head=0, tail=1;
    memset(dist,255,count); dist[0]=0; queue[0]=0;
    while(head<tail) {
        unsigned here=queue[head++];
        for(unsigned f=0;f<3;f++) {
            unsigned next=here;
            for(unsigned t=0;t<3;t++) {
                next=table[f][next];
                if(dist[next]==255) {
                    dist[next]=(uint8_t)(dist[here]+1);
                    queue[tail++]=(uint16_t)next;
                }
            }
        }
    }
    if(tail!=count) { fputs("Abstract BFS incomplete\n",stderr); exit(1); }
}
static void initialize(void)
{
    state_t s;
    for(unsigned p=0;p<PERMUTATIONS;p++) {
        unrank_state(p*ORIENTATIONS,&s);
        for(unsigned f=0;f<3;f++) {
            state_t n=quarter_turn(s,(uint8_t)f);
            pt[f][p]=(uint16_t)(rank_state(&n)/ORIENTATIONS);
        }
    }
    for(unsigned o=0;o<ORIENTATIONS;o++) {
        unrank_state(o,&s);
        for(unsigned f=0;f<3;f++) {
            state_t n=quarter_turn(s,(uint8_t)f);
            ot[f][o]=(uint16_t)(rank_state(&n)%ORIENTATIONS);
        }
    }
    abstract_bfs(pd,PERMUTATIONS,pt);
    abstract_bfs(od,ORIENTATIONS,ot);
}
static unsigned h(unsigned p,unsigned o)
{ return pd[p]>od[o]?pd[p]:od[o]; }
static int solve(unsigned p,unsigned o)
{
    candidates=entered=0;
    if(!p&&!o) return 0;
    for(unsigned bound=h(p,o);bound<=11;bound++) {
        unsigned depth=0;
        frames[0]=(frame_t){p,o,p,o,3,0,0}; entered++;
        for(;;) {
            frame_t *f=&frames[depth];
            if(f->face>=3) {
                if(!depth) break;
                depth--; continue;
            }
            if(f->face==f->previous || f->turn==3) {
                f->face++; f->turn=0; f->np=f->p; f->no=f->o;
                continue;
            }
            unsigned face=f->face,turn=f->turn++;
            f->np=pt[face][f->np]; f->no=ot[face][f->no];
            candidates++;
            unsigned next_depth=depth+1;
            if(next_depth+h(f->np,f->no)>bound) continue;
            path[depth]=(uint8_t)(face*3+turn);
            if(!f->np&&!f->no) return (int)next_depth;
            if(next_depth==bound) continue;
            unsigned np=f->np,no=f->no;
            depth=next_depth;
            frames[depth]=(frame_t){np,no,np,no,face,0,0};
            entered++;
        }
    }
    return -1;
}
static int replay(state_t s,int length)
{
    if(length<0) return 0;
    for(int i=0;i<length;i++) s=apply_move(s,path[i]);
    return rank_state(&s)==0;
}
static uint8_t *exact_distances(void)
{
    uint8_t *dist=malloc(STATES);
    uint32_t *q=malloc((size_t)STATES*sizeof *q);
    if(!dist||!q) { free(dist);free(q);return NULL; }
    memset(dist,255,STATES);dist[0]=0;q[0]=0;
    unsigned head=0,tail=1;
    while(head<tail) {
        unsigned here=q[head++],p=here/ORIENTATIONS,o=here%ORIENTATIONS;
        for(unsigned f=0;f<3;f++) {
            unsigned np=p,no=o;
            for(unsigned t=0;t<3;t++) {
                np=pt[f][np];no=ot[f][no];unsigned next=np*ORIENTATIONS+no;
                if(dist[next]==255) {dist[next]=dist[here]+1;q[tail++]=next;}
            }
        }
    }
    free(q);
    if(tail!=STATES) {free(dist);return NULL;}
    return dist;
}
int main(int argc,char **argv)
{
    initialize();
    if(argc==2&&(!strcmp(argv[1],"--gates")||!strcmp(argv[1],"--all"))) {
        clock_t start=clock();
        uint8_t *exact=exact_distances(); if(!exact)return 1;
        unsigned maxp=0,maxo=0,diameter=0,count11=0;
        for(unsigned p=0;p<PERMUTATIONS;p++) {
            if(pd[p]==255)return 1;
            if(pd[p]>maxp)maxp=pd[p];
            for(unsigned f=0;f<3;f++)if(pt[f][p]>=PERMUTATIONS)return 1;
        }
        for(unsigned o=0;o<ORIENTATIONS;o++) {
            if(od[o]==255)return 1;
            if(od[o]>maxo)maxo=od[o];
            for(unsigned f=0;f<3;f++)if(ot[f][o]>=ORIENTATIONS)return 1;
        }
        if(pd[0]||od[0])return 1;
        for(unsigned r=0;r<STATES;r++) {
            if(exact[r]==255||h(r/ORIENTATIONS,r%ORIENTATIONS)>exact[r])return 1;
            if(exact[r]>diameter)diameter=exact[r];
            count11+=exact[r]==11;
        }
        printf("H1 passed: %u states\n",STATES);
        printf("H2 passed: P entries=%u max=%u solved=%u; O entries=%u max=%u solved=%u\n",PERMUTATIONS,maxp,pd[0],ORIENTATIONS,maxo,od[0]);
        printf("Exact BFS: diameter=%u distance11=%u\n",diameter,count11);
        printf("H4: not applicable (unpacked tables)\n");fflush(stdout);
        if(!strcmp(argv[1],"--all")) {
            for(unsigned r=0;r<STATES;r++) {
                int length=solve(r/ORIENTATIONS,r%ORIENTATIONS);
                state_t s;unrank_state(r,&s);
                if(length!=exact[r]||!replay(s,length)) {
                    fprintf(stderr,"H3 failed at rank %u\n",r);free(exact);return 1;
                }
                if(r%10000==0){printf("H3 progress: %u/%u\n",r,STATES);fflush(stdout);}
            }
            puts("H3 passed: entire domain");
        }
        printf("CPU seconds: %.3f\n",(double)(clock()-start)/CLOCKS_PER_SEC);
        free(exact);return 0;
    }
    state_t s;
    if(argc!=2||!parse_state(argv[1],&s)) {fputs("usage: solver_iterative_host STATE | --gates | --all\n",stderr);return 2;}
    unsigned rank=rank_state(&s);
    int length=solve(rank/ORIENTATIONS,rank%ORIENTATIONS);
    printf("Shortest length: %d\n",length);
    if(length<0||!replay(s,length))return 1;
    for(int i=0;i<length;i++)printf("%s%s",i?" ":"",move_names[path[i]]);
    printf("\nSolution verified: yes\nChild candidates: %llu\nEntered frames: %llu\n",(unsigned long long)candidates,(unsigned long long)entered);
    return 0;
}
