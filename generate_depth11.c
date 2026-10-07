#define main baseline_main
#include "solver.c"
#undef main

int main(void)
{
    uint8_t diameter;
    uint8_t *table = build_table(&diameter);
    if (!table)
        return 1;

    FILE *out = fopen("depth11.csv", "w");
    if (!out) {
        free(table);
        return 1;
    }

    fprintf(out, "P,O\n");
    unsigned count = 0;

    for (uint32_t rank = 0; rank < STATES; ++rank) {
        state_t state;
        unrank_state(rank, &state);

        uint32_t here = rank;
        unsigned depth = 0;

        while (here != 0) {
            state = apply_move(state, table[here]);
            here = rank_state(&state);
            ++depth;
        }

        if (depth == 11) {
            fprintf(out, "%u,%u\n",
                    (unsigned)(rank / ORIENTATIONS),
                    (unsigned)(rank % ORIENTATIONS));
            ++count;
        }
    }

    int failed = ferror(out);
    if (fclose(out) != 0)
        failed = 1;
    free(table);

    printf("Diameter: %u\nDepth-11 states: %u\n",
           diameter, count);

    return failed || diameter != 11 || count != 2644;
}