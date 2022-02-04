#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
bool constitute_circle(int base, int loser); // helper function for lock_pairs
void print_winner(void);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    // Print candidates
    printf("Candidates: ");

    for (int i = 0; i <  candidate_count; i++)
    {
        printf("%s ", candidates[i]);
    }

    printf("\n");

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    // Check if 'name' exist inside the candidates array
    for (int i = 0; i < candidate_count; i++)
    {
        // The specification assume that no two candidates have the same name, so we can ignore checking for dupiclates.
        if(strcmp(name, candidates[i]) == 0) {
            // Set rank
            // 'i' here is the index of the candidate in the candidates array
            ranks[rank] = i;
            return true;
        }
    }
    // candidate doesn't exist
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    // The specification assume that every voter will rank each of the candidates
    for (int i = 0; i < candidate_count; i++)
    {
        // Ex: int ranks[] = {2, 0, 1}
        // Loop 1 => preferences[2][0] += 1, preferences[2][1] += 1
        // Loop 2 => preferences[0][1] += 1
        for (int j = i + 1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]] += 1;
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    // Loop over preferences[]
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            // Skip if tied, ex: if preferences[1][2] == preferences[2][1]
            if (!(preferences[i][j] == preferences[j][i]))
            {
                // update the pairs[]
                if (preferences[i][j] > preferences[j][i])
                {
                    pairs[pair_count].winner = i;
                    pairs[pair_count].loser = j;
                }
                else
                {
                    pairs[pair_count].winner = j;
                    pairs[pair_count].loser = i;
                }
                pair_count++;
            }
        }
    }
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    // Sort from the largest strength of victory
    // Let's use selection sort for now
    int index;

    for (int i = 0; i < pair_count; i++)
    {
        index = i;
        for (int j = i + 1; j < pair_count; j++)
        {
            // Ex: preferences[0][1] = 7, preferences[1][0] = 2
            // strength_of_victory = 7 - 2
            int strength_of_victory = preferences[pairs[index].winner][pairs[index].loser] - preferences[pairs[index].loser][pairs[index].winner];
            int current_high = preferences[pairs[j].winner][pairs[j].loser] - preferences[pairs[j].loser][pairs[j].winner];
            if (current_high > strength_of_victory)
            {
                index = j;
            }
        }
        pair temp = pairs[i];
        pairs[i] = pairs[index];
        pairs[index] = temp;
    }
    return;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    // edge = arrow = true
    // ex: locked[0][1] = true, means that 0 is pointing to 1
    for (int i = 0; i < pair_count; i++)
    {
        // don't add an edge if it will make the graph turn circle
        if(constitute_circle(pairs[i].winner, pairs[i].loser))
        {
            continue;
        }
        else
        {
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
    }
    return;
}

bool constitute_circle(int base, int loser)
{
    // Recursive function to check if a new lock will constitute a circle
    // If it makes a circle, return true
    if (loser == base)
    {
        // If the loser is the base of the circle again
        return true;
    }

    // Loop through candidates
    for (int i = 0; i < candidate_count; i++)
    {
        // If the pair is locked, check if it will constitute a circle again
        if (locked[loser][i])
        {
            if (constitute_circle(base, i))
            {
                return true;
            }
        }
    }

    // doesn't constitute a circle
    return false;
}

// Print the winner of the election
void print_winner(void)
{
    // A candidate win if there's no arrow pointing at him

    for (int i = 0; i < candidate_count; i++)
    {
        bool pointed_at = false;

        for (int j = 0; j < candidate_count; j++)
        {
            if (locked[j][i] == true)
            {
                pointed_at = true;
            }
        }

        if (!pointed_at)
        {
            printf("Winner: %s\n", candidates[i]);
        }
    }

    return;
}
