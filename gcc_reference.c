/* Freestanding RV32I GCC reference; same nonrecursive search algorithm. */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
extern const uint16_t permutation[3][5040], orientation[3][729];
extern const uint8_t perm_distance[5040], orient_distance[729];
#define pt permutation
#define ot orientation
#define pd perm_distance
#define od orient_distance
static uint8_t path[11];
typedef struct {
    uint16_t p,o,np,no;
    uint8_t previous,face,turn;
} frame_t;
static frame_t frames[11];
static const char input_state[] = "21345671111111";
static void print_int(int x) {
    register int a0 __asm__("a0")=x;
    register int a7 __asm__("a7")=1;
    __asm__ volatile("ecall" : "+r"(a0), "+r"(a7) : : "memory");
}
static void print_char(int x) {
    register int a0 __asm__("a0")=x;
    register int a7 __asm__("a7")=11;
    __asm__ volatile("ecall" : "+r"(a0), "+r"(a7) : : "memory");
}
static void print_string(const char *x) {
    register const char *a0 __asm__("a0")=x;
    register int a7 __asm__("a7")=4;
    __asm__ volatile("ecall" : "+r"(a0), "+r"(a7) : : "memory");
}
static unsigned h(unsigned p,unsigned o)
{ return pd[p]>od[o]?pd[p]:od[o]; }
static int solve(unsigned p,unsigned o)
{
    if(!p&&!o) return 0;
    for(unsigned bound=h(p,o);bound<=11;bound++) {
        unsigned depth=0;
        frames[0]=(frame_t){p,o,p,o,3,0,0};
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
            unsigned next_depth=depth+1;
            if(next_depth+h(f->np,f->no)>bound) continue;
            path[depth]=(uint8_t)(face*3+turn);
            if(!f->np&&!f->no) return (int)next_depth;
            if(next_depth==bound) continue;
            unsigned np=f->np,no=f->no;
            depth=next_depth;
            frames[depth]=(frame_t){np,no,np,no,face,0,0};
           
        }
    }
    return -1;
}

int reference_main(void) {
    unsigned p=0,o=0,seen=0,sum=0;
    for(unsigned i=0;i<7;i++) {
        unsigned digit=(unsigned)(input_state[i]-'1');
        if(digit>=7 || (seen & (1u<<digit))) {print_int(-2);return 2;}
        seen|=1u<<digit;
        unsigned smaller=0;
        for(unsigned j=i+1;j<7;j++) smaller+=input_state[j]<input_state[i];
        volatile unsigned product=0;
        for(unsigned k=0;k<7-i;k++) product+=p;
        p=product+smaller;
    }
    for(unsigned i=0;i<7;i++) {
        unsigned digit=(unsigned)(input_state[7+i]-'1');
        if(digit>=3){print_int(-2);return 2;}
        sum+=digit;
        if(i<6)o=(o<<1)+o+digit;
    }
    while(sum>=3)sum-=3;
    if(sum || input_state[14]){print_int(-2);return 2;}
    int length=solve(p,o);
    if(length<0){print_int(-1);return 1;}
    unsigned vp=p,vo=o;
    for(int i=0;i<length;i++) {
        unsigned face=0,turn=path[i];
        while(turn>=3){turn-=3;face++;}
        for(unsigned k=0;k<=turn;k++){vp=pt[face][vp];vo=ot[face][vo];}
    }
    if(vp || vo){print_int(-3);return 3;}
    print_int(length);print_char(10);
    for(int i=0;i<length;i++){print_int(path[i]);print_char(32);}
    print_char(10);print_string("Solution verified: yes\n");
    return 0;
}
